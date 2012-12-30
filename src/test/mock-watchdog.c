#include <stdlib.h>
#include <stdio.h>

#include "../watchdog.h"

#include "test-util.h"

void
watchdog_init(void)
{
  _printf("watchdog_init\n");
}

void
watchdog_error_line(uint16_t source, uint16_t line)
{
  _printf("Aborting program, source: %d, line: %d\n", source, line);
  exit(0);
}

void
watchdog(void)
{
}

uint8_t
watchdog_mcucsr(void)
{
  return 0x8;
}

uint16_t
watchdog_get_source(void)
{
  return 0;
}

uint16_t
watchdog_get_line(void)
{
  return 0;
}

bool
watchdog_happened(void)
{
  return true;
}
