
#include "ch.h"
#include "hal.h"

#include "orchard.h"
#include "orchard-shell.h"
#include "captouch.h"

static int should_stop(void) {
  uint8_t bfr[1];
  return chnReadTimeout(serialDriver, bfr, sizeof(bfr), 1);
}

static void cmd_touch(BaseSequentialStream *buf, int argc, char **argv) {

  uint16_t val;

  (void)argc;
  (void)argv;

  chprintf(buf, "Captouch value: \n\r");
  while( !should_stop() ) {
    val = captouchRead();
    chprintf(buf, "0x%04x\r", val);
  }
  chprintf(buf, "\r\n");
  
}

orchard_command("touch", cmd_touch);
