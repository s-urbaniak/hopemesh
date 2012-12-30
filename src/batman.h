#ifndef __BATMAN_H__
#define __BATMAN_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "pt.h"
#include "net.h"

#define MAX_ROUTE_ENTRIES 2550

typedef struct route_t
{
  addr_t gateway_addr;
  addr_t target_addr;
  uint16_t seqno;
  uint16_t cnt;
  uint16_t time;
  uint16_t lost;
  struct route_t *next;
} route_t;

bool
route_present(addr_t target, addr_t *gateway);

void
route_save_or_update(addr_t target_addr, addr_t gateway_addr, uint16_t seqno);

void
batman_init(void);

PT_THREAD(
batman_tx(packet_t *packet, addr_t target_addr, uint16_t data_len));

PT_THREAD(
batman_rx(packet_t *packet));

PT_THREAD(
batman_thread(void));

void
batman_one_second_elapsed(void);

void
batman_inc_seqno(void);

route_t *
route_get(void);

#endif
