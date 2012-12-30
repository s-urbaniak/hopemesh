#include "clock.h"
#include "timer.h"

static uint16_t time;

static void
clock_update(void)
{
  time++;
}

uint16_t
clock_get_time(void)
{
  return time;
}

void
clock_init(void)
{
  timer_register_cb(&clock_update);
}
