
#include "ch.h"
#include "hal.h"
#include "i2c.h"

#include "charger.h"
#include "orchard.h"

static I2CDriver *driver;
static chargerIntent chgIntent = CHG_IDLE;

static void charger_set(uint8_t reg, uint8_t val) {

  uint8_t tx[2] = {reg, val};

  i2cAcquireBus(driver);
  i2cMasterTransmitTimeout(driver, chargerAddr,
                           tx, sizeof(tx),
                           NULL, 0,
                           TIME_INFINITE);
  i2cReleaseBus(driver);
}

static void charger_get(uint8_t adr, uint8_t *data) {
  uint8_t tx[1];
  uint8_t rx[1];

  tx[0] = adr;

  i2cAcquireBus(driver);
  i2cMasterTransmitTimeout(driver, chargerAddr,
                           tx, sizeof(tx),
                           rx, sizeof(rx),
                           TIME_INFINITE);
  i2cReleaseBus(driver);

  *data = rx[0];
}


static void do_charger_watchdog(void) {
  switch( chgIntent ) {
  case CHG_CHARGE:
    charger_set(0x00, 0x80); // command for charge mode
    break;

  case CHG_BOOST:
    charger_set(0x00, 0xC0); // command for boost mode
    break;
    
  case CHG_IDLE:
  default:
    break;
    // do nothing
  }
}

static THD_WORKING_AREA(waChargerWatchdogThread, 192);
static THD_FUNCTION(charger_watchdog_thread, arg) {
  (void)arg;

  chRegSetThreadName("Charger watchdog thread");
  while (1) {
    chThdSleepMilliseconds(1000);
    do_charger_watchdog();
  }

  return;
}

void chargerStart(I2CDriver *i2cp) {

  driver = i2cp;

  // 0x6 0xb0    -- 6 hour fast charger time limit, 1A ILIM, no TS, DPM 4.2V
  // 0x4 0x19    -- charge current at 300mA, term sense at 50mA
  // 0x1 0x2c    -- 500mA charging, enable stat and charge term
  // 0x2 0x64    -- set 4.0V as charging target
  charger_set(0x06, 0xb0);
  charger_set(0x04, 0x19);
  charger_set(0x01, 0x2c);
  charger_set(0x02, 0x64);

  chThdCreateStatic(waChargerWatchdogThread, sizeof(waChargerWatchdogThread),
                    LOWPRIO + 1, charger_watchdog_thread, NULL);
}

// this function sets the charger into "ship mode", e.g. power fully
// disconnected from the system, allowing safe long-term storage of the
// battery while shipping or in storage. The only way out of this is to plug
// power into the microUSB port, which re-engages power to the whole system.
msg_t chargerShipMode(void) {
  charger_set(0x00, 0x08); // command for ship mode
  
  return MSG_OK;
}

// this function sets boost intent based on the enable argument
// enable = 1 turn on boost intent
// enable = 0 turn off boost intent
msg_t chargerBoostIntent(uint8_t enable) {
  if( enable ) {
    chgIntent = CHG_BOOST;
  } else {
    chgIntent = CHG_IDLE;
  }
  
  return MSG_OK;
}

// this function sets charge intent with sane defaults for the bm15 board
// enable = 1 turn on charge intent
// enable = 0 turn off charge intent
msg_t chargerChargeIntent(uint8_t enable) {
  // charger_set(0x00, 0x08); // command for ship mode
  if( enable ) {
    chgIntent = CHG_CHARGE;
  } else {
    chgIntent = CHG_IDLE;
  }
  
  return MSG_OK;
}

chargerIntent chargerCurrentIntent(void) {
  return chgIntent;
}

chargerFault chargerFaultCode(void) {
  uint8_t data;
  charger_get(CHG_REG_STATUS, &data);

  return (chargerFault) (data & 0x7);
}

chargerStat chargerGetStat(void) {
  uint8_t data;
  charger_get(CHG_REG_STATUS, &data);

  return (chargerStat) ((data >> 4) & 0x3);
}
