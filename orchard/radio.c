
#include "ch.h"
#include "hal.h"
#include "spi.h"

#include "orchard.h"
#include "radio.h"

static SPIDriver *driver;

extern void spi_xmit_byte_sync(uint8_t byte);
extern uint8_t spi_recv_byte_sync(void);

static void my_spiReceive(SPIDriver *driver, int count, uint8_t *bfr) {

  int i;

  (void)driver;
  for (i = 0; i < count; i++)
    bfr[i] = spi_recv_byte_sync();
}

static void my_spiSend(SPIDriver *driver, int count, const uint8_t *bfr) {

  int i;

  (void)driver;
  for (i = 0; i < count; i++)
    spi_xmit_byte_sync(bfr[i]);
}

static const SPIConfig spi_config = {
  NULL,
  /* HW dependent part.*/
  GPIOD,
  0,
};

void radioStart(void) {

  driver = &SPID1;
  spiStart(driver, &spi_config);
}

#include "chprintf.h"

uint8_t radioRead(uint8_t addr) {

  uint8_t val;
  uint8_t dummy = 0xff;

  spiSelect(driver);
  my_spiSend(driver, 1, &addr);
  my_spiSend(driver, 1, &dummy);
  my_spiReceive(driver, 1, &val);
  spiUnselect(driver);

  return val;
}

void radioWrite(uint8_t addr, uint8_t val) {

  uint8_t buf[2] = {addr | 0x80, val};

  spiSelect(driver);
  my_spiSend(driver, 2, buf);
  spiUnselect(driver);
}
