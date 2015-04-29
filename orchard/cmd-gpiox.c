#include "orchard-shell.h"
#include "gpiox.h"

static void gpiox(BaseSequentialStream *chp, int argc, char **argv) {

  int i;

  (void)argc;
  (void)argv;

  for (i = 0; i < GPIOX_NUM_PADS; i++)
    chprintf(chp, "GPIO %d: %d\r\n", i, gpioxReadPad(GPIOX, i));

}

orchard_command("gpiox", gpiox);
