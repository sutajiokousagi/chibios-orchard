#ifndef __ORCHARD_RADIO_H__
#define __ORCHARD_RADIO_H__

void radioStart(void);
uint8_t radioRead(uint8_t addr);
void radioWrite(uint8_t addr, uint8_t val);

#endif /* __ORCHARD_RADIO_H__ */
