#include "debug.h"

static uint16_t cnt = 0;

void
debug_cnt(void)
{
  cnt++;
}

uint16_t
debug_get_cnt(void)
{
  return cnt;
}
