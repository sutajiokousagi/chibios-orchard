#include "orchard-app.h"
#include "charger.h"
#include "gasgauge.h"

static void redraw_ui(void) {
  coord_t width;
  coord_t height;
  font_t font;

  orchardGfxStart();
  // draw the title bar
  font = gdispOpenFont("fixed_5x8");
  width = gdispGetWidth();
  height = gdispGetFontMetric(font, fontHeight);

  // draw title box
  gdispClear(Black);

  gdispDrawStringBox(0, height * 2, width, height,
                     "Powering down...", font, White, justifyCenter);
  
  gdispDrawStringBox(0, height * 4, width, height,
                     "Note: press reset", font, White, justifyCenter);
  gdispDrawStringBox(0, height * 5, width, height,
                     "to power on again", font, White, justifyCenter);
  
  gdispFlush();
  gdispCloseFont(font);
  orchardGfxEnd();
}


static uint32_t shipmode_init(OrchardAppContext *context) {

  (void)context;
  return 0;
}

static void shipmode_start(OrchardAppContext *context) {

  (void)context;

  redraw_ui();
  /// NOTE TO SELF: implement flush of volatile genome state (such as playtime and favorites)
  /// to FLASH before going into power off
  halt();
}

static void shipmode_event(OrchardAppContext *context, const OrchardAppEvent *event) {

  (void)context;
}

static void shipmode_exit(OrchardAppContext *context) {

  (void)context;
}

orchard_app("power-off", shipmode_init, shipmode_start, shipmode_event, shipmode_exit);
