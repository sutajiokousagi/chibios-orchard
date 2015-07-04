#include "orchard-app.h"
#include "orchard-ui.h"
#include "accel.h"

#define BUMP_LIMIT 32  // 32 shakes to get to the limit
#define RETIRE_RATE 100 // retire one bump per "100ms"
static uint8_t bump_level = 0;

static void redraw_ui(void) {
  font_t font;
  coord_t width;
  coord_t fontheight, header_height;
  uint8_t fillwidth, intwidth;

  orchardGfxStart();
  font = gdispOpenFont("fixed_5x8");
  width = gdispGetWidth();
  intwidth = width / BUMP_LIMIT;
  intwidth = intwidth * BUMP_LIMIT;
  fontheight = gdispGetFontMetric(font, fontHeight);
  header_height = fontheight;

  gdispClear(Black);
  
  gdispDrawStringBox(0, header_height + 1 * fontheight, width, fontheight,
		     "shake it", font, White, justifyCenter);
  
  gdispDrawBox((width - intwidth) / 2, header_height + 4 * fontheight,
	       intwidth, fontheight, White);

  fillwidth = (bump_level * intwidth) / BUMP_LIMIT;
  gdispFillArea((width - intwidth) / 2, header_height + 4 * fontheight,
		fillwidth, fontheight, White);
  
  gdispCloseFont(font);
  gdispFlush();
  orchardGfxEnd();
}

static uint32_t shakebar_init(OrchardAppContext *context) {

  (void)context;
  return 0;
}

static void shakebar_start(OrchardAppContext *context) {

  (void)context;

  bump_level = 0;
  orchardAppTimer(context, RETIRE_RATE * 1000 * 1000, true);  // fire every 500ms to retire bumps
  redraw_ui();
  
}

static void shakebar_event(OrchardAppContext *context, const OrchardAppEvent *event) {

  (void)context;

  if (event->type == keyEvent) {
  }
  else if (event->type == timerEvent) {
    if( bump_level > 0 )
      bump_level--;
    redraw_ui();
  } else if( event->type == accelEvent ) {
    if( bump_level < BUMP_LIMIT )
      bump_level++;
    
    redraw_ui();
  }
}

static void shakebar_exit(OrchardAppContext *context) {

  (void)context;
}

orchard_app("shakebar", shakebar_init, shakebar_start, shakebar_event, shakebar_exit);
