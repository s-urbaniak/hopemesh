#ifndef __HAMMING_H__
#define __HAMMING_H__

#include <avr/pgmspace.h>

extern const prog_uint8_t hamming_enc[16];

extern const prog_uint8_t hamming_dec[256];

#define hamming_enc_high(data) pgm_read_byte(&hamming_enc[(data>>4)&0x0f])
#define hamming_enc_low(data)  pgm_read_byte(&hamming_enc[data&0x0f])

#define hamming_dec_high(data) pgm_read_byte(&hamming_dec[data])<<4
#define hamming_dec_low(data)  pgm_read_byte(&hamming_dec[data])

#endif
