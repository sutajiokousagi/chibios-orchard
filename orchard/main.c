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

static void shell_termination_handler(eventid_t id)
{
  static int i = 1;
  (void)id;

  chprintf(stream, "\r\nRespawning shell (shell #%d)\r\n", ++i);
  orchardShellRestart();
}

#if 0
/* Triggered when the button is pressed. The blue led is toggled. */
static void extcb1(EXTDriver *extp, expchannel_t channel)
{
  (void)extp;
  (void)channel;

  palTogglePad(IOPORT4, 1);
}

static const EXTConfig extcfg = {
  {
   {EXT_CH_MODE_FALLING_EDGE | EXT_CH_MODE_AUTOSTART, extcb1, PORTA, 1}
  }
};
#endif

static evhandler_t event_handlers[] = {
  shell_termination_handler,
};

static event_listener_t event_listeners[ARRAY_SIZE(event_handlers)];

static THD_WORKING_AREA(waToggleThread, 16);
static msg_t toggle_thread(void *arg)
{
  (void)arg;

  //chSysLock();
  while (1) {
    chThdYield();
    FGPIOD->PSOR = (1 << 6);
    FGPIOD->PCOR = (1 << 6);
    FGPIOD->PSOR = (1 << 6);
    FGPIOD->PCOR = (1 << 6);
    FGPIOD->PSOR = (1 << 6);
    FGPIOD->PCOR = (1 << 6);
    FGPIOD->PSOR = (1 << 6);
    FGPIOD->PCOR = (1 << 6);
    FGPIOD->PSOR = (1 << 6);
    FGPIOD->PCOR = (1 << 6);
    FGPIOD->PSOR = (1 << 6);
    FGPIOD->PCOR = (1 << 6);
    FGPIOD->PSOR = (1 << 6);
    FGPIOD->PCOR = (1 << 6);
    FGPIOD->PSOR = (1 << 6);
    FGPIOD->PCOR = (1 << 6);
  }
  //chSysUnlock();

  return MSG_OK;
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

  /* Start serial, so we can get status output.*/
  orchardShellInit();
  chEvtRegister(&shell_terminated, &event_listeners[0], 0);


#if 0
  /* Turn the LEDs OFF */
  palSetPad(IOPORT2, 18);
  palSetPad(IOPORT2, 19);
  palSetPad(IOPORT4, 1);

  /*
   * Activates the EXT driver 1.
   */
  palSetPadMode(IOPORT1, 1, PAL_MODE_INPUT_PULLUP);
  extStart(&EXTD1, &extcfg);
#endif

  orchardShellRestart();

  chprintf(stream, "Orchard shell.  Based on build %s\r\n", gitversion);
  chThdCreateStatic(waToggleThread, sizeof(waToggleThread),
                    LOWPRIO, toggle_thread, NULL);

  while (TRUE)
    chEvtDispatch(event_handlers, chEvtWaitOne(ALL_EVENTS));
}
