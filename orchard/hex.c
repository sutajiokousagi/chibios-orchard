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
#include "chprintf.h"

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
