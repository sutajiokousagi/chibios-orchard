#include "orchard-app.h"

static uint32_t test_init(OrchardAppContext *context) {

  (void)context;
  chprintf(stream, "TEST: Initializing test app\r\n");
  return 0;
}

static void test_start(OrchardAppContext *context) {

  (void)context;
  chprintf(stream, "TEST: Starting test app\r\n");
}

static void test_event(OrchardAppContext *context, OrchardAppEvent *event) {

  (void)context;
  chprintf(stream, "TEST: Received %d event\r\n", event->type);
}

static void test_exit(OrchardAppContext *context) {

  (void)context;
  chprintf(stream, "TEST: Exiting test app\r\n");
}

orchard_app("Test", test_init, test_start, test_event, test_exit);
