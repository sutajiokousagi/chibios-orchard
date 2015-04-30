
#include "ch.h"
#include "hal.h"
#include "spi.h"

#include "orchard.h"
#include "radio.h"

#include "chprintf.h"

#define REG_TEMP1                 0x4e
#define REG_TEMP1_START             (1 << 3)
#define REG_TEMP1_RUNNING           (1 << 2)
#define REG_TEMP2                 0x4f

/* This number was guessed based on observations (133 at 30 degrees) */
static int temperature_offset = 133 + 30;

#define dummy_vector(n) OSAL_IRQ_HANDLER(n) { while(1); }

dummy_vector(VectorA0)
dummy_vector(VectorA4)
dummy_vector(VectorAC)
dummy_vector(VectorB0)
dummy_vector(VectorB4)

static SPIDriver *driver;

static void radio_select(void) {

  spiAcquireBus(driver);
  spiSelect(driver);
}

static void radio_unselect(void) {

  spiUnselect(driver);
  spiReleaseBus(driver);
}

void radioStart(SPIDriver *spip) {

  driver = spip;
}

uint8_t radioRead(uint8_t addr) {

  uint8_t val;

  radio_select();
  spiSend(driver, 1, &addr);
  spiReceive(driver, 1, &val);
  radio_unselect();

  return val;
}

void radioWrite(uint8_t addr, uint8_t val) {

  uint8_t buf[2] = {addr | 0x80, val};

  radio_select();
  spiSend(driver, 2, buf);
  radio_unselect();
}

int radioDump(uint8_t addr, void *bfr, int count) {

  radio_select();
  spiSend(driver, 1, &addr);
  spiReceive(driver, count, bfr);
  radio_unselect();

  return 0;
}

int radioTemperature(void) {

  uint8_t buf[2];

  buf[0] = REG_TEMP1 | 0x80;
  buf[1] = REG_TEMP1_START;

  radio_select();
  spiSend(driver, 2, buf);

  do {
    buf[0] = REG_TEMP1 | 0x80;

    spiUnselect(driver);
    spiSelect(driver);
    spiSend(driver, 1, buf);
    spiReceive(driver, 2, buf);
  } while (buf[0] & REG_TEMP1_RUNNING);

  radio_unselect();

  return (temperature_offset - buf[1]);
}
