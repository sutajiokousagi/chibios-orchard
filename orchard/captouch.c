
#include "ch.h"
#include "hal.h"
#include "i2c.h"
#include "chprintf.h"

#include "orchard.h"
#include "orchard-events.h"

#include "captouch.h"
#include "gpiox.h"

#include <stdlib.h>

static I2CDriver *driver;
event_source_t captouch_changed;
static uint16_t captouch_state;

static void captouch_set(uint8_t reg, uint8_t val) {

  uint8_t tx[2] = {reg, val};

  i2cMasterTransmitTimeout(driver, touchAddr,
                           tx, sizeof(tx),
                           NULL, 0,
                           TIME_INFINITE);
}

static uint8_t captouch_get(uint8_t reg) {

  uint8_t val;

  i2cMasterTransmitTimeout(driver, touchAddr,
                           &reg, 1,
                           (void *)&val, 1,
                           TIME_INFINITE);
  return val;
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

  captouch_set(ELE_CFG, 0x00);  // register updates only allowed when in stop mode

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

  // Section F
  // Enable Auto Config and auto Reconfig
  captouch_set(ATO_CFG_CTL0, 0x0B);

  captouch_set(ATO_CFG_USL, 0xC4);  // USL 196
  captouch_set(ATO_CFG_LSL, 0x7F);  // LSL 127
  captouch_set(ATO_CFG_TGT, 0xB0);  // target level 176

  // Section E
  // Electrode Configuration
  // Enable 6 Electrodes and set to run mode
  // Set ELE_CFG to 0x00 to return to standby mode
  captouch_set(ELE_CFG, 0x0C);  // Enables all 12 Electrodes

}

static void captouch_keychange(eventid_t id) {

  uint16_t mask;
  bool changed = false;

  (void)id;

  i2cAcquireBus(driver);
  mask = captouch_read();
  i2cReleaseBus(driver);

  if (captouch_state != mask)
    changed = true;
  captouch_state = mask;

  if (changed)
    chEvtBroadcast(&captouch_changed);
}

uint16_t captouchRead(void) {
  return captouch_state;
}

void captouchStart(I2CDriver *i2cp) {

  driver = i2cp;

  i2cAcquireBus(driver);
  captouch_config();
  i2cReleaseBus(driver);

  chEvtObjectInit(&captouch_changed);

  gpioxSetPadMode(GPIOX, 7, GPIOX_IN | GPIOX_IRQ_FALLING | GPIOX_PULL_UP);
  evtTableHook(orchard_events, gpiox_falling[7], captouch_keychange);
}

static void dump(uint8_t *byte, uint32_t count) {
  uint32_t i;

  for( i = 0; i < count; i++ ) {
    if( (i % 16) == 0 ) {
      chprintf(stream, "\n\r%08x: ", i );
    }
    chprintf(stream, "%02x ", byte[i] );
  }
  chprintf(stream, "\n\r" );
}

void captouchPrint(uint8_t reg) {
  uint8_t val;

  i2cAcquireBus(driver);
  val = captouch_get(reg);
  i2cReleaseBus(driver);
  
  chprintf( stream, "Value at %02x: %02x\n\r", reg, val );
}

void captouchSet(uint8_t adr, uint8_t dat) {
  chprintf( stream, "Writing %02x into %02x\n\r", dat, adr );

  i2cAcquireBus(driver);
  captouch_set(adr, dat);
  i2cReleaseBus(driver);
}

void captouchDebug(void) {
  uint8_t i;
  uint8_t val[128];

  i2cAcquireBus(driver);
  captouch_set(ATO_CFG_CTL0, 0x0B);

  captouch_set(ATO_CFG_USL, 196);  // USL
  captouch_set(ATO_CFG_LSL, 127);  // LSL
  captouch_set(ATO_CFG_TGT, 176);  // target level
  i2cReleaseBus(driver);

  for( i = 0; i < 128; i++ ) {
    i2cAcquireBus(driver);
    val[i] = captouch_get(i);
    i2cReleaseBus(driver);
  }
  dump( val, 128 );

#if 0
  chprintf( stream, "ELE OOR/TCH:\n\r" );
  for( i= 0; i < 4; i++ ) {
    i2cAcquireBus(driver);
    val = captouch_get(i);
    i2cReleaseBus(driver);
    chprintf( stream, "%02x: %d\n\r", i, val );
  }

  chprintf( stream, "HD/CL/DL/HD:\n\r" );
  for( i= 0x2B; i < 0x33; i++ ) {
    i2cAcquireBus(driver);
    val = captouch_get(i);
    i2cReleaseBus(driver);
    chprintf( stream, "%02x: %d\n\r", i, val );
  }

  chprintf( stream, "ELE T/R:\n\r" );
  for( i= 0x41; i < 0x59; i++ ) {
    i2cAcquireBus(driver);
    val = captouch_get(i);
    i2cReleaseBus(driver);
    chprintf( stream, "%d ", val );
  }

  chprintf( stream, "\n\rCFG:\n\r" );
  for( i= 0x5D; i < 0x5F; i++ ) {
    i2cAcquireBus(driver);
    val = captouch_get(i);
    i2cReleaseBus(driver);
    chprintf( stream, "%02x: %d\n\r", i, val );
  }

  chprintf( stream, "ATO:\n\r" );
  for( i= 0x7B; i < 0x80; i++ ) {
    i2cAcquireBus(driver);
    val = captouch_get(i);
    i2cReleaseBus(driver);
    chprintf( stream, "%02x: %d\n\r", i, val );
  }
#endif
}

