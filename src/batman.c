#include <string.h>

#include "batman.h"
#include "llc.h"
#include "debug.h"
#include "timer.h"
#include "config.h"
#include "pt-sem.h"
#include "clock.h"

typedef enum
{
  BATMAN_DROP,
  BATMAN_REBROADCAST,
  BATMAN_RX
}
batman_bc_status;

static struct pt pt_thread, pt_tx, pt_rx;
static struct pt_sem mutex;
static bool send_ogm;
static route_t *route_table;
static uint16_t seqno = 0;

route_t *
route_get(void)
{
  return route_table;
}

bool
route_present(addr_t target, addr_t *gateway)
{
  route_t *r = route_table;
  route_t *next = NULL;
  route_t *best = NULL;
  *gateway = 0;

  while (r != NULL) {
    next = r->next;

    if (r->target_addr == target) {
      if (best == NULL) {
        best = r;
      } else {
        // See RFC 5.4
        if (r->cnt > best->cnt) {
          best = r;
        }
      }
    }

    r = next;
  }

  if (best == NULL) {
    return false;
  } else {
    *gateway = best->gateway_addr;
    return true;
  }
}

static inline void
route_delete(route_t *entry)
{
  route_t *cur = NULL;
  route_t *prev = NULL;

  for (cur = route_table; cur != NULL; prev = cur, cur = cur->next) {
    if (cur == entry) {
      if (prev == NULL) {
        route_table = cur->next;
      } else {
        prev->next = cur->next;
      }

      free(cur);
      return;
    }
  }
}

static void
route_check_purge_timeout(void)
{
  uint16_t current_time = clock_get_time();
  route_t *r = route_table;
  route_t *next = NULL;

  while (r != NULL) {
    next = r->next;
    if ((current_time - r->time) > PURGE_TIMEOUT) {
      route_delete(r);
    }
    r = next;
  }
}

static inline bool
route_is_bidirectional(ogm_t *ogm)
{
  route_t *r = route_table;

  while (r != NULL) {
    if (r->gateway_addr == ogm->sender_addr) {
      return true;
    }

    r = r->next;
  }

  return false;
}

void
route_save_or_update(addr_t target_addr, addr_t gateway_addr, uint16_t seqno)
{
  route_t *r = route_table;
  route_t *prev = NULL;
  uint16_t cnt = 0;
  bool new = false;

  while (r != NULL) {
    if ((r->target_addr == target_addr) && (r->gateway_addr == gateway_addr)) {
      break;
    } else {
      cnt++;
      prev = r;
      r = r->next;
    }
  }

  if (r == NULL) {
    if (cnt != MAX_ROUTE_ENTRIES) {
      r = malloc(sizeof(route_t));
      r->cnt = 0;
      r->lost = 0;
      r->next = NULL;
      if (prev != NULL) {
        prev->next = r;
      }

      if (route_table == NULL) {
        route_table = r;
      }

      new = true;
    } else {
      return;
    }
  }

  if (!new && (r->cnt == 1)) {
    new = true;
  }

  if (!new && (seqno-1 != r->seqno)) {
    r->lost += seqno - r->seqno;
  }

  r->gateway_addr = gateway_addr;
  r->target_addr = target_addr;
  r->seqno = seqno;
  r->time = clock_get_time();
  r->cnt++;
}

static bool
ogm_rebroadcast(ogm_t *ogm)
{
  bool examine_packet = false;

  // RFC 5.2.1, 5.2.2 (5.2.3 not relevant)
  examine_packet = (ogm->version == BATMAN_VERSION);
  examine_packet &= (ogm->sender_addr != config_get(CONFIG_NODE_ADDR));
  examine_packet &= (ogm->ttl > 0);

  if (examine_packet) {
    if (ogm->originator_addr == config_get(CONFIG_NODE_ADDR)) {
      // RFC 5.2.4 -> 5.3
      examine_packet = ogm->flags & (1 << OGM_FLAG_IS_DIRECT);
      examine_packet &= !route_is_bidirectional(ogm);
      if (examine_packet) {
        route_save_or_update(ogm->sender_addr, ogm->sender_addr, 0);
      }
    } else {
      // RFC 5.2.5
      if (!(ogm->flags & (1 << OGM_FLAG_UNIDIRECTIONAL))) {
        ogm->flags = 0;

        if (route_is_bidirectional(ogm)) {
          // RFC 5.2.6
          route_save_or_update(ogm->originator_addr, ogm->sender_addr,
              ogm->seqno);
        } else {
          // ogm is not in routing table, therefore unidirectional
          ogm->flags |= (1 << OGM_FLAG_UNIDIRECTIONAL);
        }

        // sender is the same as the originator, therefore a direct neighbour
        if (ogm->sender_addr == ogm->originator_addr) {
          ogm->flags |= (1 << OGM_FLAG_IS_DIRECT);
        }

        // RFC 5.2.7
        ogm->sender_addr = config_get(CONFIG_NODE_ADDR);
        ogm->ttl--;

        return true;
      }
    }
  }

  return false;
}

