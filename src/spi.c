#include "spi.h"
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>
#include <stdio.h>

#define CS   GPIO4
#define CLK  GPIO5
#define MISO GPIO6
#define MOSI GPIO7

#define M25LC256_PAGE_SIZE 64

#define M25LC256_STATUS_WIP (1 << 0)

enum CS_STATE {
    SELECT = 0,
    DESELECT
};

static void cs(bool set) {
    if (set == SELECT) {
        gpio_clear(GPIOA, CS); /* CS* select*/
    } else {
        gpio_set(GPIOA, CS); /* CS* deselect */
    }
}

uint8_t spi_read_reg(uint8_t reg) {
    (void)reg;
    cs(SELECT);
    (void)spi_xfer(SPI1, (uint8_t)reg);
    uint8_t data = (uint8_t)spi_xfer(SPI1, 0);
    cs(DESELECT);
    return data;
}

uint8_t spi_read_status_reg(void) {
    return spi_read_reg(M25LC256_RDSR_INST);
}

static void _write(uint16_t addr, const uint8_t *data, uint8_t count) {
    cs(SELECT);
    spi_set_dff_8bit(SPI1);
    (void)spi_xfer(SPI1, M25LC256_WRITE_INST);
    spi_set_dff_16bit(SPI1);
    (void)spi_xfer(SPI1, addr);
    spi_set_dff_8bit(SPI1);
    for (uint8_t i = 0; i < count; i++) {
        (void)spi_xfer(SPI1, data[i]);
    }
    cs(DESELECT);
    uint8_t status = spi_read_status_reg();
    while ((status & M25LC256_STATUS_WIP)) {
        status = spi_read_status_reg();
    }
}

void spi_write_byte(uint16_t addr, uint8_t data) {
    _write(addr, &data, 1);
}

void spi_write_page(uint16_t addr, uint8_t *data, uint8_t count) {
    _write(addr, data, count);
}

void spi_write_bytes(uint16_t addr, const uint8_t *data, size_t count) {
    size_t bytes_left = count;
    uint8_t pages_bytes_left =
        M25LC256_PAGE_SIZE - addr % M25LC256_PAGE_SIZE; /* Remaining bytes before hitting page boundary */

    /*
     * This loop constitutes a write cycle which ensures no more than
     * 64 bytes is written in a single cycle and to ensure data
     * isn't trying to be written over page boundaries.
     */
    while (bytes_left > pages_bytes_left) {
        _write((uint16_t)(addr + count - bytes_left), data + count - bytes_left, pages_bytes_left);
        bytes_left -= pages_bytes_left;

        /*
         * We may start in the middle of a page which is why we had to calculate the pageBytesLeft
         * After the first page is finished, every subsequent page will simply be a full 64 bytes.
         */
        pages_bytes_left = M25LC256_PAGE_SIZE;
    }
    _write((uint16_t)(addr + count - bytes_left), data + count - bytes_left, (uint8_t)bytes_left);
}

void spi_write_ints(uint8_t instruction) {
    spi_set_dff_8bit(SPI1);
    cs(SELECT);
    (void)spi_xfer(SPI1, instruction);
    cs(DESELECT);
}

static void _read(uint16_t addr, uint8_t *data, uint16_t count) {
    cs(SELECT);
    spi_set_dff_8bit(SPI1);
    (void)spi_xfer(SPI1, M25LC256_READ_INST);
    spi_set_dff_16bit(SPI1);
    (void)spi_xfer(SPI1, addr);
    spi_set_dff_8bit(SPI1);
    for (uint8_t i = 0; i < count; i++) {
        data[i] = (uint8_t)spi_xfer(SPI1, 0);
    }

    cs(DESELECT);
}

void spi_read_bytes(uint16_t addr, uint8_t *data, uint16_t count) {
    _read(addr, data, count);
}

uint8_t spi_read_byte(uint16_t addr) {
    uint8_t data;
    _read(addr, &data, 1);
    return data;
}

int spi_read_lba(uint32_t lba, uint8_t *copy_to) {
    printf("spi_read_lba lba: 0x%08lx\n", lba);
    _read((uint16_t)lba * 512, copy_to, 64);
    return 0;
}

int spi_write_lba(uint32_t lba, const uint8_t *copy_from) {
    printf("write lba: 0x%08lx\n", lba);

    spi_write_bytes((uint16_t)lba * 512, copy_from, 512);
    return 0;
}

void spi_setup(void) {
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_SPI1);

    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, CS);

    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN, CLK | MISO | MOSI);

    gpio_set_af(GPIOA, GPIO_AF5, CLK | MISO | MOSI);

    gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, CLK | MOSI);

    cs(DESELECT);

    spi_init_master(SPI1,
                    SPI_CR1_BAUDRATE_FPCLK_DIV_16,
                    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                    SPI_CR1_CPHA_CLK_TRANSITION_1,
                    SPI_CR1_DFF_8BIT,
                    SPI_CR1_MSBFIRST);

    spi_enable(SPI1);
}