#include "usb.h"
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/desig.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/usb/msc.h>
#include <libopencm3/usb/usbd.h>
#include <stddef.h>

#define SIZEOFARRAY(x) (sizeof(x) / sizeof((x)[0]))

#define USB_DATA_ENDPOINT_IN  0x81
#define USB_DATA_ENDPOINT_OUT 0x01
#define USB_CDC_ENDPOINT_IN   0x83

static char usb_serial_number[13]; /* 12 bytes of desig and a \0 */

static usbd_device *usb_device;

static const struct usb_device_descriptor dev_descr = {
    .bLength = USB_DT_DEVICE_SIZE,
    .bDescriptorType = USB_DT_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 64,
    .idVendor = 0xaabb,
    .idProduct = 0xccdd,
    .bcdDevice = 0x0200,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};

static const struct {
    struct usb_cdc_header_descriptor header;
    struct usb_cdc_call_management_descriptor call_mgmt;
    struct usb_cdc_acm_descriptor acm;
    struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed))
cdcacm_functional_descriptors = {.header =
                                     {
                                         .bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
                                         .bDescriptorType = CS_INTERFACE,
                                         .bDescriptorSubtype = USB_CDC_TYPE_HEADER,
                                         .bcdCDC = 0x0110,
                                     },
                                 .call_mgmt =
                                     {
                                         .bFunctionLength = sizeof(struct usb_cdc_call_management_descriptor),
                                         .bDescriptorType = CS_INTERFACE,
                                         .bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
                                         .bmCapabilities = 0,
                                         .bDataInterface = 1,
                                     },
                                 .acm =
                                     {
                                         .bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
                                         .bDescriptorType = CS_INTERFACE,
                                         .bDescriptorSubtype = USB_CDC_TYPE_ACM,
                                         .bmCapabilities = 0,
                                     },
                                 .cdc_union = {
                                     .bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
                                     .bDescriptorType = CS_INTERFACE,
                                     .bDescriptorSubtype = USB_CDC_TYPE_UNION,
                                     .bControlInterface = 0,
                                     .bSubordinateInterface0 = 1,
                                 }};

/*
 * This notification endpoint isn't implemented. According to CDC spec it's
 * optional, but its absence causes a NULL pointer dereference in the
 * Linux cdc_acm driver.
 */
static const struct usb_endpoint_descriptor comm_endp[] = {{
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = USB_CDC_ENDPOINT_IN,
    .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
    .wMaxPacketSize = 16,
    .bInterval = 255,
}};

static const struct usb_endpoint_descriptor data_endp[] = {{
                                                               .bLength = USB_DT_ENDPOINT_SIZE,
                                                               .bDescriptorType = USB_DT_ENDPOINT,
                                                               .bEndpointAddress = USB_DATA_ENDPOINT_OUT,
                                                               .bmAttributes = USB_ENDPOINT_ATTR_BULK,
                                                               .wMaxPacketSize = 64,
                                                               .bInterval = 1,
                                                           },
                                                           {
                                                               .bLength = USB_DT_ENDPOINT_SIZE,
                                                               .bDescriptorType = USB_DT_ENDPOINT,
                                                               .bEndpointAddress = USB_DATA_ENDPOINT_IN,
                                                               .bmAttributes = USB_ENDPOINT_ATTR_BULK,
                                                               .wMaxPacketSize = 64,
                                                               .bInterval = 1,
                                                           }};

static const struct usb_interface_descriptor comm_iface[] = {{
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 1,
    .bInterfaceClass = USB_CLASS_CDC,
    .bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
    .bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
    .iInterface = 0,

    .endpoint = comm_endp,

    .extra = &cdcacm_functional_descriptors,
    .extralen = sizeof(cdcacm_functional_descriptors),
}};

static const struct usb_interface_descriptor data_iface[] = {{
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = 1,
    .bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = USB_CLASS_DATA,
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface = 0,

    .endpoint = data_endp,
}};

static const struct usb_interface ifaces[] = {
    {
        .num_altsetting = 1,
        .altsetting = comm_iface,
    },
    {
        .num_altsetting = 1,
        .altsetting = data_iface,
    },
};

static const struct usb_config_descriptor config_descr = {
    .bLength = USB_DT_CONFIGURATION_SIZE,
    .bDescriptorType = USB_DT_CONFIGURATION,
    .wTotalLength = 0,
    .bNumInterfaces = 2,
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = 0x80,
    .bMaxPower = 0x32,

    .interface = ifaces,
};

