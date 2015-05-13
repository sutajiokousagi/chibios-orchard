
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
  charger_set(CHG_REG_SAFETY, 0xb0);
  charger_set(CHG_REG_CURRENT, 0x19);
  charger_set(CHG_REG_CTL, 0x2c);
  charger_set(CHG_REG_BATTV, 0x64);

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

void chargerForce500(void) {
  uint8_t data;

  charger_get(CHG_REG_CTL, &data);
  data &= 0x8F;
  data |= 0x20;
  charger_set(CHG_REG_CTL, data);
}

void chargerForce900(void) {
  uint8_t data;

  charger_get(CHG_REG_CTL, &data);
  data &= 0x8F;
  data |= 0x30;
  charger_set(CHG_REG_CTL, data);
}

void chargerForce1500(void) {
  uint8_t data;

  charger_get(CHG_REG_CTL, &data);
  data &= 0x8F;
  data |= 0x40;
  charger_set(CHG_REG_CTL, data);
}

// returns host port current capability in mA
uint16_t chargerGetHostCurrent(void) {
  uint8_t data;
  charger_get(CHG_REG_CTL, &data);

  data = (data >> 4) & 0x7;
  switch(data) {
  case 0:
    return 100;
    break;
  case 1:
    return 150;
    break;
  case 2:
    return 500;
    break;
  case 3:
    return 900;
    break;
  case 4:
    return 1500;
    break;
  case 5:
    return 1950;
    break;
  case 6:
    return 2500;
    break;
  case 7:
    return 2000;
    break;
  default:
    return 0;
  }
}

// returns current battery target voltage in mV
uint16_t chargerGetTargetVoltage(void) {
  uint8_t data;
  charger_get(CHG_REG_BATTV, &data);

  return ((data >> 2) & 0x3F) * 20 + 3500;
}


// returns current battery target current in mA
uint16_t chargerGetTargetCurrent(void) {
  uint8_t data;
  charger_get(CHG_REG_CURRENT, &data);

  return ((data >> 3) & 0x1F) * 100 + 500;
}

// returns battery termination current threshold in mA
uint16_t chargerGetTerminationCurrent(void) {
  uint8_t data;
  uint16_t retval;
  charger_get(CHG_REG_CTL, &data);
  if( !(data & 0x4) ) {
    // charge termination is disabled
    return 0;
  }
  
  charger_get(CHG_REG_CURRENT, &data);

  retval = (data & 0x7) * 50 + 50;
  retval = retval > 300 ? 300 : retval;

  return retval;
}

// returns boost current limit
uint16_t chargerGetBoostLimit(void) {
  uint8_t data;
  charger_get(CHG_REG_SAFETY, &data);

  return (data & 0x10) ? 1000: 500;
}

void chargerSetTargetVoltage(uint16_t voltage) {
  uint16_t code;
  uint8_t data;
  
  if( voltage > 4160 )
    voltage = 4160;

  code = (voltage - 3500) / 20;
  if( code > 0x21 ) // this safety check caps voltage to 4.16V
    code = 0x21;
  
  charger_get(CHG_REG_BATTV, &data);
  data &= 0x3;
  data |= code << 2;
  charger_set(CHG_REG_BATTV, data);
}

void chargerSetTargetCurrent(uint16_t current) {
  uint16_t code;
  uint8_t data;
  
  if( current > 3000 )
    current = 3000;

  code = (current - 500) / 100;
  if( code > 0x14 ) // this safety check caps current to 2000mA
    code = 0x14;
  
  charger_get(CHG_REG_CURRENT, &data);
  data &= 0x7;
  data |= code << 3;
  charger_set(CHG_REG_CURRENT, data);
}
