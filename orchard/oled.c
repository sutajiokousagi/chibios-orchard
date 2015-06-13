#include "ch.h"
#include "hal.h"
#include "oled.h"
#include "spi.h"

#include "orchard.h"
#include "gpiox.h"
#include "orchard-ui.h"

#include "gfx.h"

#include "orchard-test.h"
#include "test-audit.h"

static SPIDriver *driver;

static void oled_command_mode(void) {
#if ORCHARD_BOARD_REV == ORCHARD_REV_EVT1
  /* EVT1 swapped GPIOX and OLED_DC pins */
  palClearPad(GPIOD, 4);
#else
  palClearPad(GPIOB, 0);
#endif
}

static void oled_data_mode(void) {
#if ORCHARD_BOARD_REV == ORCHARD_REV_EVT1
  /* EVT1 swapped GPIOX and OLED_DC pins */
  palSetPad(GPIOD, 4);
#else
  palSetPad(GPIOB, 0);
#endif
}

static void oled_select(void) {
  palClearPad(GPIOA, 19);
}

static void oled_unselect(void) {
  palSetPad(GPIOA, 19);
}

void oledStop(SPIDriver *spip) {
  (void)spip;
  
  oledAcquireBus();
  oledCmd(0xAE); // display off
  oledReleaseBus();

  oledAcquireBus();
  oledCmd(0x8D); // disable charge pump
  oledCmd(0x10);
  oledReleaseBus();

  // or just yank the reset line low?
  // gpioxSetPadMode(GPIOX, oledResPad, GPIOX_OUT_PUSHPULL | GPIOX_VAL_LOW);
  
  // wait 100ms per datasheet
  chThdSleepMilliseconds(100);
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
  
  orchardGfxStart();
  width = gdispGetWidth();
  font = gdispOpenFont("UI2");
  
  gdispClear(Black);
  gdispDrawStringBox(0, 0, width, gdispGetFontMetric(font, fontHeight),
                     "Orchard EVT1", font, White, justifyCenter);
  gdispFlush();
  gdispCloseFont(font);
  orchardGfxEnd();
}

OrchardTestResult test_oled(const char *my_name, OrchardTestType test_type) {

  (void)my_name;
  
  switch(test_type) {
  case orchardTestPoweron:
  case orchardTestTrivial:
    // the OLED is not trivially testable as it's "write-only"
    return orchardResultUnsure;
  case orchardTestInteractive:
    return orchardTestPrompt("press button", "immediately", 5);
  default:
    return orchardResultNoTest;
  }
  
  return orchardResultNoTest;
}
orchard_test("oled", test_oled);
