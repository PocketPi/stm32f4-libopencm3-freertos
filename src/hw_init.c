#include "hw_init.h"
#include "usb.h"
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <stdio.h>

static volatile uint32_t tim2_counter = 0;

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

    // USB GPIOs and Peripherial
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_OTGFS);
}

static void gpio_setup(void) {
    /* Setup GPIO pin GPIO13 on GPIO port C for LED. */
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);

    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11 | GPIO12);
    gpio_set_output_options(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, GPIO11 | GPIO12);
    gpio_set_af(GPIOA, GPIO_AF10, GPIO11 | GPIO12);
}

static void tim2_setup(void) {
    /* Enable TIM2 clock. */
    rcc_periph_clock_enable(RCC_TIM2);

    timer_disable_counter(TIM2);

    /* Enable TIM2 interrupt. */
    nvic_enable_irq(NVIC_TIM2_IRQ);

    // apb2 = 96MHz, counter freq 10000Hz, period = freq * 2 as we have 50/50 duty cycle
    timer_set_period(TIM2, rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ].apb2_frequency / (10000 * 2));

    /* Counter enable. */
    timer_enable_counter(TIM2);

    /* Enable Channel 1 compare interrupt to recalculate compare values */
    timer_enable_irq(TIM2, TIM_DIER_CC1IE);
}

uint32_t get_tim2_counter(void) {
    return tim2_counter;
}

void hw_init(void) {
    clock_setup();
    gpio_setup();
    systick_setup();
    usb_setup();
    tim2_setup();
}

void tim2_isr(void) {
    if (timer_get_flag(TIM2, TIM_SR_CC1IF)) {
        /* Clear compare interrupt flag. */
        timer_clear_flag(TIM2, TIM_SR_CC1IF);
        tim2_counter++;

        /* Toggle LED to indicate compare event. */
        gpio_toggle(GPIOC, GPIO13); /* LED on/off */
    }
}