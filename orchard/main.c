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

#include "accel.h"
#include "captouch.h"
#include "charger.h"
#include "gpiox.h"
#include "led.h"
#include "oled.h"
#include "radio.h"

struct evt_table orchard_events;

#define LED_COUNT 16
static uint8_t fb[LED_COUNT * 3];

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

static void key_mod(eventid_t id) {

  (void)id;
  int i;
  int val = captouchRead();

  for (i = 0; i < sizeof(fb); i++)
    fb[i] = 0;
  if ((val & (1 << 7)))
    ledSetRGB(fb, 0, 32, 32, 0, 0);

  if ((val & (1 << 8)) && (val & (1 << 7)))
    ledSetRGB(fb, 1, 32, 32, 0, 0);

  if ((val & (1 << 8)))
    ledSetRGB(fb, 2, 32, 32, 0, 0);

  if ((val & (1 << 8)) && (val & (1 << 9)))
    ledSetRGB(fb, 3, 32, 32, 0, 0);

  if ((val & (1 << 9)))
    ledSetRGB(fb, 4, 32, 32, 0, 0);

  if ((val & (1 << 10)) && (val & (1 << 9)))
    ledSetRGB(fb, 5, 32, 32, 0, 0);

  if ((val & (1 << 10)))
    ledSetRGB(fb, 6, 32, 32, 0, 0);

  if ((val & (1 << 1)))
    ledSetRGB(fb, 7, 32, 32, 0, 0);

  if ((val & (1 << 1)) && (val & (1 << 2)))
    ledSetRGB(fb, 8, 32, 32, 0, 0);

  if ((val & (1 << 2)))
    ledSetRGB(fb, 9, 32, 32, 0, 0);

  if ((val & (1 << 3)) && (val & (1 << 2)))
    ledSetRGB(fb, 10, 32, 32, 0, 0);

  if ((val & (1 << 3)))
    ledSetRGB(fb, 11, 32, 32, 0, 0);

  if ((val & (1 << 4)) && (val & (1 << 3)))
    ledSetRGB(fb, 12, 32, 32, 0, 0);

  if ((val & (1 << 4)))
    ledSetRGB(fb, 13, 32, 32, 0, 0);

  if ((val & (1 << 4)) && (val & (1 << 6)))
    ledSetRGB(fb, 14, 32, 32, 0, 0);

  if ((val & (1 << 7)) && (val & (1 << 6)))
    ledSetRGB(fb, 15, 32, 32, 0, 0);

  chSysLock();
  ledUpdate(fb, LED_COUNT);
  chSysUnlock();
}

static void rf_ready(eventid_t id) {

  (void)id;
  chprintf(stream, " [RF ready] ");
}

static void ble_ready(eventid_t id) {

  (void)id;
  chprintf(stream, " [BLE ready] ");
}

static void freefall(eventid_t id) {

  (void)id;
  chprintf(stream, "Unce. ");
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

  i2cStart(i2cDriver, &i2c_config);

  spiStart(&SPID1, &spi_config);
  spiStart(&SPID2, &spi_config);

  gpioxStart(i2cDriver);

  accelStart(i2cDriver);
  chargerStart(i2cDriver);
  captouchStart(i2cDriver);
  radioStart(&SPID1);
  oledStart(&SPID2);
  orchardEventsStart();
  ledStart(LED_COUNT, fb);

  evtTableHook(orchard_events, shell_terminated, shell_termination_handler);
  evtTableHook(orchard_events, captouch_release, key_mod);
  evtTableHook(orchard_events, captouch_press, key_mod);
  evtTableHook(orchard_events, ble_rdy, ble_ready);
  evtTableHook(orchard_events, accel_freefall, freefall);
  evtTableHook(orchard_events, rf_pkt_rdy, rf_ready);

  orchardShellRestart();

  while (TRUE)
    chEvtDispatch(evtHandlers(orchard_events), chEvtWaitOne(ALL_EVENTS));
}
