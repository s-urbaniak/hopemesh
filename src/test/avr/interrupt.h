#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include "mock-interrupt.h"

#define ISR(a) void a(void)

#define CALL_ISR(a)	  \
  if (is_int_enabled()) { \
    a();		  \
  }

void
cli(void);
void
sei(void);

#endif
