#include <stdbool.h>
#include <stdio.h>

#include <avr/interrupt.h>

#include "error.h"
#include "rfm12.h"
#include "timer.h"
#include "uart.h"
#include "error.h"
#include "watchdog.h"

#define MAX_TIMER_CB 2

static uint8_t timer_cnt;
static bool second_notify;
static struct pt pt_timer;
static volatile timer_cb cb_list[MAX_TIMER_CB + 1];

ISR(SIG_OVERFLOW0)
{
  timer_cnt++;

  if (timer_cnt == 61) {
    second_notify = true;
    timer_cnt = 0;
  }
}

static inline bool
one_second_elapsed(void)
{
  if (second_notify) {
    second_notify = false;
    return true;
  } else {
    return false;
  }
}

static volatile timer_cb *cb;

void
timer_register_cb(timer_cb new_cb)
{
  cb = cb_list;
  while (*cb != NULL) {
    if (cb++ == &cb_list[MAX_TIMER_CB - 1]) {
      watchdog_error(ERR_TIMER);
    }
  }

  *cb = new_cb;
  cb++;
  cb = NULL;
}

PT_THREAD(timer_thread(void))
{
  PT_BEGIN(&pt_timer);

  PT_WAIT_UNTIL(&pt_timer, one_second_elapsed());

  cb = cb_list;
  while (*cb != NULL) {
    (*cb++)();
  }

  PT_END(&pt_timer);
}

void
timer_init(void)
{
  PT_INIT(&pt_timer);
  timer_cnt = 0;
  second_notify = false;
  cb_list[0] = NULL;

  TCCR0 = (1 << CS02) | (1 << CS00);
  TIMSK |= (1 << TOIE0);
}
