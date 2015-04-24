#ifndef __ORCHARD_ACCEL_H__
#define __ORCHARD_ACCEL_H__

struct accel_data {
  int x;
  int y;
  int z;
};

void accelStart(I2CDriver *driver);
msg_t accelPoll(struct accel_data *data);

#endif /* __ORCHARD_ACCEL_H__ */
