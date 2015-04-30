#ifndef __ORCHARD_RADIO_H__
#define __ORCHARD_RADIO_H__

void radioStart(SPIDriver *spip);
uint8_t radioRead(uint8_t addr);
void radioWrite(uint8_t addr, uint8_t val);
int radioDump(uint8_t addr, void *bfr, int count);
int radioTemperature(void);

#endif /* __ORCHARD_RADIO_H__ */