PT_THREAD(batman_tx(packet_t *packet, addr_t target_addr, uint16_t data_len))
{
  PT_BEGIN(&pt_tx);
  static batman_t *header;

  header = (batman_t *) packet_get_batman(packet);
  if (!route_present(target_addr, &header->gateway_addr)) {
    PT_END(&pt_tx);
  }

  PT_SEM_WAIT(&pt_tx, &mutex);

  header->originator_addr = config_get(CONFIG_NODE_ADDR);
  header->sender_addr = header->originator_addr;
  header->target_addr = target_addr;
  header->ttl = config_get(CONFIG_TTL);
  header->version = BATMAN_VERSION;

  PT_WAIT_THREAD(&pt_tx,
      llc_tx(packet, UNICAST, BATMAN_HEADER_SIZE + data_len));

  PT_SEM_SIGNAL(&pt_tx, &mutex);
  PT_END(&pt_tx);
}

inline static batman_bc_status
batman_rebroadcast(batman_t *header)
{
  if (header->target_addr == config_get(CONFIG_NODE_ADDR)) {
    // packet received targeted at us
    return BATMAN_RX;
  }

  if (header->ttl == 0) {
    return BATMAN_DROP;
  }

  if (header->gateway_addr != config_get(CONFIG_NODE_ADDR)) {
    // our node is not supposed to be the gateway for this packet.
    // do not rebroadcast
    return BATMAN_DROP;
  }

  if (header->originator_addr == config_get(CONFIG_NODE_ADDR)) {
    // only resend if originator is a different address than our node address.
    // if we received such a packet it looped back to us in the network
    return BATMAN_DROP;
  }

  if (route_present(header->target_addr, &header->gateway_addr)) {
    // only rebroadcast if we have a gateway for this target
    header->sender_addr = config_get(CONFIG_NODE_ADDR);
  } else {
    return BATMAN_DROP;
  }

  header->ttl--;

  return BATMAN_REBROADCAST;
}

PT_THREAD(batman_rx(packet_t *packet))
{
  PT_BEGIN(&pt_rx);
  static bool loop;
  static batman_t *header;
  static llc_t *llc;

  loop = true;
  while (loop) {
    PT_WAIT_UNTIL(&pt_rx, llc_rx(packet));

    llc = (llc_t *) packet_get_llc(packet);

    if (llc->type == BROADCAST) {
      ogm_t *ogm_rx = (ogm_t *) packet_get_ogm(packet);
      if (ogm_rebroadcast(ogm_rx)) {
        PT_SEM_WAIT(&pt_rx, &mutex);
        PT_WAIT_THREAD(&pt_rx, llc_tx(packet, BROADCAST, OGM_HEADER_SIZE));
        PT_SEM_SIGNAL(&pt_rx, &mutex);
      }
    } else {
      header = (batman_t *) packet_get_batman(packet);

      switch (batman_rebroadcast(header)) {
        case (BATMAN_RX):
          loop = false;
          break;
        case (BATMAN_REBROADCAST):
          PT_WAIT_THREAD(&pt_rx,
              llc_tx(packet, UNICAST, BATMAN_HEADER_SIZE + llc->len));
          break;
        case (BATMAN_DROP):
          break;
      }
    }
  }

  PT_END(&pt_rx);
}

void
batman_inc_seqno(void)
{
  seqno++;
}

PT_THREAD(batman_thread(void))
{
  PT_BEGIN(&pt_thread);
  static packet_t ogm_packet_tx;

  PT_WAIT_UNTIL(&pt_thread, send_ogm);
  send_ogm = false;

  PT_SEM_WAIT(&pt_thread, &mutex);
  ogm_t *ogm_tx = (ogm_t *) packet_get_ogm(&ogm_packet_tx);

  ogm_tx->version = BATMAN_VERSION;
  ogm_tx->flags = 0;
  ogm_tx->ttl = config_get(CONFIG_TTL);
  ogm_tx->seqno = seqno;
  ogm_tx->originator_addr = config_get(CONFIG_NODE_ADDR);
  ogm_tx->sender_addr = ogm_tx->originator_addr;
  PT_WAIT_THREAD(&pt_thread, llc_tx(&ogm_packet_tx, BROADCAST, sizeof(ogm_t)));
  batman_inc_seqno();
  PT_SEM_SIGNAL(&pt_thread, &mutex);

  PT_END(&pt_thread);
}

void
batman_one_second_elapsed(void)
{
  route_check_purge_timeout();
  send_ogm = true;
}

void
batman_init(void)
{
  PT_INIT(&pt_tx);
  PT_INIT(&pt_rx);
  PT_INIT(&pt_thread);
  PT_SEM_INIT(&mutex, 1);
  route_table = NULL;

  send_ogm = false;
  timer_register_cb(batman_one_second_elapsed);
}
