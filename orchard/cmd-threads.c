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
#include "shell.h"
#include "chprintf.h"

#include "orchard-shell.h"

static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[])
{
  static const char *states[] = {CH_STATE_NAMES};
  thread_t *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: threads\r\n");
    return;
  }
  chprintf(chp, " addr     pc       stack    prio refs state         name\r\n");
  tp = chRegFirstThread();
  do {
    chprintf(chp, " %-8lx %-8lx %-8lx %4lu %4lu %-12s  %-10s\r\n",
      (uint32_t)tp, (uint32_t)tp->p_ctx.r13->lr, (uint32_t)tp->p_ctx.r13,
      (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
      states[tp->p_state],
      tp->p_name);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
}

orchard_command("threads", cmd_threads);
