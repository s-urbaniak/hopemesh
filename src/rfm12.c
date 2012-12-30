#include "rfm12.h"

#include <avr/interrupt.h>
#include "config.h"
#include "pt.h"
#include "pt-sem.h"
#include "spi.h"
#include "mac.h"
#include "debug.h"

#define rfm12_enable_nirq_isr() (GICR |= (1<<RFM12_INT_NIRQ))
#define rfm12_disable_nirq_isr() (GICR &= ~(1<<RFM12_INT_NIRQ))
#define rfm12_cmd16(data) spi_tx16(data, _SS_RADIO)
#define rfm12_cmd(data) spi_tx(data, _SS_RADIO)

#define FIFO_FILL_OFF 0xca81
#define FIFO_FILL_ON 0xca83
#define CONFIG_TXREG_OFF 0x8067
#define CONFIG_TXREG_ON 0x80e7
#define PM_RX_ON 0x82C8
#define PM_RXTX_OFF 0x8209
#define PM_TX_ON 0x8238
#define CMD_TX 0xb800
#define CMD_RX 0xb000
#define CMD_STATUS 0x0000
#define CMD_STATUS8 0x00

#define CMD8_RSSI 0x01
#define CMD8_FFIT 0x80

typedef enum
{
  STATE_TX, STATE_RX
} radio_state_t;

static volatile uint8_t last_status_fast;
static struct pt pt_nirq, pt_tx;
static volatile struct pt_sem mutex;
static radio_state_t radio_state;

uint8_t
rfm12_status_fast(void)
{
  last_status_fast = rfm12_cmd(0x00);
  return last_status_fast;
}

uint16_t
rfm12_status(void)
{
  return rfm12_cmd16(0x0000);
}

void
rfm12_reset_fifo(void)
{
  rfm12_cmd16(FIFO_FILL_OFF);
  rfm12_cmd16(FIFO_FILL_ON);
}

static bool
rfm12_tx_cb(void)
{
  uint8_t data;
  bool fin = mac_tx_rfm12(&data);
  rfm12_cmd16(CMD_TX | data);
  return fin;
}

static bool
rfm12_rx_cb(void)
{
  uint8_t byte = (uint8_t) rfm12_cmd16(CMD_RX);
  rfm12_rx_t packet;
  packet.payload = byte;

  bool _rssi = config_get(CONFIG_FLAGS) & (1 << CONFIG_FLAG_RSSI_DETECTION);
  if (_rssi) {
    _rssi = rfm12_is_carrier_free();
  }

  if (_rssi) {
    // signal lost (rssi is not set) -> abort reception
    packet.status = RFM12_RX_LOST_SIGNAL;
    mac_rx_rfm12(&packet);
    return true;
  } else {
    packet.status = RFM12_RX_OK;
    return mac_rx_rfm12(&packet);
  }
}

bool
rfm12_is_carrier_free(void)
{
  return !(rfm12_status_fast() & CMD8_RSSI);
}

static bool
rfm12_is_fifo_ready(void)
{
  return (rfm12_status_fast() & CMD8_FFIT);
}

static void
rfm12_enable_rx(void)
{
  radio_state = STATE_RX;
  rfm12_cmd16(CONFIG_TXREG_OFF);
  rfm12_cmd16(PM_RX_ON);
  rfm12_reset_fifo();
}

void
rfm12_enable_tx(void)
{
  radio_state = STATE_TX;
  rfm12_cmd16(CONFIG_TXREG_ON);
  rfm12_cmd16(PM_TX_ON);
  PT_INIT(&pt_nirq);
}

static inline
PT_THREAD(rfm12_nirq_thread(void))
{
  PT_BEGIN(&pt_nirq);

  switch (radio_state) {
    case STATE_TX:
      PT_WAIT_UNTIL(&pt_nirq, rfm12_tx_cb());
      PT_SEM_SIGNAL(&pt_nirq, &mutex);
      break;
    case STATE_RX:
      PT_SEM_WAIT(&pt_nirq, &mutex);
      PT_WAIT_UNTIL(&pt_nirq, rfm12_rx_cb());
      PT_SEM_SIGNAL(&pt_nirq, &mutex);
      break;
  }

  rfm12_enable_rx();

  PT_END(&pt_nirq);
}

ISR(SIG_RFM12_NIRQ)
{
  if (rfm12_is_fifo_ready()) {
    rfm12_nirq_thread();
  }
}

void
rfm12_init(void)
{
  rfm12_disable_nirq_isr();

  PT_INIT(&pt_nirq);
  PT_INIT(&pt_tx);
  PT_SEM_INIT(&mutex, 1);

  // 868.3MHz frequency
  rfm12_cmd16(0xA640);
  // data rate: 0xC6FF = 300bps | 0xC67F = 2.7kbps | 0xC647 = 4.8kbps | 0xC606 = 57.6kbps
  rfm12_cmd16(0xC605);
  // VDI at pin16, FAST VDI response, 134kHz baseband bandwidth, 0dBm LNA gain, -91dBm DRSSI thershold
  rfm12_cmd16(0x94A5);
  // (?) data filtering, no idea what that does; auto lock, no fast mode, digital filter, DQD4 dhreshold: 00
  rfm12_cmd16(0xC2AC);
  rfm12_cmd16(0xCA81);
  // sync aka grouping; no sync, sync on at 0x2DD4;
  rfm12_cmd16(0xCED4);
  // (?) AFC setting: @PWR,NO RSTRIC,!st,!fi,OE,EN
  rfm12_cmd16(0xC483);
  // no modulatin polarity, 90kHz frequency toleration, max output power
  rfm12_cmd16(0x9850);
  // PLL - auto frequency regulation 5/10MHz, low power consumption, no phase detector delay, no dithering, 86.2 PLL bandwidth
  rfm12_cmd16(0xCC77);
  // wake up timer - NOT USED
  rfm12_cmd16(0xE000);
  // low-duty cycle command - NOT USED
  rfm12_cmd16(0xC800);
  // 1.66MHz clock pin frequency, 2.2V low battery detector threshold
  rfm12_cmd16(0xC040);

  rfm12_enable_rx();
  rfm12_enable_nirq_isr();
}

void
rfm12_release(void)
{
  rfm12_enable_nirq_isr();
}

PT_THREAD(rfm12_lock(void))
{
  PT_BEGIN(&pt_tx);

  PT_SEM_WAIT_ATOMIC(&pt_tx, &mutex);
  rfm12_disable_nirq_isr();
  sei();

  PT_END(&pt_tx);
}
