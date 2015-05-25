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
#include "flash.h"

void cmd_flashsec(BaseSequentialStream *chp, int argc, char *argv[]) {
  uint8_t securityStatus;
  
  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: flashsec\r\n");
    return;
  }

  securityStatus = flashGetSecurity();
  /**************************************************
        Message to user on flash security state
        #define FLASH_NOT_SECURE                   0x01
        #define FLASH_SECURE_BACKDOOR_ENABLED      0x02
        #define FLASH_SECURE_BACKDOOR_DISABLED     0x04
  ****************************************************/
  switch(securityStatus) {
  case FLASH_NOT_SECURE:
  default:
    chprintf(stream, "Flash status: unsecure.\n\r");
    break;
  case FLASH_SECURE_BACKDOOR_ENABLED:
    chprintf(stream, "Flash status: secure, backdoor enabled.\n\r");
    break;
  case FLASH_SECURE_BACKDOOR_DISABLED:
    chprintf(stream, "Flash status: secure, backdoor disabled.\n\r");
    break;
  }
  
}
orchard_command("flashsec", cmd_flashsec);

void cmd_flasherase(BaseSequentialStream *chp, int argc, char *argv[]) {
  uint16_t numSec;
  uint32_t offset;
  
  (void)argv;
  if (argc != 2) {
    chprintf(chp, "Usage: flasherase <sector number> <number of 1k sectors>\r\n");
    return;
  }
  
  offset = strtoul(argv[0], NULL, 0);
  numSec = strtoul(argv[1], NULL, 0);
  
  flashErase(offset, numSec);
}
orchard_command("flasherase", cmd_flasherase);

static void dump(uint8_t *byte, uint32_t count) {
  uint32_t i;

  for( i = 0; i < count; i++ ) {
    if( (i % 16) == 0 ) {
      chprintf(stream, "\n\r%08x: ", ((uint32_t) byte) + i );
    }
    chprintf(stream, "%02x ", byte[i] );
  }
  chprintf(stream, "\n\r" );
}

#define TESTSIZE 64
void cmd_flashtest(BaseSequentialStream *chp, int argc, char *argv[]) {
  uint8_t testdat[TESTSIZE];
  uint8_t i;
  uint8_t *dest;
  int8_t ret;
  
  if (argc != 1) {
    chprintf(chp, "Usage: flashtest <destination hex address>\r\n");
    return;
  }
  dest = (uint8_t *) strtoul(argv[0], NULL, 16);
  
  chprintf(chp, "Flash test routine.\n\r");
  for( i = 0; i < TESTSIZE; i++ ) {
    testdat[i] = (uint8_t) rand();
  }

  chprintf(chp, "Before programming: \n\r" );
  dump( dest, TESTSIZE );
  
  ret = flashProgram( testdat, dest, TESTSIZE ); 
  chprintf(chp, "Return value is %d\n\r", ret );

  chprintf(chp, "After programming: \n\r" );
  dump( dest, TESTSIZE );
  chprintf(chp, "Reference data: \n\r" );
  dump( testdat, TESTSIZE );
}
orchard_command("flashtest", cmd_flashtest);
