#ifndef __LLC_H__
#define __LLC_H__

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "pt.h"
#include "mac.h"
#include "net.h"

bool
llc_tx_mac(uint8_t *dest);

void
llc_rx_mac(mac_rx_t *mac_rx);

void
llc_init(void);

bool
llc_rx(packet_t *packet);

PT_THREAD(
llc_tx(packet_t *packet, llc_type_t type, uint16_t data_len));

#endif
