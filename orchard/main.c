/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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

#include "ch.h"
#include "hal.h"
#include "i2c.h"
#include "spi.h"

#include "shell.h"
#include "chprintf.h"

#include "orchard.h"
#include "orchard-shell.h"
#include "orchard-events.h"
#include "orchard-app.h"

#include "accel.h"
#include "captouch.h"
#include "charger.h"
#include "gpiox.h"
#include "led.h"
#include "oled.h"
#include "radio.h"

#include "gfx.h"

struct evt_table orchard_events;

#define LED_COUNT 16
#define UI_LED_COUNT 16
static uint8_t fb[LED_COUNT * 3];
static uint8_t ui_fb[LED_COUNT * 3];

static const I2CConfig i2c_config = {
  100000
};

static const SPIConfig spi_config = {
  NULL,
  /* HW dependent part.*/
  GPIOD,
  0,
};

static void shell_termination_handler(eventid_t id) {
  static int i = 1;
  (void)id;

  chprintf(stream, "\r\nRespawning shell (shell #%d, event %d)\r\n", ++i, id);
  orchardShellRestart();
}

static void orchard_app_restart(eventid_t id) {
  static int i = 1;
  (void)id;

  chprintf(stream, "\r\nRunning next app (pid #%d)\r\n", ++i);
  orchardAppRestart();
}

static void key_mod(eventid_t id) {

  (void)id;
  int i;
  int val = captouchRead();
  Color uiColor;

  uiColor.r = 0;
  uiColor.g = 0;
  uiColor.b = 0;
  for (i = 0; i < UI_LED_COUNT; i++)
    uiLedSet(i, uiColor);

  uiColor.r = 200;
  uiColor.g = 200;
  uiColor.b = 0;
  
  if ((val & (1 << 7)))
    uiLedSet(0, uiColor);

  if ((val & (1 << 8)) && (val & (1 << 7)))
    uiLedSet(1, uiColor);

  if ((val & (1 << 8)))
    uiLedSet(2, uiColor);

  if ((val & (1 << 8)) && (val & (1 << 9)))
    uiLedSet(3, uiColor);

  if ((val & (1 << 9)))
    uiLedSet(4, uiColor);

  if ((val & (1 << 10)) && (val & (1 << 9)))
    uiLedSet(5, uiColor);

  if ((val & (1 << 10)))
    uiLedSet(6, uiColor);

  if ((val & (1 << 1)))
    uiLedSet(7, uiColor);

  if ((val & (1 << 1)) && (val & (1 << 2)))
    uiLedSet(8, uiColor);

  if ((val & (1 << 2)))
    uiLedSet(9, uiColor);

  if ((val & (1 << 3)) && (val & (1 << 2)))
    uiLedSet(10, uiColor);

  if ((val & (1 << 3)))
    uiLedSet(11, uiColor);

  if ((val & (1 << 4)) && (val & (1 << 3)))
    uiLedSet(12, uiColor);

  if ((val & (1 << 4)))
    uiLedSet(13, uiColor);

  if ((val & (1 << 4)) && (val & (1 << 6)))
    uiLedSet(14, uiColor);

  if ((val & (1 << 7)) && (val & (1 << 6)))
    uiLedSet(15, uiColor);

}

static void ble_ready(eventid_t id) {

  (void)id;
  chprintf(stream, " [BLE ready] ");
}

static void freefall(eventid_t id) {

  (void)id;
  chprintf(stream, "Unce. ");
}

extern int print_hex(BaseSequentialStream *chp,
                     const void *block, int count, uint32_t start);

static void default_radio_handler(uint8_t type, uint8_t src, uint8_t dst,
                                  uint8_t length, void *data) {

  chprintf(stream, "\r\nNo handler for packet found.  %02x -> %02x : %02x\r\n",
           src, dst, type);
  print_hex(stream, data, length, 0);
}

static void print_mcu_info(void) {
  uint32_t sdid = SIM->SDID;
  const char *famid[] = {
    "KL0%d (low-end)",
    "KL1%d (basic)",
    "KL2%d (USB)",
    "KL3%d (Segment LCD)",
    "KL4%d (USB and Segment LCD)",
  };
  const uint8_t ram[] = {
    0,
    1,
    2,
    4,
    8,
    16,
    32,
    64,
  };

  const uint8_t pins[] = {
    16,
    24,
    32,
    36,
    48,
    64,
    80,
  };

  if (((sdid >> 20) & 15) != 1) {
    chprintf(stream, "Device is not Kinetis KL-series\r\n");
    return;
  }

  chprintf(stream, famid[(sdid >> 28) & 15], (sdid >> 24) & 15);
  chprintf(stream, " with %d kB of ram detected"
                   " (Rev: %04x  Die: %04x  Pins: %d).\r\n",
                   ram[(sdid >> 16) & 15],
                   (sdid >> 12) & 15,
                   (sdid >> 7) & 31,
                   pins[(sdid >> 0) & 15]);
}


/*
 * Application entry point.
 */
int main(void)
{

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  evtTableInit(orchard_events, 32);

  orchardShellInit();

  chprintf(stream, "\r\n\r\nOrchard shell.  Based on build %s\r\n", gitversion);
  print_mcu_info();

  i2cStart(i2cDriver, &i2c_config);
  spiStart(&SPID1, &spi_config);
  spiStart(&SPID2, &spi_config);

  orchardEventsStart();

  gpioxStart(i2cDriver);

  accelStart(i2cDriver);
  chargerStart(i2cDriver);
  captouchStart(i2cDriver);
  radioStart(&KRADIO1, &SPID1);
  oledStart(&SPID2);
  ledStart(LED_COUNT, fb, UI_LED_COUNT, ui_fb);
  effectsStart();
  orchardAppInit();

  evtTableHook(orchard_events, shell_terminated, shell_termination_handler);
  evtTableHook(orchard_events, orchard_app_terminated, orchard_app_restart);
  evtTableHook(orchard_events, captouch_changed, key_mod);
  evtTableHook(orchard_events, ble_rdy, ble_ready);
  evtTableHook(orchard_events, accel_freefall, freefall);
  radioSetDefaultHandler(radioDriver, default_radio_handler);

  gfxInit();
  
  orchardShellRestart();
  orchardAppRestart();

  while (TRUE)
    chEvtDispatch(evtHandlers(orchard_events), chEvtWaitOne(ALL_EVENTS));
}
