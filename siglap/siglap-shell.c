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
#include "siglap.h"
#include "siglap-shell.h"

/* Global stream variable, lets modules use chprintf().*/
void *stream;

void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]);

static const ShellCommand shellCommands[] = {
  {"mem", cmd_mem},
  {"threads", cmd_threads},
  {NULL, NULL}
};

static const ShellConfig shellConfig = {
  stream_driver,
  shellCommands
};

static const SerialConfig serialConfig = {
  115200,
};

static thread_t *shell_tp = NULL;
static THD_WORKING_AREA(waShellThread, 256);

void siglapShellInit(void)
{
  sdStart(serialDriver, &serialConfig);
  stream = stream_driver;

  shellInit();
}

void siglapShellRestart(void)
{
  /* Recovers memory of the previous shell. */
  if (shell_tp && chThdTerminatedX(shell_tp))
    chThdRelease(shell_tp);
  shell_tp = shellCreateStatic(&shellConfig, waShellThread,
                              sizeof(waShellThread), NORMALPRIO - 5);
}
