#ifndef __SHELL_H__
#define __SHELL_H__

#include "pt.h"

void
shell_init(void);

PT_THREAD(
shell(void));

#endif
