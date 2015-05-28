
int16_t ggVoltage(void);
int16_t ggAvgCurrent(void);
int16_t ggAvgPower(void);
int16_t ggRemainingCapacity(void);
int16_t ggStateofCharge(void);
void ggStart(I2CDriver *i2cp);

#define GG_CMD_CNTL  0x00
#define GG_CMD_TEMP  0x02     // 0.1 degrees K, of battery
#define GG_CMD_VOLT  0x04     // (mV)
#define GG_CMD_FLAG  0x06
#define GG_CMD_NOM_CAP  0x08  // nominal available capacity (mAh)
#define GG_CMD_FULL_CAP 0x0A  // full available capacity (mAh)
#define GG_CMD_RM       0x0C  // remaining capacity (mAh)
#define GG_CMD_FCC      0x0E  // full charge capacity (mAh)
#define GG_CMD_AVGCUR   0x10  // (mA)
#define GG_CMD_SBYCUR   0x12  // standby current (mA)
#define GG_CMD_MAXCUR   0x14  // max load current (mA)
#define GG_CMD_AVGPWR   0x18  // average power (mW)
#define GG_CMD_SOC      0x1C  // state of charge in %
#define GG_CMD_INTTEMP  0x1E  // temperature of gg IC
#define GG_CMD_SOH      0x20  // state of health, num / %

