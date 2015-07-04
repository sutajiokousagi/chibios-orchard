#include "orchard-app.h"
#include "orchard-events.h"
#include "orchard-ui.h"
#include <string.h>

static void list_redraw(OrchardAppContext *context) {
  font_t font;
  coord_t width;
  coord_t header_height;
  coord_t fontheight;
  uint8_t i;
  color_t text_color = White;
  color_t bg_color = Black;
  
  const char **itemhandles;
  
  orchardGfxStart();
  font = gdispOpenFont("fixed_5x8");
  width = gdispGetWidth();
  
  fontheight = gdispGetFontMetric(font, fontHeight);
  header_height = fontheight + 1;
  fontheight = fontheight + 1; // 5x8 font metric does not include adjacent space
  
  gdispClear(Black);
  
  // first item in context list is the title bar
  itemhandles = context->instance->uicontext->itemlist;
  if( itemhandles == NULL )
    return; // what else can we do?
  
  gdispFillArea(0, 0, width, header_height - 1, White); // a one pixel line btw header and list
  gdispDrawStringBox(0, 0, width, header_height - 1,
		     itemhandles[0], font, Black, justifyCenter);

  
  // now draw the list items, keeping track of which one is selected
  for( i = 0; i < context->instance->uicontext->total; i++ ) {
    if( i == context->instance->uicontext->selected ) {
      text_color = Black;
      bg_color = White;
    } else {
      text_color = White;
      bg_color = Black;
    }
    gdispFillArea(0, header_height + i * fontheight, width, fontheight, bg_color);
    gdispDrawStringBox(0, header_height + i * fontheight, width, fontheight,
		       itemhandles[i+1], font, text_color, justifyCenter);
  }
  
  gdispFlush();
  orchardGfxEnd();
}

static void list_start(OrchardAppContext *context) {

  list_redraw(context);
}

static void list_event(OrchardAppContext *context, const OrchardAppEvent *event) {
  struct OrchardUiContext *uicontext;

  uicontext = context->instance->uicontext;
  if (event->type == keyEvent) {
    if (event->key.flags == keyDown) {
      if( event->key.code == keyCW ) {
	uicontext->selected = (uicontext->selected + 1) % uicontext->total;
      } else if( event->key.code == keyCCW) {
	if( uicontext->selected == 0 )
	  uicontext->selected = uicontext->total - 1;
	else
	  uicontext->selected--;
      } else if( event->key.code == keySelect ) {
	context->instance->ui_result = (uint32_t) uicontext->selected;
	chEvtBroadcast(&ui_completed);
      }
    }
  }

  list_redraw(context);
}

static void list_exit(OrchardAppContext *context) {

  (void)context;
}

orchard_ui("list", list_start, list_event, list_exit);

