#include <stdbool.h>
#include <stdio.h>

#include <avr/pgmspace.h>

#include "uart.h"
#include "pt.h"
#include "net.h"
#include "batman.h"

static PROGMEM const char packet_info[] = "\n\r$ RX sender: 0x%x, "
    "originator=0x%x, msg=";

static PROGMEM const char new_prompt[] = "\n\r$ ";

static struct pt pt;
static const char *out_ptr;
static char out_buf[256];
static packet_t packet;
static batman_t *batman_header;

void
rx_thread_init(void)
{
  PT_INIT(&pt);
}

PT_THREAD(rx_thread(void))
{
  PT_BEGIN(&pt);
  PT_WAIT_THREAD(&pt, batman_rx(&packet));
  batman_header = (batman_t *) packet_get_batman(&packet);

  UART_WAIT(&pt);

  sprintf_P(out_buf, packet_info, batman_header->sender_addr, batman_header->originator_addr);
  UART_TX_NOSIGNAL(&pt, out_buf);
  out_ptr = (const char *) packet_get_l4(&packet);
  UART_TX_NOSIGNAL(&pt, out_ptr);
  UART_TX_PGM_NOSIGNAL(&pt, new_prompt, out_buf);

  UART_SIGNAL(&pt);

  PT_END(&pt);
}
