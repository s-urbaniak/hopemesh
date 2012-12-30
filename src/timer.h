#ifndef __TIMER_H__
#define __TIMER_H__

#include "pt.h"

void
timer_init(void);

typedef void
(*timer_cb)(void);

void
timer_register_cb(timer_cb cb);

PT_THREAD(
timer_thread(void));

#endif
