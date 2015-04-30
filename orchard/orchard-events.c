#include "ch.h"
#include "hal.h"
#include "ext.h"
#include "orchard-events.h"

event_source_t ble_rdy;
event_source_t rf_pkt_rdy;

static void ble_rdyn_cb(EXTDriver *extp, expchannel_t channel) {

  (void)extp;
  (void)channel;

  chSysLockFromISR();
  chEvtBroadcastI(&ble_rdy);
  chSysUnlockFromISR();
}

static void rf_pkt_rdy_cb(EXTDriver *extp, expchannel_t channel) {

  (void)extp;
  (void)channel;

  chSysLockFromISR();
  chEvtBroadcastI(&rf_pkt_rdy);
  chSysUnlockFromISR();
}

static const EXTConfig ext_config = {
  {
    {EXT_CH_MODE_FALLING_EDGE | EXT_CH_MODE_AUTOSTART, ble_rdyn_cb, PORTC, 3},
    {EXT_CH_MODE_RISING_EDGE | EXT_CH_MODE_AUTOSTART, rf_pkt_rdy_cb, PORTC, 4},
  }
};


void orchardEventsStart(void) {

  chEvtObjectInit(&ble_rdy);
  chEvtObjectInit(&rf_pkt_rdy);
	extStart(&EXTD1, &ext_config);
}
