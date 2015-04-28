
#include "ch.h"
#include "hal.h"
#include "spi.h"

#include "orchard.h"
#include "radio.h"

static SPIDriver *driver;

void radioStart(SPIDriver *spip) {

  driver = spip;
}

#include "chprintf.h"

uint8_t radioRead(uint8_t addr) {

  uint8_t val;

  spiSelect(driver);
  spiSend(driver, 1, &addr);
  spiReceive(driver, 1, &val);
  spiUnselect(driver);

  return val;
}

void radioWrite(uint8_t addr, uint8_t val) {

  uint8_t buf[2] = {addr | 0x80, val};

  spiSelect(driver);
  spiSend(driver, 2, buf);
  spiUnselect(driver);
}

int radioDump(uint8_t addr, void *bfr, int count) {

  spiSelect(driver);
  spiSend(driver, 1, &addr);
  spiReceive(driver, count, bfr);
  spiUnselect(driver);

  return 0;
}
