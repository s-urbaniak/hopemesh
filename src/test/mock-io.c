#include "avr/io.h"

#include <stdio.h>

#include "test-util.h"

uint8_t io8_reg[256];

void
io8_reg_dump(void)
{
  _printf("io register dump:\n");
  for (int i = 0; i < 256; i++) {
    _printf("0x%x = 0x%x\n", i, io8_reg[i]);
  }
}
