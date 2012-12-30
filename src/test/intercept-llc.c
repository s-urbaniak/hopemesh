#include <stdbool.h>
#include "../llc.h"
#include "../net.h"
#include "test-util.h"

extern bool
__real_llc_rx(packet_t *packet);

bool
__wrap_llc_rx(packet_t *packet)
{
  bool packet_arrived = __real_llc_rx(packet);
  if (packet_arrived) {
    llc_t *llc = (llc_t *) packet_get_llc(packet);

    for (uint16_t i = 0; i < llc->len + LLC_OFFSET; i++) {
      uint8_t data = packet->data[i];
      _spi_printf("llc_rx>0x%02x  |", data);
    }
  }
  return packet_arrived;
}
