
#include "ch.h"
#include "hal.h"
#include "spi.h"

#include "orchard.h"
#include "gpiox.h"

#include "gfx.h"

static SPIDriver *driver;

static void oled_command_mode(void) {
#ifdef REV_EVT1
  palClearPad(GPIOD, 4);
#elif REV_EVT1B
  palClearPad(GPIOB, 0);
#else
#error "Please specify a board rev in the Makfile"
#endif
}

static void oled_data_mode(void) {
#ifdef REV_EVT1
  palSetPad(GPIOD, 4);
#elif REV_EVT1B
  palSetPad(GPIOB, 0);
#else
#error "Please specify a board rev in the Makefile"
#endif
}

static void oled_select(void) {
  palClearPad(GPIOA, 19);
}

static void oled_unselect(void) {
  palSetPad(GPIOA, 19);
}

void oledStart(SPIDriver *spip) {

  driver = spip;

  gpioxSetPadMode(GPIOX, oledResPad, GPIOX_OUT_PUSHPULL | GPIOX_VAL_HIGH);
  gpioxSetPadMode(GPIOX, 4, GPIOX_OUT_PUSHPULL | GPIOX_VAL_HIGH);
}

/* Exported functions, called by uGFX.*/
void oledCmd(uint8_t cmd) {

  oled_command_mode();
  spiSend(driver, 1, &cmd);
}

void oledData(uint8_t *data, uint16_t length) {
  unsigned int i;

  oled_data_mode();
  for( i = 0; i < length; i++ )
    spiSend(driver, 1, &data[i]);
}

void oledAcquireBus(void) {
  spiAcquireBus(driver);
  oled_select();
}

void oledReleaseBus(void) {
  oled_unselect();
  spiReleaseBus(driver);
}

void oledOrchardBanner(void) {
  coord_t width;
  font_t font;
  
  width = gdispGetWidth();
  font = gdispOpenFont("UI2");
  
  gdispClear(Black);
  gdispDrawStringBox(0, 0, width, gdispGetFontMetric(font, fontHeight),
                     "Orchard EVT1", font, White, justifyCenter);
  gdispFlush();
}
