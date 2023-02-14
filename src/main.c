#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

/* monotonically increasing number of milliseconds from reset
 * overflows every 49 days if you're wondering
 */
volatile uint32_t system_millis;

/* Called when systick fires */
void sys_tick_handler(void) {
    system_millis++;
}

/* sleep for delay milliseconds */
static void msleep(uint32_t delay) {
    uint32_t wake = system_millis + delay;
    while (wake > system_millis)
        ;
}

/*
 * systick_setup(void)
 *
 * This function sets up the 1khz "system tick" count. The SYSTICK counter is a
 * standard feature of the Cortex-M series.
 */
static void systick_setup(void) {
    /* clock rate / 1000 to get 1mS interrupt rate */
    systick_set_reload(84000);
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
    systick_counter_enable();
    /* this done last */
    systick_interrupt_enable();
}

/* Set STM32 system clock to 168 MHz. */
static void clock_setup(void) {
    rcc_clock_setup(&rcc_clock_config[RCC_CLOCK_CONFIG_HSI_PLL_64MHZ]);

    rcc_periph_clock_enable(RCC_GPIOC);
}

static void gpio_setup(void) {
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO6);
}

int main(void) {
    clock_setup();
    gpio_setup();
    systick_setup();

    // The task scheduler is blocking, so we should never come here...
    for (;;) {
        gpio_toggle(GPIOC, GPIO6); /* LED on/off */
        msleep(1000);
    };
    return 0;
}