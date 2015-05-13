#include "orchard-app.h"
#include "charger.h"

static uint32_t boost_init(OrchardAppContext *context) {

  (void)context;
  chprintf(stream, "BOOST: Initializing boost app\r\n");
  return 0;
}

static void boost_start(OrchardAppContext *context) {

  (void)context;
  chargerIntent cstate;

  chprintf(stream, "BOOST: Starting boost app\r\n");
  cstate = chargerCurrentIntent();
  if( cstate != CHG_BOOST ) {
    chargerBoostIntent(1);
  } else {
    chargerBoostIntent(0);
  }
}

static void boost_event(OrchardAppContext *context, OrchardAppEvent *event) {

  (void)context;
  chprintf(stream, "BOOST: Received %d event\r\n", event->type);
}

static void boost_exit(OrchardAppContext *context) {

  (void)context;
  chprintf(stream, "BOOST: Exiting boost app\r\n");
}

orchard_app("boost", boost_init, boost_start, boost_event, boost_exit);
