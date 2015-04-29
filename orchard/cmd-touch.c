
#include "ch.h"
#include "hal.h"

#include "orchard-shell.h"
#include "captouch.h"

static void cmd_touch(BaseSequentialStream *buf, int argc, char **argv) {

  uint16_t val;

  (void)argc;
  (void)argv;

  chprintf(buf, "Captouch value: ");
  val = captouchRead();
  chprintf(buf, "0x%04x\r\n", val);
}

orchard_command("touch", cmd_touch);
