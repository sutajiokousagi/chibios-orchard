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

#include "orchard.h"
#include "orchard-shell.h"

#include "accel.h"

static int should_stop(void) {
  uint8_t bfr[1];
  return chnReadTimeout(serialDriver, bfr, sizeof(bfr), 1);
}

void cmd_accel(BaseSequentialStream *chp, int argc, char *argv[])
{

  (void)argv;
  (void)argc;
  struct accel_data d;
  chprintf(chp, "\r\nPress any key to quit\r\n");

  while (!should_stop()) {
    accelPoll(&d);
    chprintf(chp, "\rX: %5d  Y: %5d  Z: %5d", d.x, d.y, d.z);
  }
  chprintf(chp, "\r\n");
}

orchard_command("accel", cmd_accel);
