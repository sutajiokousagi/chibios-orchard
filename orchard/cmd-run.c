#include <strings.h>
#include "orchard-shell.h"
#include "orchard-app.h"

void cmd_run(BaseSequentialStream *chp, int argc, char *argv[])
{
  const OrchardApp *launcher = orchard_app_start();

  if (argc) {
    int i;

    /* This is a bad way to do it */
    for (i = 1; i < argc; i++)
      argv[i][-1] = ' ';

    while (launcher->name) {
      if (!strcasecmp(launcher->name, argv[0]))
        break;
      launcher++;
    }

    /* No match found, print help */
    if (!launcher->name) {
      launcher = orchard_app_start();
      chprintf(chp, "Usage: run [app]\r\n");
      chprintf(chp, "    Available apps:\r\n");
      while (launcher->name) {
        chprintf(chp, "        ");
        chprintf(chp, launcher->name);
        chprintf(chp, "\r\n");
        launcher++;
      }
      return;
    }
  }
    
  chprintf(chp, "Running %s...\r\n", launcher->name);
  orchardAppRun(launcher);
}

orchard_command("run", cmd_run);
