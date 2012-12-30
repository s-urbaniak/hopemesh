#include <stdbool.h>
#include <stdlib.h>
#include <curses.h>
#include <panel.h>

#include "test-util.h"

#define LINE_FORMAT "%0003d: "

#ifdef USE_NCURSES
static PANEL *stdscr_panel;
static PANEL *spi_panel;
static WINDOW *spi_win;
static uint8_t cnt;
static uint8_t row;
#endif

void
curses_spi_nl(void)
{
#ifdef USE_NCURSES
  if ((cnt % 10) == 0) {
    cnt = 0;
    wprintw(curses_get_spi_window(), "\n");
    wprintw(curses_get_spi_window(), LINE_FORMAT, row++);
  }
  cnt++;
#else
  printf("\n");
#endif
}

#ifdef USE_NCURSES
WINDOW *
curses_get_spi_window(void)
{
  return spi_win;
}
#endif

void
curses_init(void)
{
#ifdef USE_NCURSES
  initscr();
  noecho();
  cbreak();
  scrollok(stdscr, true);
  keypad(stdscr, true);
  stdscr_panel = new_panel(stdscr);
  spi_win = newwin(0, 0, 0, 0);
  scrollok(spi_win, true);
  spi_panel = new_panel(spi_win);
  timeout(3);
#endif
}

bool
curses_getch(uint8_t *dest)
{
#ifdef USE_NCURSES
  int ch;
  bool result = false;

  ch = getch();

  switch (ch) {
    case ERR:
      // no key was pressed
      break;
    case KEY_F(2):
      hide_panel(spi_panel);
      break;
    case KEY_F(3):
      show_panel(spi_panel);
      break;
    case KEY_F(12):
      curses_close();
      exit(0);
      break;
    case KEY_BACKSPACE:
      hide_panel(spi_panel);

      move(getcury(stdscr), getcurx(stdscr) - 1);
      delch();
      *dest = '\b';
      result = true;
      break;
    case '\n':
      if (!panel_hidden(spi_panel)) {
        cnt = 0;
        curses_spi_nl();
        break;
      }
    default:
      hide_panel(spi_panel);

      echochar(ch);
      *dest = (uint8_t) ch;
      result = true;
      break;
  }
  update_panels();
  doupdate();

  return result;

#else
  printf("curses_getch not supported in non-ncurses mode ...");
  exit(1);
#endif
}

void
curses_close(void)
{
#ifdef USE_NCURSES
  endwin();
#endif
}
