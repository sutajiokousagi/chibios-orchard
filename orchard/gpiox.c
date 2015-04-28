
#include "ch.h"
#include "hal.h"
#include "i2c.h"

#include "gpiox.h"
#include "orchard.h"

static I2CDriver *driver;

static uint8_t regcache[10];

#define REG_ID            0x01
#define REG_DIR           0x03
#define REG_OUT           0x05
#define REG_HIZ           0x07
#define REG_IRQ_LEVEL     0x09
#define REG_PULL_ENABLE   0x0b
#define REG_PULL_DIR      0x0d
#define REG_IN            0x0f
#define REG_IRQ_MASK      0x11
#define REG_IRQ_STATUS    0x13

static void gpiox_set(uint8_t reg, uint8_t val) {

  uint8_t tx[2] = {reg, val};

  i2cMasterTransmitTimeout(driver, gpioxAddr,
                           tx, sizeof(tx),
                           NULL, 0,
                           TIME_INFINITE);
}

static uint8_t gpiox_get(uint8_t reg) {

  const uint8_t tx[1] = {reg};
  uint8_t rx[1];

  i2cMasterTransmitTimeout(driver, gpioxAddr,
                           tx, sizeof(tx),
                           rx, sizeof(rx),
                           TIME_INFINITE);
  return rx[0];
}

void gpioxStart(I2CDriver *i2cp) {

  unsigned int i;

  driver = i2cp;

  for (i = 0; i < ARRAY_SIZE(regcache); i++)
    regcache[i] = 0;

  i2cAcquireBus(driver);
  gpiox_set(REG_ID, 1);
  i2cReleaseBus(driver);
}

void gpioxSetPad(void *port, int pad) {

  (void)port;
  
  regcache[REG_OUT / 2] |= (1 << pad);
  i2cAcquireBus(driver);
  gpiox_set(REG_OUT, regcache[REG_OUT / 2]);
  i2cReleaseBus(driver);
}

void gpioxClearPad(void *port, int pad) {

  (void)port;
  
  regcache[REG_OUT / 2] &= ~(1 << pad);
  i2cAcquireBus(driver);
  gpiox_set(REG_OUT, regcache[REG_OUT / 2]);
  i2cReleaseBus(driver);
}

void gpioxTogglePad(void *port, int pad) {

  (void)port;
  
  regcache[REG_OUT / 2] ^= (1 << pad);
  i2cAcquireBus(driver);
  gpiox_set(REG_OUT, regcache[REG_OUT / 2]);
  i2cReleaseBus(driver);
}

void gpioxSetPadMode(void *port, int pad, int mode) {

  int bit = (1 << pad);

  (void)port;

  /* Start out by masking the IRQ, to prevent spurrious interrupts */
  regcache[REG_IRQ_MASK / 2] |= bit;
  i2cAcquireBus(driver);
  gpiox_set(REG_IRQ_MASK, regcache[REG_IRQ_MASK / 2]);

  switch (mode & GPIOX_IRQMASK) {

    case GPIOX_IRQ_HIGH:
      regcache[REG_IRQ_MASK  / 2] &= ~bit;
      regcache[REG_IRQ_LEVEL / 2] |=  bit;
      break;

    case GPIOX_IRQ_LOW:
      regcache[REG_IRQ_MASK  / 2] &= ~bit;
      regcache[REG_IRQ_LEVEL / 2] &= ~bit;
      break;

    case GPIOX_IRQ_NONE:
    default:
      regcache[REG_IRQ_MASK / 2] |= bit;
      break;
  }

  switch (mode & GPIOX_DIRMASK) {

    case GPIOX_OUT_OPENDRAIN:
      regcache[REG_DIR / 2] |= bit;
      regcache[REG_HIZ / 2] |= bit;
      break;

    case GPIOX_OUT_PUSHPULL:
      regcache[REG_DIR / 2] |= bit;
      regcache[REG_HIZ / 2] &= ~bit;
      break;

    case GPIOX_IN:
    default:
      regcache[REG_DIR / 2] &= ~bit;
      regcache[REG_HIZ / 2] &= ~bit;
      break;
  }

  if (mode & GPIOX_VAL_HIGH)
    regcache[REG_OUT / 2] |= bit;
  else
    regcache[REG_OUT / 2] &= ~bit;

  switch (mode & GPIOX_PULLMASK) {
    case GPIOX_PULL_UP:
      regcache[REG_PULL_ENABLE / 2] |= bit;
      regcache[REG_PULL_DIR    / 2] |= bit;
      break;

    case GPIOX_PULL_DOWN:
      regcache[REG_PULL_ENABLE / 2] |= bit;
      regcache[REG_PULL_DIR    / 2] &= ~bit;
      break;

    case GPIOX_PULL_NONE:
    default:
      regcache[REG_PULL_ENABLE / 2] &= ~bit;
      regcache[REG_PULL_DIR    / 2] &= ~bit;
      break;
  }

  gpiox_set(REG_OUT, regcache[REG_OUT / 2]);
  gpiox_set(REG_HIZ, regcache[REG_HIZ / 2]);
  gpiox_set(REG_PULL_DIR, regcache[REG_PULL_DIR / 2]);
  gpiox_set(REG_PULL_ENABLE, regcache[REG_PULL_ENABLE / 2]);
  gpiox_set(REG_DIR, regcache[REG_DIR / 2]);
  gpiox_set(REG_IRQ_LEVEL, regcache[REG_IRQ_LEVEL / 2]);
  gpiox_set(REG_IRQ_MASK, regcache[REG_IRQ_MASK / 2]);

  i2cReleaseBus(driver);
}

uint8_t gpioxReadPad(void *port, int pad) {

  (void)port;
  uint8_t val;
  
  regcache[REG_OUT / 2] |= (1 << pad);
  i2cAcquireBus(driver);
  val = gpiox_get(REG_IN);
  i2cReleaseBus(driver);

  return !!val;
}
