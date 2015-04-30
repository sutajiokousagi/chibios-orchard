#include "orchard-shell.h"
#include "radio.h"

void cmd_temp(BaseSequentialStream *chp, int argc, char **argv) {

  int temp;

  (void)argc;
  (void)argv;

  chprintf(chp, "System temperature: ");
  temp = radioTemperature();

  chprintf(chp, "%d C\r\n", temp);
}

orchard_command("temp", cmd_temp);
