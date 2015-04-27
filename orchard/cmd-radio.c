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
#include "orchard-shell.h"

#include "radio.h"

static inline int isprint(int c)
{
  return c > 32 && c < 127;
}

int print_hex_offset(BaseSequentialStream *chp,
                     const void *block, int count, int offset, uint32_t start)
{

  int byte;
  const uint8_t *b = block;

  count += offset;
  b -= offset;
  for ( ; offset < count; offset += 16) {
    chprintf(chp, "%08x", start + offset);

    for (byte = 0; byte < 16; byte++) {
      if (byte == 8)
        chprintf(chp, " ");
      chprintf(chp, " ");
      if (offset + byte < count)
        chprintf(chp, "%02x", b[offset + byte] & 0xff);
      else
        chprintf(chp, "  ");
    }

    chprintf(chp, "  |");
    for (byte = 0; byte < 16 && byte + offset < count; byte++)
      chprintf(chp, "%c", isprint(b[offset + byte]) ?  b[offset + byte] : '.');
    chprintf(chp, "|\r\n");
  }
  return 0;
}

int print_hex(BaseSequentialStream *chp,
              const void *block, int count, uint32_t start)
{
  return print_hex_offset(chp, block, count, 0, start);
}

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

static void radio_dump(BaseSequentialStream *chp, int argc, char *argv[]) {

  unsigned int addr, count;

  if (argc != 3) {
    chprintf(chp, "No address/count specified\r\n");
    return;
  }

  addr  = strtoul(argv[1], NULL, 0);
  count = strtoul(argv[2], NULL, 0);
  if (count > 32) {
    chprintf(chp, "Error: Cannot request more than 32 bytes\r\n");
    return;
  }

  uint8_t buf[count];

  chprintf(chp, "Dumping %d bytes from address 0x%02x:\r\n", count, addr);
  radioDump(addr, buf, count);

  print_hex_offset(chp, buf, count, 0, addr);
}

static void cmd_radio(BaseSequentialStream *chp, int argc, char *argv[])
{

  (void)argv;
  (void)argc;

  if (argc == 0) {
    chprintf(chp, "Radio commands:\r\n");
    chprintf(chp, "   get [addr]           Get a SPI register\r\n");
    chprintf(chp, "   set [addr] [val]     Set a SPI register\r\n");
    chprintf(chp, "   dump [addr] [count]  Dump a set of SPI registers\r\n");
    return;
  }

  if (!strcasecmp(argv[0], "get"))
    radio_get(chp, argc, argv);
  else if (!strcasecmp(argv[0], "set"))
    radio_set(chp, argc, argv);
  else if (!strcasecmp(argv[0], "dump"))
    radio_dump(chp, argc, argv);
  else
    chprintf(chp, "Unrecognized radio command\r\n");
}

orchard_command("radio", cmd_radio);
