/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <stdlib.h>
#include <ctype.h>

#include "orchard-shell.h"

static const char *gpio_name(int bank, int pad) {
  switch (bank) {

  case 'A':
  case 0:
    switch (pad) {
      case 0: return "SWD_CLK";
      case 1: return "TOUCH2";
      case 2: return "TOUCH3";
      case 3: return "SWD_DIO";
      case 4: return "TOUCH5";
      case 18: return "RF_CLK";
      case 19: return "OLED_CS";
      case 20: return "MCU_RESET";
      default: return NULL;
    }

  case 'B':
  case 1:
    switch (pad) {
      case 0: return "OLED_DC";
      case 1: return "TOUCH6";
      case 2: return "TOUCH7";
      case 17: return "TOUCH8";
      default: return NULL;
    }

  case 'C':
  case 2:
    switch (pad) {
      case 1: return "SCL";
      case 2: return "SDA";
      case 3: return "BLE_RDYN";
      case 4: return "RF_PKT_RDY";
      case 5: return "SPI0_SCK";
      case 6: return "SPI0_MOSI";
      case 7: return "SPI0_MISO";
      default: return NULL;
    } 

  case 'D':
  case 3:
    switch (pad) {
      case 0: return "RADIO_CS";
      case 4: return "GPIO_INT";
      case 5: return "SPI1_SCK";
      case 6: return "DUART_RX";
      case 7: return "DUART_TX";
      default: return NULL;
    } 

  case 'E':
  case 4:
    switch (pad) {
      case 0: return "SPI1_MISO";
      case 1: return "SPI1_MOSI";
      case 16: return "MIC_OUT";
      case 17: return "LED0";
      case 18: return "BLE_REQN";
      case 19: return "DAC0_OUT";
      case 30: return "RF_RESET";
      default: return NULL;
    } 

  default:
    return NULL;
  }
}

static int gpio_set(int bank, int pad, int val)
{

  switch(bank) {
  case 'A':
  case 0:
    palWritePad(GPIOA, pad, val);
    break;
  case 'B':
  case 1:
    palWritePad(GPIOB, pad, val);
    break;
  case 'C':
  case 2:
    palWritePad(GPIOC, pad, val);
    break;
  case 'D':
  case 3:
    palWritePad(GPIOD, pad, val);
    break;
  case 'E':
  case 4:
    palWritePad(GPIOE, pad, val);
    break;
  default:
    return -1;
  }
  return 0;
}

static int gpio_get(int bank, int pad)
{
  switch(bank) {
  case 'A':
  case 0:
    return palReadPad(GPIOA, pad);
    break;
  case 'B':
  case 1:
    return palReadPad(GPIOB, pad);
    break;
  case 'C':
  case 2:
    return palReadPad(GPIOC, pad);
    break;
  case 'D':
  case 3:
    return palReadPad(GPIOD, pad);
    break;
  case 'E':
  case 4:
    return palReadPad(GPIOE, pad);
    break;
  default:
    return -1;
  }
}

static void print_help(BaseSequentialStream *chp) {
  chprintf(chp, "Usage: gpio PAD [val]\r\n");
  chprintf(chp, "       Where PAD is a pad such as PA0 or PB15, and\r\n");
  chprintf(chp, "       the optional [val] is a value to write,\r\n");
  chprintf(chp, "       either 0 or 1. If omitted, the value is read.\r\n");
  return;
}

static void cmd_gpio(BaseSequentialStream *chp, int argc, char *argv[]) {
  int bank, pad;
  (void)argc;
  (void)argv;

  if (argc > 0) {
    int val;
    int pad;
    char bank;

    if (!((argv[0][0] == 'P') || (argv[0][0] == 'p'))) {
      print_help(chp);
      return;
    }

    bank = toupper(argv[0][1]);
    pad = strtoul(&argv[0][2], NULL, 10);
    if ((bank < 'A') || (bank > 'E')) {
      print_help(chp);
      return;
    }

    if (argc > 1) {
      const char *name = gpio_name(bank, pad);
      if (!name) {
        chprintf(chp, "Pad P%c%d unconnected\r\n", bank, pad);
        return;
      }

      val = strtoul(argv[1], NULL, 0);
      chprintf(chp, "Setting P%c%d (%s) to %d\r\n", bank, pad, name, val);
      gpio_set(bank, pad, !!val);
    }
    else {
      const char *name = gpio_name(bank, pad);
      if (!name) {
        chprintf(chp, "Pad P%c%d unconnected\r\n", bank, pad);
        return;
      }

      val = gpio_get(bank, pad);
      chprintf(chp, "Value at P%c%d (%s): %d\r\n", bank, pad, name, val);
    }
  }
  else {
    for (bank = 'A'; bank <= 'E'; bank++) {
      chprintf(chp, "GPIO %c:\r\n", bank);
      for (pad = 0; pad < 16; pad++) {
        const char *name = gpio_name(bank, pad);
        if (!name)
          continue;
        chprintf(chp, "    P%c%d: %d  %s\r\n", bank, pad, gpio_get(bank, pad),
          name);
      }
    }
  }
}

orchard_command("gpio", cmd_gpio);
