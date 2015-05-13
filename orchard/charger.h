#ifndef __ORCHARD_CHARGER_H__
#define __ORCHARD_CHARGER_H__

struct charger_data {
  uint32_t placeholder;   // placeholder for when we need it
};

typedef enum chargerIntent {
  CHG_IDLE,
  CHG_CHARGE,
  CHG_BOOST,
} chargerIntent;

typedef enum chargerFault {
  ENORMAL = 0,
  EBOOSTOVP,
  ELOWV,
  ETHERMAL,
  ETIMER,
  EBATTOVP,
  ENOBATT
} chargerFault;

typedef enum chargerStat {
  STAT_RDY = 0,
  STAT_CHARGING,
  STAT_DONE,
  STAT_FAULT
} chargerStat;

void chargerStart(I2CDriver *driver);
msg_t chargerShipMode(void);
msg_t chargerBoostIntent(uint8_t enable);
msg_t chargerChargeIntent(uint8_t enable);
chargerIntent chargerCurrentIntent(void);
chargerFault chargerFaultCode(void);
chargerStat chargerGetStat(void);

#define CHG_REG_STATUS  0
#define CHG_REG_CTL     1
#define CHG_REG_BATTV   2
#define CHG_REG_ID      3
#define CHG_REG_CURRENT 4
#define CHG_REG_DPM     5
#define CHG_REG_SAFETY  6

#endif /* __ORCHARD_CHARGER_H__ */
