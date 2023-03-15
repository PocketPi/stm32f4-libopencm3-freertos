#include "i2c.h"
#include "smbus.h"
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <stdint.h>

void i2c_transmit(uint8_t i2c_addr, uint8_t *tx_data, uint8_t count) {
    i2c_transfer7(I2C1, i2c_addr, tx_data, count, NULL, 0);
}

void i2c_recieve(uint8_t i2c_addr, uint8_t *rx_data, uint8_t count) {
    i2c_transfer7(I2C1, i2c_addr, NULL, 0, rx_data, count);
}

void i2c_setup(void) {
    rcc_periph_clock_enable(RCC_I2C1);
    rcc_periph_clock_enable(RCC_GPIOB);

    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6 | GPIO7);
    gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO6 | GPIO7);
    gpio_set_af(GPIOB, GPIO_AF4, GPIO6 | GPIO7);
    i2c_peripheral_disable(I2C1);
    i2c_set_speed(I2C1, i2c_speed_fm_400k, rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ].ahb_frequency / (uint32_t)1e6);
    i2c_peripheral_enable(I2C1);
}