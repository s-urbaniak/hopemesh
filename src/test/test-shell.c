#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include <avr/interrupt.h>

#include "../config.h"
#include "../uart.h"
#include "../shell.h"
#include "../timer.h"
#include "../spi.h"
#include "../rfm12.h"
#include "../mac.h"
#include "../llc.h"
#include "../batman.h"
#include "../clock.h"
#include "../rxthread.h"

#include "test-util.h"

int
main(int argc, char **argv)
{
  curses_init();
  config_init();

  uart_init();
  timer_init();
  spi_init();
  clock_init();
  rfm12_init();
  mac_init();
  llc_init();
  batman_init();
  rx_thread_init();
  shell_init();
  sei();

  route_save_or_update (0xaaaa, 0xaaaa, 16);

  do {
    // CALL_ISR(SIG_OVERFLOW0);
    CALL_ISR(SIG_INTERRUPT0);
    timer_thread();
    batman_thread();

    shell();
    uart_tx_thread();
    rx_thread();

    while (UCSRB & (1 << UDRIE)) {
      CALL_ISR(SIG_USART_DATA);

      if (UCSRB & (1 << UDRIE)) {
        echochar(UDR);
      }
    }

    if (curses_getch(&UDR)) {
      CALL_ISR(SIG_USART_RECV);
    }
  }
  while (true);

  return 0;
}
