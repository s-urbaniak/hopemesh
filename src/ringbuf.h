#ifndef __RINGBUF_H__
#define __RINGBUF_H__

#include <stdint.h>
#include <stdbool.h>
#include "watchdog.h"

#define ringbuf_add_err(buf, byte, err_source) \
  if (!ringbuf_add(buf, byte)) watchdog_error(err_source)

#define ringbuf_remove_err(buf, byte, err_source) \
  if (!ringbuf_remove(buf, byte)) watchdog_error(err_source)

typedef struct ringbuf_t ringbuf_t;

ringbuf_t *
ringbuf_new(uint16_t max);

void
ringbuf_free(volatile ringbuf_t *buf);

void
ringbuf_clear(volatile ringbuf_t *buf);

bool
ringbuf_add(volatile ringbuf_t *buf, uint8_t what);

bool
ringbuf_remove(volatile ringbuf_t *buf, uint8_t *what);

uint8_t
ringbuf_size(volatile ringbuf_t *buf);

#endif
