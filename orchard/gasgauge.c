
#include "ch.h"
#include "hal.h"
#include "i2c.h"

#include "orchard.h"
#include "gasgauge.h"

static I2CDriver *driver;

static void gg_set(uint8_t cmdcode, int16_t val) {

  uint8_t tx[3] = {cmdcode, (uint8_t) val & 0xFF, (uint8_t) (val >> 8) & 0xFF};

  i2cMasterTransmitTimeout(driver, ggAddr,
                           tx, sizeof(tx),
                           NULL, 0,
                           TIME_INFINITE);
}

static void gg_get(uint8_t cmdcode, int16_t *data) {
  uint8_t tx[1];
  uint8_t rx[2];

  tx[0] = cmdcode;

  i2cMasterTransmitTimeout(driver, ggAddr,
                           tx, sizeof(tx),
                           rx, sizeof(rx),
                           TIME_INFINITE);

  *data = rx[0] | (rx[1] << 8);
}

void ggStart(I2CDriver *i2cp) {

  driver = i2cp;

}

int16_t ggAvgCurrent(void) {
  int16_t data;
  
  i2cAcquireBus(driver);
  gg_get( GG_CMD_AVGCUR, &data );
  i2cReleaseBus(driver);

  return data;
}

int16_t ggAvgPower(void) {
  int16_t data;
  
  i2cAcquireBus(driver);
  gg_get( GG_CMD_AVGPWR, &data );
  i2cReleaseBus(driver);

  return data;
}

int16_t ggRemainingCapacity(void) {
  int16_t data;
  
  i2cAcquireBus(driver);
  gg_get( GG_CMD_RM, &data );
  i2cReleaseBus(driver);

  return data;
}

int16_t ggStateofCharge(void) {
  int16_t data;
  
  i2cAcquireBus(driver);
  gg_get( GG_CMD_SOC, &data );
  i2cReleaseBus(driver);

  return data;
}

int16_t ggVoltage(void) {
  int16_t data;
  
  i2cAcquireBus(driver);
  gg_get( GG_CMD_VOLT, &data );
  i2cReleaseBus(driver);

  return data;
}


