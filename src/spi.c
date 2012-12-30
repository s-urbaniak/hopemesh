#include <avr/interrupt.h>
#include <stdint.h>

#include "config.h"
#include "ringbuf.h"

#define spi_disable_isr() (SPCR &= ~(1<<SPIE))
#define spi_enable_isr() (SPCR |= (1<<SPIE))
#define spi_is_isr_disabled() !(SPCR & (1<<SPIE))

#define spi_ss_low(ss) (PORT_SS &= ~(1<<ss))
#define spi_ss_high(ss) (PORT_SS |= (1<<ss))
#define spi_is_ss_high(ss) (PORT_SS & (1<<ss))

#define spi_block_until_sent() while (!(SPSR & (1<<SPIF)))

void
spi_init(void)
{
  SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1);
  // SPSR =(1<<SPI2X);

  spi_ss_high(_SS_RADIO);
  spi_disable_isr();
}

uint8_t
spi_tx(const uint8_t data, uint8_t _ss)
{
  spi_ss_low(_ss);
  SPDR = data;
  spi_block_until_sent()
    ;spi_ss_high(_ss);

  return SPDR;
}

uint16_t
spi_tx16(const uint16_t data, uint8_t _ss)
{
  uint16_t response = 0x0000;
  spi_ss_low(_ss);

  // tx & rx upper bytes
  SPDR = data >> 8;
  spi_block_until_sent()
    ;
  response = SPDR << 8;

  // tx & rx lower bytes
  SPDR = data;
  spi_block_until_sent()
    ;
  response |= SPDR;

  spi_ss_high(_ss);
  return response;
}
