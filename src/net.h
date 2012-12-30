#ifndef __NET_H__
#define __NET_H__

#include <stdint.h>

typedef uint16_t addr_t;

#define BATMAN_VERSION 1

#define OGM_FLAG_IS_DIRECT 0
#define OGM_FLAG_UNIDIRECTIONAL 1

#define BI_LINK_TIMEOUT 10
#define PURGE_TIMEOUT 10

typedef enum
{
  UNICAST, BROADCAST
} llc_type_t;

typedef struct
{
  llc_type_t type :4;
  uint16_t len :12;
  uint16_t crc;
} llc_t;
#define LLC_HEADER_SIZE (2 + sizeof(uint16_t))

typedef struct
{
  uint8_t version :4;
  uint8_t flags :4;
  uint8_t ttl;
  addr_t originator_addr;
  addr_t target_addr;
  addr_t sender_addr;
  addr_t gateway_addr;
} batman_t;
#define BATMAN_HEADER_SIZE (2 + 4 * sizeof(addr_t))

typedef struct
{
  uint8_t version :4;
  uint8_t flags :4;
  uint8_t ttl;
  uint16_t seqno;
  addr_t originator_addr;
  addr_t sender_addr;
} ogm_t;
#define OGM_HEADER_SIZE (1 + sizeof(uint8_t) + sizeof(uint16_t) + 2 * sizeof(addr_t))

#define PACKET_MAX_SIZE 256
#define LLC_OFFSET LLC_HEADER_SIZE
#define BATMAN_OFFSET (LLC_OFFSET + BATMAN_HEADER_SIZE)

#define packet_get_llc(p)     ((p)->data)
#define packet_get_batman(p)  ((p)->data + LLC_OFFSET)
#define packet_get_l4(p)      ((p)->data + BATMAN_OFFSET)

#define packet_get_ogm(p)     ((p)->data + LLC_OFFSET)

typedef struct
{
  uint8_t data[PACKET_MAX_SIZE];
} packet_t;

#endif