static const char *usb_strings[] = {
    "SOUNDBOKS",
    "STROMBOKS",
    usb_serial_number,
};

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

static enum usbd_request_return_codes
cdcacm_control_request(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
                       void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req)) {
    (void)complete;
    (void)buf;
    (void)usbd_dev;

    switch (req->bRequest) {
        case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
            /*
             * This Linux cdc_acm driver requires this to be implemented
             * even though it's optional in the CDC spec, and we don't
             * advertise it in the ACM functional descriptor.
             */
            return USBD_REQ_HANDLED;
        }
        case USB_CDC_REQ_SET_LINE_CODING:
            if (*len < sizeof(struct usb_cdc_line_coding)) {
                return USBD_REQ_NOTSUPP;
            }

            return USBD_REQ_HANDLED;
    }
    return USBD_REQ_NOTSUPP;
}
/*
static void cdcacm_sof_callback(void) {
    char buf[64];
    uint16_t i = 0;
    while (!usb_fifo_is_empty() && i < 64) {
        usb_fifo_get(&buf[i++]);
    }

    if (i && usb_device) {
        while (usbd_ep_write_packet(usb_device, USB_DATA_ENDPOINT_IN, buf, i) == 0) {
        };
    }
}
*/
static void cdcacm_data_rx_cb(usbd_device *usbd_dev, uint8_t ep) {
    char buf[64];
    uint16_t len = usbd_ep_read_packet(usbd_dev, ep, buf, 64);
    (void)len;
}

static void usb_cdc_set_config_callback(usbd_device *usbd_dev, uint16_t wValue) {
    (void)wValue;

    usbd_ep_setup(usbd_dev, USB_DATA_ENDPOINT_OUT, USB_ENDPOINT_ATTR_BULK, 64, cdcacm_data_rx_cb);
    usbd_ep_setup(usbd_dev, USB_DATA_ENDPOINT_IN, USB_ENDPOINT_ATTR_BULK, 64, NULL);
    usbd_ep_setup(usbd_dev, USB_CDC_ENDPOINT_IN, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

    usbd_register_control_callback(usbd_dev,
                                   USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
                                   USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
                                   cdcacm_control_request);

    // usbd_register_sof_callback(usbd_dev, cdcacm_sof_callback);
}

void usb_setup(void) {
    desig_get_unique_id_as_string(usb_serial_number, sizeof(usb_serial_number));

    usb_device = usbd_init(&otgfs_usb_driver,
                           &dev_descr,
                           &config_descr,
                           usb_strings,
                           SIZEOFARRAY(usb_strings),
                           usbd_control_buffer,
                           sizeof(usbd_control_buffer));

    usbd_register_set_config_callback(usb_device, usb_cdc_set_config_callback);

    /* NVIC setup. */
    nvic_enable_irq(NVIC_OTG_FS_IRQ);
    nvic_set_priority(NVIC_OTG_FS_IRQ, 1);
}

void otg_fs_isr(void) {
    if (usb_device) {
        usbd_poll(usb_device);
    }
}

int _write(int fd, char *ptr, int len);

/*
 * Called by libc stdio fwrite functions
 */
int _write(int fd, char *ptr, int len) {
    int i = 0;
    char carrigereturn = '\r';

    // if (fifo_initialized == false) {
    //     USB_TX_FIFO_init(&usb_tx_fifo);
    // }

    /*
     * Write "len" of char from "ptr" to file id "fd"
     * Return number of char written.
     *
     * Only work for STDOUT, STDIN, and STDERR
     */
    if (fd > 2) {
        return -1;
    }
    while (*ptr && (i < len)) {
        while (usbd_ep_write_packet(usb_device, USB_DATA_ENDPOINT_IN, ptr, 1) == 0) {
        };

        // USB_TX_FIFO_put(&usb_tx_fifo, ptr);
        if (*ptr == '\n') {
            while (usbd_ep_write_packet(usb_device, USB_DATA_ENDPOINT_IN, &carrigereturn, 1) == 0) {
            };
            // USB_TX_FIFO_put(&usb_tx_fifo, &carrigereturn);
        }
        i++;
        ptr++;
    }
    return i;
}
