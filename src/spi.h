#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <stddef.h>

#define M25LC256_WRSR_INST  0x1
#define M25LC256_WRITE_INST 0x2
#define M25LC256_READ_INST  0x3
#define M25LC256_WRDI_INST  0x4
#define M25LC256_RDSR_INST  0x5
#define M25LC256_WREN_INST  0x6

void spi_setup(void);
uint8_t spi_read_reg(uint8_t reg);
void spi_write_reg(uint8_t reg, uint8_t value);
void spi_write_ints(uint8_t instruction);
void spi_write_byte(uint16_t addr, uint8_t data);
uint8_t spi_read_byte(uint16_t addr);
uint8_t spi_read_status_reg(void);
void spi_write_page(uint16_t addr, uint8_t *data, uint8_t count);
void spi_read_bytes(uint16_t addr, uint8_t *data, uint16_t count);
void spi_write_bytes(uint16_t addr, const uint8_t *data, size_t count);


int spi_read_lba(uint32_t lba, uint8_t *copy_to);
int spi_write_lba(uint32_t lba, const uint8_t *copy_from);

#endif
