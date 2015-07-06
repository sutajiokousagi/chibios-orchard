#include "orchard-app.h"
#include "orchard-ui.h"
#include "userconfig.h"

static uint8_t mode = 0;

static void redraw_ui(uint8_t mode) {
  char uiStr[32];
  
  coord_t width;
  coord_t height;
  coord_t tallheight;
  font_t font;
  font_t font2;
  const struct userconfig *config;

  orchardGfxStart();
  // draw the title bar
  font = gdispOpenFont("fixed_5x8");
  width = gdispGetWidth();
  height = gdispGetFontMetric(font, fontHeight);

  gdispClear(Black);
  gdispFillArea(0, 0, width, height, White);

  if( mode )
    chsnprintf(uiStr, sizeof(uiStr), "Your attempts at sex:");
  else
    chsnprintf(uiStr, sizeof(uiStr), "Fucked this many times:");
    
  gdispDrawStringBox(0, 0, width, height,
                     uiStr, font, Black, justifyCenter);

  gdispCloseFont(font);
  font2 = gdispOpenFont("DejaVuSans32");
  tallheight = gdispGetFontMetric(font2, fontHeight);

  config = getConfig();
  if( mode ) {
    chsnprintf(uiStr, sizeof(uiStr), "%d", config->sex_initiations );
  } else {
    chsnprintf(uiStr, sizeof(uiStr), "%d", config->sex_responses );
  }
  gdispDrawStringBox(0, height*2, width, tallheight,
		     uiStr, font2, White, justifyCenter);
  
  gdispCloseFont(font2);
  
  gdispFlush();
  orchardGfxEnd();
}

static uint32_t sexstats_init(OrchardAppContext *context) {

  (void)context;
  return 0;
}

static void sexstats_start(OrchardAppContext *context) {

  (void)context;

}

static void sexstats_event(OrchardAppContext *context, const OrchardAppEvent *event) {

  (void)context;
  if (event->type == keyEvent) {
    if ( (event->key.flags == keyDown) && ((event->key.code == keyLeft) || (event->key.code == keyRight)) ) {
      mode = !mode;
    } else if ( (event->key.flags == keyDown) && ((event->key.code == keyCW) || (event->key.code == keyCCW)) ) {
      mode = !mode;
    } else if( (event->key.flags == keyDown) && (event->key.code == keySelect) ) {
      orchardAppExit();
    }
  }

  redraw_ui(mode);
}

static void sexstats_exit(OrchardAppContext *context) {

  (void)context;
}

orchard_app("Sex stats", sexstats_init, sexstats_start, sexstats_event, sexstats_exit);
