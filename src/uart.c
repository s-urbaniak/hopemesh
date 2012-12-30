#include "uart.h"

#include <stdlib.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/setbaud.h>

#include "error.h"
#include "ringbuf.h"

#define uart_disable_udrie_isr() (UCSRB &= ~(1<<UDRIE))
#define uart_enable_udrie_isr() (UCSRB |= (1<<UDRIE))

static volatile ringbuf_t *uart_out_buf;
static volatile ringbuf_t *uart_in_buf;

ISR(SIG_USART_RECV)
{
  ringbuf_add_err(uart_in_buf, UDR, ERR_UART);
}

ISR(SIG_USART_DATA)
{
  uint8_t c;
  if (ringbuf_remove(uart_out_buf, &c)) {
    UDR = c;
  } else {
    uart_disable_udrie_isr();
  }
}

void
uart_init(void)
{
  UCSRB |= (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
  UCSRC |= (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
  UBRRH = UBRRH_VALUE;
  UBRRL = UBRRL_VALUE;

  uart_enable_udrie_isr();

  uart_out_buf = ringbuf_new(10);
  uart_in_buf = ringbuf_new(10);
  PT_SEM_INIT(&uart_mutex, 1);
}

static const char *p;

bool
uart_tx_pgmstr(PGM_P str, char *buf)
{
  if (p == NULL) {
    strcpy_P(buf, str);
  }

  return uart_tx_str(buf);
}

bool
uart_tx_str(const char *str)
{
  uart_disable_udrie_isr();
  bool result = true;

  if (p == NULL) {
    p = str;
  }

  bool char_added = true;

  while (*p && char_added) {
    char_added = ringbuf_add(uart_out_buf, *p);
    if (char_added) {
      p++;
    }
  }

  if (*p) {
    result = false;
  } else {
    p = NULL;
    result = true;
  }

  uart_enable_udrie_isr();
  return result;
}

bool
uart_rx(char *where)
{
  return ringbuf_remove(uart_in_buf, (uint8_t*) where);
}

void
uart_tx_thread(void)
{
  if (ringbuf_size(uart_out_buf)) {
    uart_enable_udrie_isr();
  }
}
