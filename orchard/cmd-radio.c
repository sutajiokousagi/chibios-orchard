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
#include <stdlib.h>

#include "orchard.h"

#include "radio.h"

static void radio_get(BaseSequentialStream *chp, int argc, char *argv[]) {

  int addr;

  if (argc != 2) {
    chprintf(chp, "No address specified\r\n");
    return;
  }

  addr = strtoul(argv[1], NULL, 0);
  chprintf(chp, "Value at address 0x%02x: ", addr);
  chprintf(chp, "0x%02x\r\n", radioRead(addr));
}

static void radio_set(BaseSequentialStream *chp, int argc, char *argv[]) {

  int addr, val;

  if (argc != 3) {
    chprintf(chp, "No address/val specified\r\n");
    return;
  }

  addr = strtoul(argv[1], NULL, 0);
  val  = strtoul(argv[2], NULL, 0);
  chprintf(chp, "Writing address 0x%02x: ", addr);
  radioWrite(addr, val);
  chprintf(chp, "0x%02x\r\n", val);
}

void cmd_radio(BaseSequentialStream *chp, int argc, char *argv[])
{

  (void)argv;
  (void)argc;

  if (argc == 0) {
    chprintf(chp, "Radio commands:\r\n");
    chprintf(chp, "   get [addr]         Get a SPI register\r\n");
    chprintf(chp, "   set [addr] [val]   Set a SPI register\r\n");
    return;
  }

  if (!strcasecmp(argv[0], "get"))
    radio_get(chp, argc, argv);
  else if (!strcasecmp(argv[0], "set"))
    radio_set(chp, argc, argv);
  else
    chprintf(chp, "Unrecognized radio command\r\n");
}
