#include "ch.h"
#include "shell.h"
#include "chprintf.h"

#include "orchard-shell.h"
#include "orchard-app.h"

void cmd_friendlist(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  (void)argc;
  const char **friends;
  int i;
  
  friends = friendsGet();
  for(i = 0; i < MAX_FRIENDS; i++) {
    if( friends[i] != NULL ) {
      chprintf(chp, "%d %s\n\r", (int) friends[i][0], &(friends[i][1]));
    }
  }
}

orchard_command("friendlist", cmd_friendlist);
