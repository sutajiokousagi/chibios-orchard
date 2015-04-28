#ifndef __GPIOX_H__
#define __GPIOX_H__

#define GPIOX_DIRMASK         0x03
#define GPIOX_IN            0x00
#define GPIOX_OUT_PUSHPULL  0x02
#define GPIOX_OUT_OPENDRAIN 0x03

#define GPIOX_VALMASK         0x04
#define GPIOX_VAL_HIGH      0x04
#define GPIOX_VAL_LOW       0x00

#define GPIOX_PULLMASK        0x30
#define GPIOX_PULL_NONE     0x00
#define GPIOX_PULL_UP       0x20
#define GPIOX_PULL_DOWN     0x30

#define GPIOX_IRQMASK         0xc0
#define GPIOX_IRQ_NONE      0x00
#define GPIOX_IRQ_LOW       0x80
#define GPIOX_IRQ_HIGH      0xc0

#define GPIOX               NULL

void gpioxStart(I2CDriver *i2cp);
void gpioxSetPad(void *port, int pad);
void gpioxClearPad(void *port, int pad);
void gpioxTogglePad(void *port, int pad);
void gpioxSetPadMode(void *port, int pad, int mode);
uint8_t gpioxReadPad(void *port, int pad);

#endif /* __GPIOX_H__ */
