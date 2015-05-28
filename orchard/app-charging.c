#include "orchard-app.h"
#include "orchard-ui.h"
#include <string.h>


static void redraw_ui(void) {
  char tmp[] = "Charger status";
  
  coord_t width;
  coord_t height;
  font_t font;

  // draw the title bar
  font = gdispOpenFont("fixed_5x8");
  width = gdispGetWidth();
  height = gdispGetFontMetric(font, fontHeight);

  gdispClear(Black);
  gdispFillArea(0, 0, width, height, White);
  gdispDrawStringBox(0, 0, width, height,
                     tmp, font, Black, justifyCenter);

  gdispFlush();
}

static uint32_t charging_init(OrchardAppContext *context) {

  (void)context;

  return 0;
}

static void charging_start(OrchardAppContext *context) {
  const OrchardUi *textUi;
  
  orchardAppTimer(context, 500 * 1000 * 1000, true); // update UI every 500 ms
  
  redraw_ui();
  // all this app does is launch a text entry box and store the name
}

void charging_event(OrchardAppContext *context, const OrchardAppEvent *event) {

  (void)context;
  uint8_t shift;
  
  if (event->type == keyEvent) {
    if ( (event->key.flags == keyDown) && (event->key.code == keyLeft) ) {
      orchardAppExit();
    }
  } else if (event->type == timerEvent) {
    redraw_ui();
  }
}

static void charging_exit(OrchardAppContext *context) {

  (void)context;
}

orchard_app("Charging settings", charging_init, charging_start, charging_event, charging_exit);


