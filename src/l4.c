#include "l4.h"

#include <string.h>
#include "batman.h"

static struct pt pt;
uint16_t message_len;

PT_THREAD(l4_tx(packet_t *packet, const char *message, addr_t target_addr))
{
  PT_BEGIN(&pt);
  char *l4 = (char *) packet_get_l4(packet);
  message_len = strlen(message) + 1;
  memcpy(l4, message, message_len);

  PT_WAIT_THREAD(&pt, batman_tx(packet, target_addr, message_len));
  PT_END(&pt);
}

void
l4_init(void)
{
  PT_INIT(&pt);
}
