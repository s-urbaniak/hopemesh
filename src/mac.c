#include <string.h>
#include "mac.h"
#include "llc.h"
#include "rfm12.h"
#include "pt-sem.h"
#include "debug.h"
#include "config.h"

#define SYNC_AFC 0xaa
#define EOP 0xaa
#define DUMMY 0xaa

typedef enum
{
  PREAMBLE, DATA, POSTAMBLE, FIN
} mac_state_t;

static struct pt pt;

static const uint8_t preamble[] = { SYNC_AFC, SYNC_AFC, 0x2d, 0xd4, '\0' };

static const uint8_t postamble[] = { EOP, DUMMY, '\0' };
static volatile const uint8_t *p;
static volatile mac_state_t state;

bool
mac_tx_rfm12(uint8_t *dest)
{
  bool fin = false;

  switch (state) {
    case (PREAMBLE):
      *dest = *p++;
      if (!*p)
        state = DATA;
      break;
    case (DATA):
      if (!llc_tx_mac(dest)) {
        state = POSTAMBLE;
        p = postamble;
      }
      break;
    case (POSTAMBLE):
      *dest = *p++;
      if (!*p) {
        fin = true;
        state = FIN;
      }
      break;
    default:
      break;
  }

  return fin;
}

bool
mac_rx_rfm12(rfm12_rx_t *rx)
{
  bool fin = true;

  mac_rx_t mac_rx;
  mac_rx.payload = 0;

  if (rx->status == RFM12_RX_OK) {
    if (rx->payload == EOP) {
      mac_rx.status = MAC_RX_FIN;
    } else {
      mac_rx.status = MAC_RX_OK;
      mac_rx.payload = rx->payload;
      fin = false;
    }
  } else {
    mac_rx.status = MAC_RX_ABORT;
  }

  llc_rx_mac(&mac_rx);

  return fin;
}

PT_THREAD(mac_tx(void))
{
  PT_BEGIN(&pt);
  PT_WAIT_THREAD(&pt, rfm12_lock());

  state = PREAMBLE;
  p = preamble;

  if (config_get(CONFIG_FLAGS) & (1 << CONFIG_FLAG_COLLISION_DETECTION)) {
    PT_WAIT_UNTIL(&pt, rfm12_is_carrier_free());
  }

  rfm12_enable_tx();
  rfm12_release();

  PT_WAIT_UNTIL(&pt, state == FIN);
  PT_END(&pt);
}

void
mac_init(void)
{
  PT_INIT(&pt);
}
