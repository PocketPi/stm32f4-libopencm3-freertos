//#include <libopencm3/stm32/rcc.h>
//#include <libopencm3/stm32/gpio.h>
/*
static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_3V3_84MHZ]);

	rcc_periph_clock_enable(RCC_GPIOA);
}

static void gpio_setup(void)
{
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO5);
}
*/
int main(void)
{
	//clock_setup();
	//gpio_setup();

    // The task scheduler is blocking, so we should never come here...
    for (;;){
        //gpio_toggle(GPIOA, GPIO5);	/* LED on/off */
    };
	return 0;
}