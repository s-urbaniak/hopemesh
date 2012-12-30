#ifndef __RXTHREAD_H__
#define __RXTHREAD_H__

#include "pt.h"

void
rx_thread_init(void);

PT_THREAD(
rx_thread(void));

#endif
