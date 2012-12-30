#ifndef __SPI_H_
#define __SPI_H_

#include <stdint.h>
#include <stdbool.h>

void
spi_init(void);

uint16_t
spi_tx16(const uint16_t data, uint8_t _ss);

uint8_t
spi_tx(const uint8_t data, uint8_t _ss);

#endif
