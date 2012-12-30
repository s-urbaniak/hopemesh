#ifndef __L4_H__
#define __L4_H__

#include "pt.h"
#include "net.h"

PT_THREAD(l4_tx(packet_t *packet, const char *message, addr_t target_addr));

void
l4_init(void);

#endif
