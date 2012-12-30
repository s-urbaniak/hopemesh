#include "config.h"
#include <avr/eeprom.h>

static uint16_t int_settings[MAX_CONFIGS];

#ifndef NODE_ADDR
#define NODE_ADDR 0xffff
#endif

static uint16_t EEMEM int_settings_eemem[MAX_CONFIGS] = {
// CONFIG_NODE_ADDR
    NODE_ADDR,
    // CONFIG_TTL
    10,
    // CONFIG_FLAGS
    0x0000
    // | (1 << CONFIG_FLAG_RSSI_DETECTION)
        | (1 << CONFIG_FLAG_COLLISION_DETECTION) };

uint16_t
config_get(uint8_t index)
{
  return int_settings[index];
}

void
config_set(uint8_t index, uint16_t value)
{
  int_settings[index] = value;
  eeprom_write_word(&int_settings_eemem[index], value);
}

void
config_init(void)
{
  for (uint8_t i = 0; i < MAX_CONFIGS; i++) {
    int_settings[i] = eeprom_read_word(&int_settings_eemem[i]);
  }
}
