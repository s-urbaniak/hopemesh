/* Copyright (c) 2002 - 2007  Marek Michalkiewicz
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in
 the documentation and/or other materials provided with the
 distribution.
 * Neither the name of the copyright holders nor the names of
 contributors may be used to endorse or promote products derived
 from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE. */

#ifndef __PGMSPACE_H_
#define __PGMSPACE_H_ 1

#define __need_size_t
#include <inttypes.h>
#include <stddef.h>
#include <avr/io.h>
#include <string.h>

#ifndef __ATTR_CONST__
#define __ATTR_CONST__
#endif

#ifndef __ATTR_PROGMEM__
#define __ATTR_PROGMEM__
#endif

#ifndef __ATTR_PURE__
#define __ATTR_PURE__
#endif

/**
 \ingroup avr_pgmspace
 \def PROGMEM

 Attribute to use in order to declare an object being located in
 flash ROM.
 */
#define PROGMEM

#ifdef __cplusplus
extern "C"
{
#endif

  typedef void prog_void PROGMEM;
  typedef char prog_char PROGMEM;
  typedef unsigned char prog_uchar PROGMEM;

  typedef int8_t prog_int8_t PROGMEM;
  typedef uint8_t prog_uint8_t PROGMEM;
  typedef int16_t prog_int16_t PROGMEM;
  typedef uint16_t prog_uint16_t PROGMEM;
  typedef int32_t prog_int32_t PROGMEM;
  typedef uint32_t prog_uint32_t PROGMEM;
#if !__USING_MINT8
  typedef int64_t prog_int64_t PROGMEM;
  typedef uint64_t prog_uint64_t PROGMEM;
#endif

  /* Although in C, we can get away with just using __c, it does not work in
   C++. We need to use &__c[0] to avoid the compiler puking. Dave Hylands
   explaned it thusly,

   Let's suppose that we use PSTR("Test"). In this case, the type returned
   by __c is a prog_char[5] and not a prog_char *. While these are
   compatible, they aren't the same thing (especially in C++). The type
   returned by &__c[0] is a prog_char *, which explains why it works
   fine. */

# define PSTR(s) ((const PROGMEM char *)(s))

  /** \ingroup avr_pgmspace
   \def pgm_read_byte(address_short)
   Read a byte from the program space with a 16-bit (near) address. 

   \note The address is a byte address. 
   The address is in the program space. */

#define pgm_read_byte(address_short)    (*address_short)

  /** \ingroup avr_pgmspace
   \def pgm_read_word(address_short)
   Read a word from the program space with a 16-bit (near) address. 

   \note The address is a byte address. 
   The address is in the program space. */

#define pgm_read_word(address_short)    (*address_short)

  /** \ingroup avr_pgmspace
   \def pgm_read_dword(address_short)
   Read a double word from the program space with a 16-bit (near) address. 

   \note The address is a byte address. 
   The address is in the program space. */

#define pgm_read_dword(address_short)   (*address_short)

  /** \ingroup avr_pgmspace
   \def pgm_read_float(address_short)
   Read a float from the program space with a 16-bit (near) address. 

   \note The address is a byte address. 
   The address is in the program space. */

#define pgm_read_float(address_short)   (*address_short)

  /** \ingroup avr_pgmspace
   \def PGM_P

   Used to declare a variable that is a pointer to a string in program
   space. */

#ifndef PGM_P
#define PGM_P const prog_char *
#endif

  /** \ingroup avr_pgmspace
   \def PGM_VOID_P

   Used to declare a generic pointer to an object in program space. */

#ifndef PGM_VOID_P
#define PGM_VOID_P const prog_void *
#endif

#define strcpy_P(a, b) strcpy(a, b)
#define sprintf_P(str, format, args...) sprintf(str, format, ## args)

  extern PGM_VOID_P memchr_P(PGM_VOID_P, int __val, size_t __len)
      __ATTR_CONST__;
  extern int
  memcmp_P(const void *, PGM_VOID_P, size_t) __ATTR_PURE__;
  extern void *
  memccpy_P(void *, PGM_VOID_P, int __val, size_t);
  extern void *
  memcpy_P(void *, PGM_VOID_P, size_t);
  extern void *
  memmem_P(const void *, size_t, PGM_VOID_P, size_t) __ATTR_PURE__;
  extern PGM_VOID_P memrchr_P(PGM_VOID_P, int __val, size_t __len)
      __ATTR_CONST__;
  extern char *
  strcat_P(char *, PGM_P);
  extern PGM_P strchr_P(PGM_P, int __val) __ATTR_CONST__;
  extern PGM_P strchrnul_P(PGM_P, int __val) __ATTR_CONST__;
  extern int
  strcmp_P(const char *, PGM_P) __ATTR_PURE__;
  extern int
  strcasecmp_P(const char *, PGM_P) __ATTR_PURE__;
  extern char *
  strcasestr_P(const char *, PGM_P) __ATTR_PURE__;
  extern size_t
  strcspn_P(const char *__s, PGM_P __reject) __ATTR_PURE__;
  extern size_t
  strlcat_P(char *, PGM_P, size_t);
  extern size_t
  strlcpy_P(char *, PGM_P, size_t);
  extern size_t
  strlen_P(PGM_P) __ATTR_CONST__; /* program memory can't change */
  extern size_t
  strnlen_P(PGM_P, size_t) __ATTR_CONST__; /* program memory can't change */
  extern int
  strncmp_P(const char *, PGM_P, size_t) __ATTR_PURE__;
  extern int
  strncasecmp_P(const char *, PGM_P, size_t) __ATTR_PURE__;
  extern char *
  strncat_P(char *, PGM_P, size_t);
  extern char *
  strncpy_P(char *, PGM_P, size_t);
  extern char *
  strpbrk_P(const char *__s, PGM_P __accept) __ATTR_PURE__;
  extern PGM_P strrchr_P(PGM_P, int __val) __ATTR_CONST__;
  extern char *
  strsep_P(char **__sp, PGM_P __delim);
  extern size_t
  strspn_P(const char *__s, PGM_P __accept) __ATTR_PURE__;
  extern char *
  strstr_P(const char *, PGM_P) __ATTR_PURE__;
  extern char *
  strtok_P(char *__s, PGM_P __delim);
  extern char *
  strtok_rP(char *__s, PGM_P __delim, char **__last);

#ifdef __cplusplus
}
#endif

#endif /* __PGMSPACE_H_ */
