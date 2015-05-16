#include "orchard-app.h"
#include "charger.h"

static uint32_t shipmode_init(OrchardAppContext *context) {

  (void)context;
  chprintf(stream, "SHIPMODE: Initializing shipmode app\r\n");
  return 0;
}

static void shipmode_start(OrchardAppContext *context) {

  (void)context;

  chprintf(stream, "SHIPMODE: Starting shipmode app\r\n");
  chargerShipMode();
}

static void shipmode_event(OrchardAppContext *context, OrchardAppEvent *event) {

  (void)context;
  chprintf(stream, "SHIPMODE: Received %d event\r\n", event->type);
}

static void shipmode_exit(OrchardAppContext *context) {

  (void)context;
  chprintf(stream, "SHIPMODE: Exiting shipmode app\r\n");
}

orchard_app("poweroff", shipmode_init, shipmode_start, shipmode_event, shipmode_exit);
