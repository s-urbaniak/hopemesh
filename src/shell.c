#include "shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <avr/pgmspace.h>

#include "debug.h"
#include "uart.h"
#include "watchdog.h"
#include "error.h"
#include "util.h"
#include "l4.h"
#include "batman.h"
#include "rfm12.h"
#include "spi.h"
#include "clock.h"
#include "config.h"

#define MAX_CMD_BUF 80
#define MAX_OUT_BUF 256

static PROGMEM const char pgm_bootmsg[] = "HopeMesh ready ...\n\r";

static PROGMEM const char pgm_help[] = "Help:\n\r"
    "  ?: Help\n\r"
    "  l: List nodes\n\r"
    "  c [key] [value]: Configure [key] with [value]\n\r"
    "  c: Print configuration \n\r"
    "  d: Debug info\n\r"
    "  s [node] [message]: Send a [message] to [node]\n\r";

static const char ok[] = "OK\n\r";

static PROGMEM const char pgm_debug[] = "MCUCSR: 0x%x\n\r"
    "error: src=0x%x, line=%u\n\r"
    "rfm12: 0x%x\n\r"
    "debug: 0x%x\n\r"
    "uptime: %u\n\r";

static PROGMEM const char route_entry[] =
    "target_addr: 0x%x, gw_addr: 0x%x, seqno: %u, lost: %u, cnt: %u, time: %u\n\r";

static const char txt_prompt[] = "$ ";

typedef PT_THREAD((*cmd_fn))(void);
static char out_buf[MAX_OUT_BUF], cmd_buf[MAX_CMD_BUF];
static packet_t packet_tx;
static char *cmd;
static cmd_fn cmd_fn_instance;
static struct pt pt_main, pt_cmd;

static uint16_t i;
static route_t *r;

PT_THREAD(shell_debug)(void)
{
  PT_BEGIN(&pt_cmd);
  UART_WAIT(&pt_cmd);

  sprintf_P(
      out_buf,
      pgm_debug,
      watchdog_mcucsr(), watchdog_get_source(), watchdog_get_line(), rfm12_status(), debug_get_cnt(), clock_get_time());

  UART_TX(&pt_cmd, out_buf);
  PT_END(&pt_cmd);
}

PT_THREAD(shell_list)(void)
{
  PT_BEGIN(&pt_cmd);
  UART_WAIT(&pt_cmd);

  i = 0;
  r = route_get();

  while (r != NULL) {
    sprintf_P(out_buf, route_entry,
        r->target_addr, r->gateway_addr, r->seqno, r->lost, r->cnt, r->time);
    UART_TX_NOSIGNAL(&pt_cmd, out_buf);
    r = r->next;
  }

  UART_SIGNAL(&pt_cmd);
  PT_END(&pt_cmd);
}

static unsigned short int dest;
PT_THREAD(shell_tx)(void)
{
  PT_BEGIN(&pt_cmd);

  if (2 == sscanf(cmd_buf, "s 0x%hx %[^\n]", &dest, out_buf)) {
    PT_WAIT_THREAD(&pt_cmd, l4_tx(&packet_tx, out_buf, dest));

    UART_WAIT(&pt_cmd);
    UART_TX(&pt_cmd, ok);
  }

  PT_END(&pt_cmd);
}

PT_THREAD(shell_help)(void)
{
  PT_BEGIN(&pt_cmd);

  UART_WAIT(&pt_cmd);
  UART_TX_PGM(&pt_cmd, pgm_help, out_buf);

  PT_END(&pt_cmd);
}

PT_THREAD(shell_config)(void)
{
  PT_BEGIN(&pt_cmd);
  UART_WAIT(&pt_cmd);

  unsigned short int cfg_i, cfg_val;
  if (2 == sscanf(cmd_buf, "c %hd 0x%hx", &cfg_i, &cfg_val)) {
    config_set(cfg_i, cfg_val);
    UART_TX_NOSIGNAL(&pt_cmd, ok);
  } else {
    for (i = 0; i < MAX_CONFIGS; i++) {
      sprintf(out_buf, "0x%04x\n\r", config_get(i));
      UART_TX_NOSIGNAL(&pt_cmd, out_buf);
    }
  }

  UART_SIGNAL(&pt_cmd);
  PT_END(&pt_cmd);
}

const cmd_fn
shell_data_parse(void)
{
  switch (*cmd_buf) {
    case 'l':
      return shell_list;
    case 's':
      return shell_tx;
    case 'c':
      return shell_config;
    case 'd':
      return shell_debug;
    case 'o':
      batman_one_second_elapsed();
      return NULL;
    default:
      return shell_help;
  }
}

bool
shell_data_available(void)
{
  bool result = uart_rx(cmd);

  if (result) {
    switch (*cmd) {
      case '\n': // enter
      case '\r':
        *cmd++ = '\n';
        *cmd = '\0';
        cmd = cmd_buf;
        break;

      case 0x7f: // delete
      case '\b': // backspace
        if (cmd != cmd_buf) {
          cmd--;
        }
        result = false;
        break;

      default:
        // any other character pressed
        if (cmd != cmd_buf + (MAX_CMD_BUF - 2)) {
          cmd++;
        }
        result = false;
        break;
    }
  }

  return result;
}

void
shell_init(void)
{
  cmd = cmd_buf;

  PT_INIT(&pt_main);
  PT_INIT(&pt_cmd);
}

PT_THREAD(shell(void))
{
  PT_BEGIN(&pt_main);

  UART_WAIT(&pt_main);
  UART_TX_PGM_NOSIGNAL(&pt_main, pgm_bootmsg, out_buf);
  UART_TX(&pt_main, txt_prompt);

  while (true) {
    memset(out_buf, '\0', MAX_OUT_BUF);
    PT_WAIT_UNTIL(&pt_main, shell_data_available());
    cmd_fn_instance = shell_data_parse();

    if (cmd_fn_instance != NULL) {
      PT_WAIT_THREAD(&pt_main, cmd_fn_instance());
    }

    UART_WAIT(&pt_main);
    UART_TX(&pt_main, txt_prompt);
  }

  PT_END(&pt_main);
}
