#ifndef __RFM12_H__
#define __RFM12_H__

#include <stdbool.h>
#include <stdint.h>
#include "pt.h"

void
rfm12_init(void);

uint16_t
rfm12_status(void);

uint8_t
rfm12_status_fast(void);

bool
rfm12_is_carrier_free(void);

void
rfm12_enable_tx(void);

PT_THREAD(
rfm12_lock(void));

void rfm12_release(void);

typedef enum
{
  RFM12_RX_OK, RFM12_RX_LOST_SIGNAL
} rfm12_status_t;

typedef struct
{
  rfm12_status_t status;
  uint8_t payload;
} rfm12_rx_t;

#endif
