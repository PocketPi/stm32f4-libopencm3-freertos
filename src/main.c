#include "FreeRTOS.h"
#include "hw_init.h"
#include "i2c.h"
#include "ina232.h"
#include "task.h"
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <stdio.h>
#include <string.h>

static void task1(void *args __attribute((unused))) {
    static char buf[128] = {0};
    vTaskDelay(pdMS_TO_TICKS(2000));

    printf("\033[H\033[J");
    printf("SOUNDBOKS\n");
    /*
    uint16_t id = read_manufacturer_id();
    printf("manufacturer id: \t0x%04x\n", id);

    set_calibration_register(2048);
    set_configuration_register(0x4127 | 0x0400);
    */

    printf("rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ].ahb_frequency: %ld\n",
           rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ].ahb_frequency);
    printf("rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ].apb1_frequency: %ld\n",
           rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ].apb1_frequency);
    printf("rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ].apb2_frequency: %ld\n",
           rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ].apb2_frequency);

    for (;;) {
        /*
        printf("Shunt voltage[V]: \t%f\n", read_shunt_voltage());
        printf("Bus voltage[V]: \t%f\n", read_bus_voltage());
        printf("Current[A]: \t\t%f\n", read_current());
        printf("Power[W]: \t\t%f\n", read_power());
        */
        vTaskDelay(pdMS_TO_TICKS(1000));

        memset(buf, 0, sizeof(buf));

        vTaskList(buf);

        printf("\n\r----------------------------------------------");
        printf("\n\rName           State  Priority  Stack   Number");
        printf("\n\r----------------------------------------------");
        printf("\n\r%s", buf);
        printf("\n\r----------------------------------------------");
        printf("\n\r");

        memset(buf, 0, sizeof(buf));

        vTaskGetRunTimeStats(buf);

        printf("\n\rTask\t     Abs Time\t     %%Time");
        printf("\n\r*****************************************");
        printf("\n\r%s", buf);
        printf("\n\r*****************************************");
        printf("\n\r");
    }
}

int main(void) {
    hw_init();
    gpio_set(GPIOC, GPIO13); /* LED on/off */

    i2c_setup();

    xTaskCreate(task1, "Task1", 1024, NULL, configMAX_PRIORITIES - 1, NULL);
    vTaskStartScheduler();
    for (;;) {
        ;
    }

    return 0;
}
