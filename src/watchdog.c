#include <avr/wdt.h>
#include <avr/eeprom.h>

#include "watchdog.h"

#define EEPROM_DEFAULT 0xffff

static uint16_t EEMEM eeprom_source = EEPROM_DEFAULT;
static uint16_t EEMEM eeprom_line = EEPROM_DEFAULT;

static uint16_t last_source;
static uint16_t last_line;

uint8_t mcucsr_mirror __attribute__ ((section (".noinit")));

void
get_mcucsr(void) __attribute__((naked))
__attribute__((section(".init3")));

void
get_mcucsr(void)
{
  mcucsr_mirror = MCUCSR;
  MCUCSR = 0;
  wdt_disable();
}

uint16_t
watchdog_get_source(void)
{
  return last_source;
}

uint16_t
watchdog_get_line(void)
{
  return last_line;
}

uint8_t
watchdog_mcucsr(void)
{
  return mcucsr_mirror;
}

bool
watchdog_happened(void)
{
  return bit_is_set(mcucsr_mirror, WDRF);
}

void
watchdog_error_line(uint16_t source, uint16_t line)
{
  // write error source and line to eeprom
  eeprom_busy_wait ();eeprom_write_word(&eeprom_source, source);
  eeprom_busy_wait ();eeprom_write_word(&eeprom_line, line);

  // this endless loop prevents other code to be executed and
  // lets the watchdog time out
  while (1)
    ;
}

void
watchdog_init(void)
{
  get_mcucsr();
  wdt_enable(WDTO_500MS);

  eeprom_busy_wait ();
  last_source = eeprom_read_word (&eeprom_source);
  eeprom_busy_wait ();
  last_line = eeprom_read_word (&eeprom_line);

  eeprom_busy_wait ();eeprom_write_word(&eeprom_source, EEPROM_DEFAULT);
  eeprom_busy_wait ();
  eeprom_write_word(&eeprom_line, EEPROM_DEFAULT);
}

void
watchdog(void)
{
  wdt_reset();
}
