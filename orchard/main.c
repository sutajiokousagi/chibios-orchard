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

#include "shell.h"
#include "chprintf.h"

#include "orchard.h"
#include "orchard-shell.h"

#include "accel.h"

static void shell_termination_handler(eventid_t id)
{
  static int i = 1;
  (void)id;

  chprintf(stream, "\r\nRespawning shell (shell #%d)\r\n", ++i);
  orchardShellRestart();
}

static evhandler_t event_handlers[] = {
  shell_termination_handler,
};

static event_listener_t event_listeners[ARRAY_SIZE(event_handlers)];

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

  orchardShellInit();
  chEvtRegister(&shell_terminated, &event_listeners[0], 0);

  chprintf(stream, "Orchard shell.  Based on build %s\r\n", gitversion);

  i2cStart(i2cDriver, NULL);
  accelStart(i2cDriver);

  orchardShellRestart();

  while (TRUE)
    chEvtDispatch(event_handlers, chEvtWaitOne(ALL_EVENTS));
}
