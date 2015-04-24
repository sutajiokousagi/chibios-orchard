
#include "ch.h"
#include "hal.h"
#include "i2c.h"

#include "accel.h"
#include "orchard.h"

static I2CDriver *driver;

static void accel_set(uint8_t reg, uint8_t val) {

  uint8_t tx[2] = {reg, val};

  i2cMasterTransmitTimeout(driver, accelAddr,
                           tx, sizeof(tx),
                           NULL, 0,
                           TIME_INFINITE);
}

void accelStart(I2CDriver *i2cp) {

  driver = i2cp;

  i2cAcquireBus(driver);
  accel_set(0x2A, 0x01);
  i2cReleaseBus(driver);
}

msg_t accelPoll(struct accel_data *data) {
  uint8_t tx[1];
  uint8_t rx[7];

  tx[0] = 0;

  i2cAcquireBus(driver);
  i2cMasterTransmitTimeout(driver, accelAddr,
                           tx, sizeof(tx),
                           rx, sizeof(rx),
                           TIME_INFINITE);
  i2cReleaseBus(driver);

  data->x  = ((rx[1] & 0xff)) << 4;
  data->x |= ((rx[2] >> 4) & 0x0f);
  data->y  = ((rx[3] & 0xff)) << 4;
  data->y |= ((rx[4] >> 4) & 0x0f);
  data->z  = ((rx[5] & 0xff)) << 4;
  data->z |= ((rx[6] >> 4) & 0x0f);

  return MSG_OK;
}
