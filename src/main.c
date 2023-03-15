#include "FreeRTOS.h"
#include "hw_init.h"
#include "i2c.h"
#include "ina232.h"
#include "task.h"
#include <libopencm3/stm32/gpio.h>
#include <stdio.h>

static void task1(void *args __attribute((unused))) {
    vTaskDelay(pdMS_TO_TICKS(2000));

    printf("\033[H\033[J");
    printf("SOUNDBOKS\n");

    uint16_t id = read_manufacturer_id();
    printf("manufacturer id: \t0x%04x\n", id);

    set_calibration_register(2048);
    set_configuration_register(0x4127 | 0x0400);

    for (;;) {

        printf("Shunt voltage[V]: \t%f\n", read_shunt_voltage());
        printf("Bus voltage[V]: \t%f\n", read_bus_voltage());
        printf("Current[A]: \t\t%f\n", read_current());
        printf("Power[W]: \t\t%f\n", read_power());

        /*
        printf("Shunt voltage raw: \t0x%04x\n", read_shunt_voltage_raw());
        printf("Bus voltage raw: \t0x%04x\n", read_bus_voltage_raw());
        printf("Current raw: \t\t0x%04x\n", read_current_raw());
        printf("Power raw: \t\t0x%04x\n", read_power_raw());
        */
        gpio_toggle(GPIOC, GPIO13); /* LED on/off */
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

    int main(void) {
        hw_init();
        i2c_setup();

        xTaskCreate(task1, "LED1", 1024, NULL, configMAX_PRIORITIES - 1, NULL);
        vTaskStartScheduler();
        for (;;) {
            ;
        }

        return 0;
    }
