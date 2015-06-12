#include "orchard-app.h"
#include "orchard-test.h"

static uint32_t testmode_init(OrchardAppContext *context) {

  (void)context;
  return 0;
}

static void testmode_start(OrchardAppContext *context) {
  (void)context;
  
  orchardTestPrompt("Testing...", "", 0);
}

static void testmode_event(OrchardAppContext *context, const OrchardAppEvent *event) {

  (void)context;
  // dummy app that does nothing with events sent to it.
  // tests will create events and having any other app running could interfere with tests
}

static void testmode_exit(OrchardAppContext *context) {

  (void)context;
}

orchard_app("~testmode", testmode_init, testmode_start, testmode_event, testmode_exit);
