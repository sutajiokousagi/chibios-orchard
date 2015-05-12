#include "orchard-app.h"

static uint32_t test_init(OrchardAppContext *context) {

  (void)context;
  chprintf(stream, "TEST: Initializing test app\r\n");
  return 0;
}

static void test_start(OrchardAppContext *context) {

  (void)context;

  chprintf(stream, "TEST: Setting up a timer to fire every 2000 msec\r\n");
  orchardAppTimer(context, 2000 * 1000 * 1000, true);
}

static void test_event(OrchardAppContext *context, const OrchardAppEvent *event) {

  (void)context;

  chprintf(stream, "TEST: Received event %d - ", event->type);

  if (event->type == keyEvent) {
    if (event->key.flags == keyDown)
      chprintf(stream, "Got keydown for %d\r\n", event->key.code);
    else if (event->key.flags == keyUp)
      chprintf(stream, "Got keyup for %d\r\n", event->key.code);
    else
      chprintf(stream, "Got unknown event for %d\r\n", event->key.code);
  }
  else if (event->type == appEvent) {
    chprintf(stream, "App lifetime event: ");
    if (event->app.event == appTerminate)
      chprintf(stream, "Terminating\r\n");
    else if (event->app.event == appStart)
      chprintf(stream, "Starting up\r\n");
    else
      chprintf(stream, "Unknown event\r\n");
  }
  else if (event->type == timerEvent)
    chprintf(stream, "Timer event: %d usec\r\n", event->timer.usecs);
  else
    chprintf(stream, "Unrecognized event\r\n");
}

static void test_exit(OrchardAppContext *context) {

  (void)context;
  chprintf(stream, "TEST: Exiting test app\r\n");
}

orchard_app("Test", test_init, test_start, test_event, test_exit);
