
#include "ch.h"
#include "hal.h"
#include "spi.h"

#include "orchard.h"
#include "gpiox.h"

#define oledResPad 6

static const uint8_t init_sequence[] = {
  0xae, 0xff,       /* Set display off */
  0xd5, 0x80, 0xff, /* Set display clock divide ratio / oscillator frequency */
  0xa8, 0x3f, 0xff, /* Set multiplex ratio */
  0xd3, 0x00, 0xff, /* Set display offset */
  0x40, 0xff,       /* Set display start line */
  0x8d, 0x14, 0xff, /* Set charge pump to internal DC/DC */
  0xa1, 0xff,       /* Set segment re-map */
  0xc8, 0xff,       /* Set COM output scan direction */
  0xda, 0x12, 0xff, /* Set COM, plus hardware configuration */
  0x81, 0xcf, 0xff, /* Set contrast control */
  0xd9, 0xf1, 0xff, /* Set pre-charge period */
  0xdb, 0x40, 0xff, /* Set VCOMH deselect level */
  0xa4, 0xff,       /* Set entire display off */
  0xa6, 0xff,       /* Set normal (not inverted) display */
  0xaf, 0xff,       /* Turn display on */
};

static SPIDriver *driver;

static void oled_command_mode(void) {
  palClearPad(GPIOE, 19);
}

static void oled_data_mode(void) {
  palSetPad(GPIOE, 19);
}

static void oled_select(void) {
  palClearPad(GPIOA, 19);
}

static void oled_unselect(void) {
  palSetPad(GPIOA, 19);
}

void oledStart(SPIDriver *spip) {

  unsigned int i;

  driver = spip;

  gpioxSetPadMode(GPIOX, oledResPad, GPIOX_OUT_PUSHPULL | GPIOX_VAL_HIGH);
  gpioxSetPadMode(GPIOX, 4, GPIOX_OUT_PUSHPULL | GPIOX_VAL_HIGH);

  spiAcquireBus(spip);

  for (i = 0; i < ARRAY_SIZE(init_sequence); i++) {
    if (init_sequence[i] == 0xff) {
      oled_unselect();
      oled_data_mode();
      continue;
    }
    
    oled_select();
    oled_command_mode();
    spiSend(driver, 1, &init_sequence[i]);
  }

  oled_select();
  {
    uint16_t byte = 0x1234;
//    while (1) {
      byte = (byte >> 1) ^ ((byte & 1) << 5) ^ 0x55aa;
      for (i = 0; i < 64*32; i++)
        spiSend(driver, 1, &byte);
//    }
  }
  oled_unselect();

  spiReleaseBus(spip);
}
