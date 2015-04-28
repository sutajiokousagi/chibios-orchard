
#include "ch.h"
#include "hal.h"
#include "spi.h"

#include "orchard.h"
#include "radio.h"

static SPIDriver *driver;

static const SPIConfig spi_config = {
  NULL,
  /* HW dependent part.*/
  GPIOD,
  0,
};

void radioStart(SPIDriver *spip) {

  driver = spip;
  spiStart(driver, &spi_config);
}

#include "chprintf.h"

uint8_t radioRead(uint8_t addr) {

  uint8_t val[10];
  unsigned int i;

  spiSelect(driver);
  spiSend(driver, 1, &addr);
  spiReceive(driver, sizeof(val), val);
  chprintf(stream, "Debug %d bytes:\r\n", sizeof(val));
  for (i = 0; i < sizeof(val); i++)
    chprintf(stream, "    %02x: %02x\r\n", addr + i, val[i]);

  spiUnselect(driver);

  return val[0];
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
