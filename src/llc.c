#include <stdbool.h>
#include <string.h>
#include <util/crc16.h>

#include "error.h"
#include "llc.h"
#include "mac.h"
#include "hamming.h"
#include "batman.h"
#include "watchdog.h"

static struct pt pt_tx;

typedef enum
{
  LLC_STATE_DATA, LLC_STATE_ABORTED, LLC_STATE_FIN
} llc_state_e;

typedef struct
{
  packet_t *packet;
  uint16_t cnt;
  bool hasnext;
  uint8_t nextbyte;
  llc_state_e state;
} llc_state_t;

static llc_state_t state_tx, state_rx;
static packet_t packet_rx;
static bool needs_processing;

static inline void
llc_tx_frame(llc_state_t *s)
{
  s->hasnext = true;
  uint16_t pos = (s->cnt >> 1);

  switch (s->state) {
    case (LLC_STATE_DATA):
      s->nextbyte = s->packet->data[pos];

      llc_t *llc = (llc_t *) packet_get_llc(s->packet);

      if (pos == LLC_HEADER_SIZE + llc->len - 1) {
        s->state = LLC_STATE_FIN;
        s->hasnext = false;
      }
      break;

    default:
      break;
  }
}

bool
llc_tx_mac(uint8_t *dest)
{
  if (state_tx.cnt++ & 0x01) {
    *dest = hamming_enc_high(state_tx.nextbyte);
    return state_tx.hasnext;
  } else {
    llc_tx_frame(&state_tx);
    *dest = hamming_enc_low(state_tx.nextbyte);
  }

  return true;
}

static inline void
llc_rx_frame(llc_state_t *s)
{
  uint16_t pos = (s->cnt >> 1) - 1;
  s->hasnext = true;
  llc_t *llc = (llc_t *) packet_get_llc(s->packet);
  bool fin = false;

  switch (s->state) {
    case (LLC_STATE_DATA):
      fin = (pos == LLC_HEADER_SIZE - 1)
          && (llc->len > PACKET_MAX_SIZE - LLC_HEADER_SIZE);

      fin |= pos >= PACKET_MAX_SIZE;

      if (fin) {
        s->state = LLC_STATE_ABORTED;
      } else {
        s->packet->data[pos] = s->nextbyte;

        fin = (pos == LLC_HEADER_SIZE + llc->len - 1);

        if (fin) {
          s->state = LLC_STATE_FIN;
          s->hasnext = false;
        }
      }
      break;

    default:
      break;
  }
}

static inline void
llc_rx_reset(llc_state_t *s)
{
  s->state = LLC_STATE_DATA;
  s->cnt = 0;
  s->hasnext = false;
  s->nextbyte = 0;
  s->packet = &packet_rx;

  llc_t *rx = (llc_t *) packet_get_llc(s->packet);
  rx->len = 0;
  rx->crc = 0xffff;
}

void
llc_rx_mac(mac_rx_t *mac_rx)
{
  if (needs_processing) {
    // drop byte, because there is still a previous packet not being processed
    return;
  }

  switch (mac_rx->status) {
    case (MAC_RX_OK):
      if (state_rx.cnt++ & 0x01) {
        state_rx.nextbyte |= hamming_dec_high(mac_rx->payload);
        llc_rx_frame(&state_rx);
      } else {
        state_rx.nextbyte = hamming_dec_low(mac_rx->payload);
      }
      break;

    case (MAC_RX_FIN):
      if (state_rx.state == LLC_STATE_FIN) {
        needs_processing = true;
      } else {
        llc_rx_reset(&state_rx);
      }
      break;

    default:
      llc_rx_reset(&state_rx);
      break;
  }
}

bool
llc_rx(packet_t *packet)
{
  bool packet_arrived = false;

  if (needs_processing) {
    llc_t *rx = (llc_t *) packet_get_llc(state_rx.packet);

    uint16_t crc = 0xffff;
    crc = _crc16_update(crc, (uint8_t) (rx->len >> sizeof(uint8_t) * 8));
    crc = _crc16_update(crc, (uint8_t) rx->len);

    uint8_t *data = packet_get_batman(state_rx.packet);
    for (uint16_t i = 0; i < rx->len; i++) {
      crc = _crc16_update(crc, *data++);
    }

    if (crc == rx->crc) {
      memcpy(packet, state_rx.packet, PACKET_MAX_SIZE);
      packet_arrived = true;
    }

    llc_rx_reset(&state_rx);
    needs_processing = false;
  }

  return packet_arrived;
}

PT_THREAD(llc_tx(packet_t *packet, llc_type_t type, uint16_t data_len))
{
  PT_BEGIN(&pt_tx);

  state_tx.cnt = 0;
  state_tx.state = LLC_STATE_DATA;
  state_tx.packet = packet;

  llc_t *tx = (llc_t *) packet_get_llc(packet);

  tx->len = data_len;
  tx->type = type;

  tx->crc = 0xffff;
  tx->crc = _crc16_update(tx->crc, (uint8_t) (data_len >> 8));
  tx->crc = _crc16_update(tx->crc, (uint8_t) data_len);

  uint8_t *data = packet_get_batman(packet);
  for (uint16_t i = 0; i < data_len; i++) {
    tx->crc = _crc16_update(tx->crc, *data++);
  }

  PT_WAIT_THREAD(&pt_tx, mac_tx());
  PT_END(&pt_tx);
}

void
llc_init(void)
{
  PT_INIT(&pt_tx);
  llc_rx_reset(&state_rx);
  needs_processing = false;
}
