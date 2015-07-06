#include "orchard-app.h"
#include "orchard-ui.h"
#include "userconfig.h"

static void redraw_ui(void) {
  char tmp[] = "Configure Options";
  char uiStr[32];
  
  coord_t width;
  coord_t height;
  font_t font;
  color_t draw_color = White;
  const struct userconfig *config;

  orchardGfxStart();
  // draw the title bar
  font = gdispOpenFont("fixed_5x8");
  width = gdispGetWidth();
  height = gdispGetFontMetric(font, fontHeight);

  gdispClear(Black);
  gdispFillArea(0, 0, width, height, White);
  gdispDrawStringBox(0, 0, width, height,
                     tmp, font, Black, justifyCenter);

  // 1st line: configure autosex
  chsnprintf(uiStr, sizeof(uiStr), "Down To Fuck?");
  gdispDrawStringBox(0, height, width, height,
		     uiStr, font, White, justifyCenter);

  config = getConfig();
  // 2nd line: autosex prompt (left)
  chsnprintf(uiStr, sizeof(uiStr), "Always!");
  if( config->cfg_autosex ) {
    gdispFillArea(0, height*2, width/2, height, White);
    draw_color = Black;
  } else {
    draw_color = White;
  }
  gdispDrawStringBox(0, height*2, width/2, height,
		     uiStr, font, draw_color, justifyCenter);

  // 2nd line: autosex prompt (right)
  chsnprintf(uiStr, sizeof(uiStr), "Ask");
  if( !config->cfg_autosex ) {
    gdispFillArea(width/2, height*2, width/2, height, White);
    draw_color = Black;
  } else {
    draw_color = White;
  }
  gdispDrawStringBox(width/2, height*2, width/2, height,
		     uiStr, font, draw_color, justifyCenter);
  
  gdispFlush();
  orchardGfxEnd();
}

static uint32_t config_init(OrchardAppContext *context) {

  (void)context;
  return 0;
}

static void config_start(OrchardAppContext *context) {

  (void)context;

}

static void config_event(OrchardAppContext *context, const OrchardAppEvent *event) {

  (void)context;

  if (event->type == keyEvent) {
    if ( (event->key.flags == keyDown) && ((event->key.code == keyCW) || (event->key.code == keyCCW)) ) {
      configToggleAutosex();
    } else if( (event->key.flags == keyDown) && (event->key.code == keySelect) ) {
      orchardAppExit();
    }
  }

  redraw_ui();
}

static void config_exit(OrchardAppContext *context) {

  (void)context;
}

orchard_app("User options", config_init, config_start, config_event, config_exit);
