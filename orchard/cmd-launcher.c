#include <strings.h>
#include "orchard-shell.h"
#include "orchard-app.h"

void cmd_launch(BaseSequentialStream *chp, int argc, char *argv[])
{
  const OrchardApp *launcher = orchard_app_start();

  (void)argc;
  (void)argv;

  if (argc) {
    while (launcher->name) {
      if (!strcasecmp(launcher->name, argv[0]))
        break;
      launcher++;
    }

    /* No match found, print help */
    if (!launcher->name) {
      launcher = orchard_app_start();
      chprintf(chp, "Usage: launch [app]\r\n");
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

orchard_command("launch", cmd_launch);
