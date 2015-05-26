#include "orchard-app.h"
#include "orchard-ui.h"

struct OrchardUiContext textUiContext;

static void redraw_ui(void) {
  char tmp[] = "Enter your name";
  
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

static uint32_t name_init(OrchardAppContext *context) {

  (void)context;
  chprintf(stream, "NAME: Initializing name app\r\n");
  return 0;
}

static void name_start(OrchardAppContext *context) {
  const OrchardUi *textUi;
  
  chprintf(stream, "NAME: Starting name app\r\n");

  redraw_ui();
  // all this app does is launch a text entry box and store the name
  textUi = getUiByName("textentry");
  textUiContext.itemlist = NULL;
  if( textUi != NULL ) {
    context->instance->uicontext = &textUiContext;
    context->instance->ui = textUi;
  }
  textUi->start(context);
  
}

void name_event(OrchardAppContext *context, const OrchardAppEvent *event) {

  (void)context;
  uint8_t shift;
  
  chprintf(stream, "NAME: Received %d event\r\n", event->type);

  if (event->type == keyEvent) {
    if (event->key.code == keyCW)
      ; // something
    else if (event->key.code == keyCCW) {
      ; // something more
    }
    else if ((event->key.code == keyLeft)  && (event->key.flags == keyDown) ) {
      shift = getShift();
      shift++;
      if( shift > 6 )
	shift = 0;
      setShift(shift);
    } else if ( (event->key.code == keyRight) && (event->key.flags == keyDown))
      effectsNextPattern();
    else if (event->key.code == keySelect)
      ; // something
  }
}

static void name_exit(OrchardAppContext *context) {

  (void)context;
  chprintf(stream, "NAME: Exiting name app\r\n");
}

orchard_app("Set your name", name_init, name_start, name_event, name_exit);


