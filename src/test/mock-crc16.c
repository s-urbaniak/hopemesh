#include "util/crc16.h"

uint16_t
_crc16_update(uint16_t __crc, uint8_t __data)
{
  uint8_t i;

  __crc ^= __data;
  for (i = 0; i < 8; ++i) {
    if (__crc & 1)
      __crc = (__crc >> 1) ^ 0xA001;
    else
      __crc = (__crc >> 1);
  }

  return __crc;
}
