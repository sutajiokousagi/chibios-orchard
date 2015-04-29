
#include "ch.h"
#include "hal.h"
#include "i2c.h"
#include "chprintf.h"

#include "orchard.h"
#include "orchard-events.h"

#include "captouch.h"
#include "gpiox.h"

static I2CDriver *driver;
event_source_t captouch_release;
event_source_t captouch_press;
static uint16_t captouch_state;

static void captouch_set(uint8_t reg, uint8_t val) {

  uint8_t tx[2] = {reg, val};

  i2cMasterTransmitTimeout(driver, touchAddr,
                           tx, sizeof(tx),
                           NULL, 0,
                           TIME_INFINITE);
}

static uint16_t captouch_read(void) {

  uint16_t val;
  uint8_t reg;

  reg = ELE_TCHL;
  i2cMasterTransmitTimeout(driver, touchAddr,
                           &reg, 1,
                           (void *)&val, 2,
                           TIME_INFINITE);
  return val;
}

// Configure all registers as described in AN3944
static void captouch_config(void) {

  // Section A
  // This group controls filtering when data is > baseline.
  captouch_set(MHD_R, 0x01);
  captouch_set(NHD_R, 0x01);
  captouch_set(NCL_R, 0x00);
  captouch_set(FDL_R, 0x00);

  // Section B
  // This group controls filtering when data is < baseline.
  captouch_set(MHD_F, 0x01);
  captouch_set(NHD_F, 0x01);
  captouch_set(NCL_F, 0xFF);
  captouch_set(FDL_F, 0x02);

  // Section C
  // This group sets touch and release thresholds for each electrode
  captouch_set(ELE0_T, TOU_THRESH);
  captouch_set(ELE0_R, REL_THRESH);
  captouch_set(ELE1_T, TOU_THRESH);
  captouch_set(ELE1_R, REL_THRESH);
  captouch_set(ELE2_T, TOU_THRESH);
  captouch_set(ELE2_R, REL_THRESH);
  captouch_set(ELE3_T, TOU_THRESH);
  captouch_set(ELE3_R, REL_THRESH);
  captouch_set(ELE4_T, TOU_THRESH);
  captouch_set(ELE4_R, REL_THRESH);
  captouch_set(ELE5_T, TOU_THRESH);
  captouch_set(ELE5_R, REL_THRESH);
  captouch_set(ELE6_T, TOU_THRESH);
  captouch_set(ELE6_R, REL_THRESH);
  captouch_set(ELE7_T, TOU_THRESH);
  captouch_set(ELE7_R, REL_THRESH);
  captouch_set(ELE8_T, TOU_THRESH);
  captouch_set(ELE8_R, REL_THRESH);
  captouch_set(ELE9_T, TOU_THRESH);
  captouch_set(ELE9_R, REL_THRESH);
  captouch_set(ELE10_T, TOU_THRESH);
  captouch_set(ELE10_R, REL_THRESH);
  captouch_set(ELE11_T, TOU_THRESH);
  captouch_set(ELE11_R, REL_THRESH);

  // Section D
  // Set the Filter Configuration
  // Set ESI2
  captouch_set(FIL_CFG, 0x04);

  // Section E
  // Electrode Configuration
  // Enable 6 Electrodes and set to run mode
  // Set ELE_CFG to 0x00 to return to standby mode
  captouch_set(ELE_CFG, 0x0C);  // Enables all 12 Electrodes

  // Section F
  // Enable Auto Config and auto Reconfig
  captouch_set(ATO_CFG_CTL0, 0x0B);

  captouch_set(ATO_CFG_USL, 196);  // USL
  captouch_set(ATO_CFG_LSL, 127);  // LSL
  captouch_set(ATO_CFG_TGT, 176);  // target level
}

static void captouch_keychange(eventid_t id) {

  uint16_t mask;
  int i;

  (void)id;

  i2cAcquireBus(driver);
  mask = captouch_read();
  i2cReleaseBus(driver);

  for (i = 0; i < 14; i++) {
    int bit = (1 << i);

    if ((mask & bit) && !(captouch_state & bit)) {
      chprintf(stream, "Key %d down\r\n", i);
      chEvtBroadcast(&captouch_press);
    }
    else if (!(mask & bit) && (captouch_state & bit)) {
      chprintf(stream, "Key %d up\r\n", i);
      chEvtBroadcast(&captouch_release);
    }
  }
  captouch_state = mask;
}

uint16_t captouchRead(void) {
  return captouch_state;
}

void captouchStart(I2CDriver *i2cp) {

  driver = i2cp;

  i2cAcquireBus(driver);
  captouch_config();
  i2cReleaseBus(driver);

  chEvtObjectInit(&captouch_press);
  chEvtObjectInit(&captouch_release);

  gpioxSetPadMode(GPIOX, 7, GPIOX_IN | GPIOX_IRQ_FALLING | GPIOX_PULL_UP);
  evtTableHook(orchard_events, gpiox_falling[7], captouch_keychange);
}
