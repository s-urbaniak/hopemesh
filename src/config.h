#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>
#include <avr/io.h>

#define UCSRB UCSR0B
#define TXEN TXEN0
#define RXEN RXEN0
#define UDRIE UDRIE0
#define RXCIE RXCIE0
#define UCSRC UCSR0C
#define URSEL URSEL0
#define UCSZ1 UCSZ01
#define UCSZ0 UCSZ00
#define UBRRH UBRR0H
#define UBRRL UBRR0L
#define UCSRA UCSR0A
#define UDRE UDRE0
#define UCSRB UCSR0B
#define UDRIE UDRIE0
#define UDR UDR0
#define SIG_USART_DATA SIG_USART0_DATA
#define SIG_USART_RECV SIG_USART0_RECV

#define BAUD 38400

#define PORT_SS PORTB
#define _SS_RADIO PB4

#define RFM12_INT_NIRQ INT0
#define SIG_RFM12_NIRQ SIG_INTERRUPT0

#define MAX_CONFIGS 3
#define CONFIG_NODE_ADDR 0
#define CONFIG_TTL 1
#define CONFIG_FLAGS 2

#define CONFIG_FLAG_RSSI_DETECTION 0
#define CONFIG_FLAG_COLLISION_DETECTION 1

uint16_t
config_get(uint8_t index);

void
config_init(void);

void
config_set(uint8_t index, uint16_t value);

#endif
