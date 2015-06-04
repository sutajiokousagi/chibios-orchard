#ifndef __OLED_H__
#define __OLED_H__

#include "spi.h"

void oledStart(SPIDriver *device);
void oledStop(SPIDriver *device);
void oledAcquireBus(void);
void oledReleaseBus(void);
void oledCmd(uint8_t cmd);
void oledData(uint8_t *data, uint16_t length);
void oledOrchardBanner(void);

#endif /* __OLED_H__ */
