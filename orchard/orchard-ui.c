#include "orchard-ui.h"
#include <string.h>

const OrchardUi *getUiByName(const char *name) {
  const OrchardUi *current;

  current = orchard_ui_start();
  while(current->name) {
    if( !strncmp(name, current->name, 16) ) {
      return current;
    }
    current++;
  }
  return NULL;
}
