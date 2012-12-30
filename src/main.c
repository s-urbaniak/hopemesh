#include <stdbool.h>

#include <avr/interrupt.h>
#include <util/delay.h>

#include "pt.h"
#include "watchdog.h"
#include "config.h"
#include "uart.h"
#include "shell.h"
#include "spi.h"
#include "rfm12.h"
#include "mac.h"
#include "llc.h"
#include "batman.h"
#include "l4.h"
#include "rxthread.h"
#include "timer.h"
#include "clock.h"

#define DDRSPI DDRB
#define DDMOSI DDB5
#define DDSCK DDB7
#define DD_SS_RADIO DDB4

#define DDRNIRQ DDRD
#define DDNIRQ DDD2
#define DDRFFIT DDRE
#define DDFFIT DDE0

void
ram_init(void) __attribute__ ((naked))
__attribute__ ((section (".init1")));

/**
 * Enable RAM and setup wait states
 */
void
ram_init(void)
{
  MCUCR = (1 << SRE) | (1 << SRW10);
}

/**
 * Initializes data direction registers (DDR*)
 */
static inline void
port_init(void)
{
  DDRSPI |= (1 << DDMOSI) | (1 << DDSCK) | (1 << DD_SS_RADIO);
  DDRNIRQ &= ~(1 << DDNIRQ);
  DDRFFIT &= ~(1 << DDFFIT);
}

/**
 * Causes a bootstrap delay of 500msec. This is needed in order to wait
 * for the uart chip to bootstrap.
 */
static inline void
bootstrap_delay(void)
{
  for (uint8_t i = 0; i < 50; i++) {
    _delay_ms(10);
  }
}

static inline void
main_init(void)
{
  bootstrap_delay();
  config_init();
  port_init();
  watchdog_init();
  timer_init();

  clock_init();
  spi_init();
  uart_init();
  shell_init();
  rfm12_init();
  mac_init();
  llc_init();
  batman_init();
  l4_init();
  rx_thread_init();

  sei();
}

int
main(void)
{
  main_init();

  while (true) {
    shell();
    batman_thread();
    rx_thread();
    uart_tx_thread();
    watchdog();
    timer_thread();
  }
}
