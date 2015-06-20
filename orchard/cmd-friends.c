#include "ch.h"
#include "shell.h"
#include "chprintf.h"

#include "orchard-shell.h"
#include "orchard-app.h"
#include "genes.h"

#include <stdlib.h>
#include <string.h>

#define FRIENDS_INIT_CREDIT 4

void cmd_friendlist(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  (void)argc;
  const char **friends;
  int i;
  
  friends = friendsGet();
  for(i = 0; i < MAX_FRIENDS; i++) {
    if( friends[i] != NULL ) {
      chprintf(chp, "%d: %d %s\n\r", i, (int) friends[i][0], &(friends[i][1]));
    }
  }
}

orchard_command("friendlist", cmd_friendlist);

void cmd_friendadd(BaseSequentialStream *chp, int argc, char *argv[]) {
  char **friends;
  uint32_t i;

  if (argc != 2) {
    chprintf(chp, "Usage: friendadd <index> <name>\r\n");
    return;
  }
  friends = (char **)friendsGet();
  
  i = strtoul(argv[0], NULL, 0);
  if( i > MAX_FRIENDS )
    i = MAX_FRIENDS;
   
  if( friends[i] != NULL ) {
    chprintf(chp, "Index used. Incrementing %s instead.\n\r", &(friends[i][1]));
    friends[i][0]++;
  } else {
    friends[i] = (char *) chHeapAlloc(NULL, GENE_NAMELENGTH + 2); // space for NULL + metric byte
    friends[i][0] = FRIENDS_INIT_CREDIT;
    strncpy(&(friends[i][1]), argv[1], GENE_NAMELENGTH);
  }
}
orchard_command("friendadd", cmd_friendadd);

void cmd_friendping(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void) chp;
  (void) argc;
  (void) argv;

  chEvtBroadcast(&radio_app);
}
orchard_command("friendping", cmd_friendping);
