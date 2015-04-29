#ifndef __ORCHARD_CHARGER_H__
#define __ORCHARD_CHARGER_H__

struct charger_data {
  uint32_t placeholder;   // placeholder for when we need it
};

void chargerStart(I2CDriver *driver);
msg_t chargerShipMode(void);

#endif /* __ORCHARD_CHARGER_H__ */
