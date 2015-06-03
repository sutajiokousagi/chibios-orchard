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

#include "ch.h"
#include "hal.h"
#include "shell.h"
#include "chprintf.h"

#include <strings.h>
#include <string.h>
#include <stdlib.h>

#include "orchard.h"
#include "orchard-shell.h"
#include "hex.h"

#include "ble.h"
#include "ble-service.h"

static void ble_echo(BaseSequentialStream *chp, int argc, char *argv[]) {

  if (argc != 2) {
    chprintf(chp, "Must specify a string to echo\r\n");
    return;
  }

  chprintf(chp, "Sending echo to BLE: %s\r\n", argv[1]);
  bleEcho(bleDriver, strlen(argv[1]), (void *)argv[1]);
}

static void ble_pump(BaseSequentialStream *chp, int argc, char *argv[]) {

  int timeout;

  if (argc != 2) {
    chprintf(chp, "No timeout specified, using defaults\r\n");
    timeout = 1000;
  }
  else
    timeout = strtoul(argv[1], NULL, 0);

  chprintf(chp, "State: %d  Polling, waiting for %d ms...\r\n",
      bleDeviceState(bleDriver), timeout);
  blePoll(bleDriver, timeout);
}

static void ble_bond(BaseSequentialStream *chp, int argc, char *argv[]) {

  uint16_t timeout;
  uint16_t interval;

  timeout = 180; /* seconds */
  interval = 0x50; /* milliseconds */

  if (argc > 1)
    timeout = strtoul(argv[1], NULL, 0);

  if (argc > 2)
    interval = strtoul(argv[2], NULL, 0);

  chprintf(chp, "Enabling bonding broadcast for %d seconds, every %d ms\r\n",
      timeout, interval);
  bleBond(bleDriver, timeout, interval);
}

static void ble_reset(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argc;
  (void)argv;

  chprintf(chp, "Resetting BLE radio...\r\n");
  bleReset(bleDriver);
}

static void ble_kbd(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argc;
  (void)argv;

  chprintf(chp, "Starting BLE HID keyboard...\r\n");
  bleSetup(bleDriver, &hid_keyboard);
}

static void ble_bc(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argc;
  (void)argv;

  chprintf(chp, "Starting BLE broadcast...\r\n");
  bleSetup(bleDriver, &ble_broadcast);
}

static void cmd_ble(BaseSequentialStream *chp, int argc, char *argv[])
{

  if (argc == 0) {
    chprintf(chp, "BLE commands:\r\n");
    chprintf(chp, "   echo [msg]         Send string to BLE radio\r\n");
    chprintf(chp, "   pump [timeout ms]  Poll BLE chip, with optional timeout\r\n");
    chprintf(chp, "   bc                 Set up BLE as a broadcast device\r\n");
    chprintf(chp, "   kbd                Set up BLE as a keyboard\r\n");
    chprintf(chp, "   reset              Reset BLE radio completely\r\n");
    chprintf(chp, "   bond [timeout] [interval]  Pair with something\r\n");
    return;
  }

  if (!strcasecmp(argv[0], "echo"))
    ble_echo(chp, argc, argv);
  else if (!strcasecmp(argv[0], "pump"))
    ble_pump(chp, argc, argv);
  else if (!strcasecmp(argv[0], "kbd"))
    ble_kbd(chp, argc, argv);
  else if (!strcasecmp(argv[0], "bc"))
    ble_bc(chp, argc, argv);
  else if (!strcasecmp(argv[0], "reset"))
    ble_reset(chp, argc, argv);
  else if (!strcasecmp(argv[0], "bond"))
    ble_bond(chp, argc, argv);
  else
    chprintf(chp, "Unrecognized BLE command\r\n");
}

orchard_command("ble", cmd_ble);
