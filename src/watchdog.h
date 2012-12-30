#include <stdint.h>
#include <stdbool.h>

#include "pt.h"

#define watchdog_error(a) watchdog_error_line(a, __LINE__)

void
watchdog_init(void);

void
watchdog_error_line(uint16_t source, uint16_t line);

void
watchdog(void);

uint8_t
watchdog_mcucsr(void);

uint16_t
watchdog_get_source(void);

uint16_t
watchdog_get_line(void);

bool
watchdog_happened(void);
