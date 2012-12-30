#include <stdlib.h>

#include "ringbuf.h"
#include "watchdog.h"
#include "error.h"

struct ringbuf_t
{
  uint8_t *buf;
  uint8_t *start;
  uint8_t *end;
  uint16_t max;
  uint16_t size;
};

uint8_t
ringbuf_size(volatile ringbuf_t *buf)
{
  return buf->size;
}

ringbuf_t *
ringbuf_new(uint16_t max)
{
  ringbuf_t *new_buf = malloc(sizeof(ringbuf_t));
  if (new_buf == NULL) {
    watchdog_error(ERR_RINGBUF);
  }

  uint8_t *buf = malloc(sizeof(uint8_t) * max);
  if (buf == NULL) {
    watchdog_error(ERR_RINGBUF);
  }
  new_buf->buf = buf;
  new_buf->max = max;

  ringbuf_clear(new_buf);
  return new_buf;
}

void
ringbuf_free(volatile ringbuf_t *buf)
{
  free((void *) buf->buf);
  free((void *) buf);
}

void
ringbuf_clear(volatile ringbuf_t *buf)
{
  buf->size = 0;
  buf->start = buf->buf;
  buf->end = buf->buf;
}

bool
ringbuf_add(volatile ringbuf_t *buf, uint8_t what)
{
  if (buf->size == buf->max) {
    return false;
  }

  *(buf->end++) = what;
  buf->size++;

  if (buf->end == (buf->buf + buf->max)) {
    buf->end = buf->buf;
  }

  return true;
}

bool
ringbuf_remove(volatile ringbuf_t *buf, uint8_t *what)
{
  if (buf->size == 0) {
    return false;
  }

  *what = *(buf->start++);
  buf->size--;

  if (buf->start == (buf->buf + buf->max)) {
    buf->start = buf->buf;
  }

  return true;
}
