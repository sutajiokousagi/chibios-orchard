#include "orchard-app.h"
#include "led.h"

static uint32_t led_init(OrchardAppContext *context) {

  (void)context;
  chprintf(stream, "LED: Initializing led app\r\n");
  return 0;
}

static void led_start(OrchardAppContext *context) {

  (void)context;
  chprintf(stream, "LED: Starting led app\r\n");
}

static void led_event(OrchardAppContext *context, OrchardAppEvent *event) {

  (void)context;
  uint8_t shift;
  
  chprintf(stream, "LED: Received %d event\r\n", event->type);

  if (event->type == keyEvent) {
    if (event->key.code == keyCW)
      ; // something
    else if (event->key.code == keyCCW) {
      ; // something more
    }
    else if (event->key.code == keyLeft) {
      shift = getShift();
      shift++;
      if( shift > 6 )
	shift = 0;
      setShift(shift);
    } else if (event->key.code == keyRight)
      effectsNextPattern();
    else if (event->key.code == keySelect)
      ; // something
  }
}

static void led_exit(OrchardAppContext *context) {

  (void)context;
  chprintf(stream, "LED: Exiting led app\r\n");
}

orchard_app("Light display (default)", led_init, led_start, led_event, led_exit);
