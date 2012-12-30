#ifndef __TEST_UTIL__
#define __TEST_UTIL__

#include <stdint.h>
#include <curses.h>

#ifdef USE_NCURSES
#define _printf(fmt, args...) printw(fmt, ## args)
#else
#define _printf(fmt, args...) printf(fmt, ## args)
#endif

#ifdef USE_NCURSES
#define _spi_printf(fmt, args...) \
    curses_spi_nl(); \
    wprintw(curses_get_spi_window(), fmt, ## args);
#else
#define _spi_printf(fmt, args...) \
    printf(fmt, ## args); \
    printf("\n");
#endif

void
curses_init(void);

bool
curses_getch(uint8_t *dest);

void
curses_close(void);

#ifdef USE_NCURSES
WINDOW *
curses_get_spi_window(void);
#endif

void
curses_spi_nl(void);

#endif
