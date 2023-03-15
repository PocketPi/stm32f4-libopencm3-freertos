#include "FreeRTOS.h"
#include "task.h"
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

/* Set up a timer to create 1mS ticks. */
static void systick_setup(void) {
    /* clock rate / 1000 to get 1mS interrupt rate */
    systick_set_reload(rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ].ahb_frequency / 1000);
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
    systick_counter_enable();
    /* this done last */
    systick_interrupt_enable();
}

static void clock_setup(void) {
    rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);

    rcc_periph_clock_enable(RCC_GPIOC);
}

static void gpio_setup(void) {
    /* Setup GPIO pin GPIO13 on GPIO port C for LED. */
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
}

static void hw_init(void) {
    clock_setup();
    gpio_setup();
    systick_setup();
}

static void task1(void *args __attribute((unused))) {
    for (;;) {
        gpio_toggle(GPIOC, GPIO13);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void) {
    hw_init();
    xTaskCreate(task1, "LED1", 100, NULL, configMAX_PRIORITIES - 1, NULL);
    vTaskStartScheduler();
    for (;;)
        ;

    return 0;
}
