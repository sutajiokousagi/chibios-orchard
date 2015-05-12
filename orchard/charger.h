#ifndef __ORCHARD_CHARGER_H__
#define __ORCHARD_CHARGER_H__

struct charger_data {
  uint32_t placeholder;   // placeholder for when we need it
};

typedef enum chargerStates {
  CHG_CHARGE,
  CHG_BOOST,
  CHG_IDLE
} chargerStates;

void chargerStart(I2CDriver *driver);
msg_t chargerShipMode(void);
msg_t chargerBoostMode(uint8_t enable);
chargerStates chargerCurrentState(void);

#endif /* __ORCHARD_CHARGER_H__ */
