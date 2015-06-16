#include "orchard-app.h"
#include "orchard-ui.h"

#include "storage.h"

#include <string.h>

static  const char title[] = "header";
static  const char item1[] = "item 1";
static  const char item2[] = "item 2";
static  const char item3[] = "item 3";

struct OrchardUiContext listUiContext;
uint8_t selected = 0;

static void redraw_ui(void) {
  char uiStr[16];
  
  coord_t width;
  coord_t height;
  font_t font;

  orchardGfxStart();

  font = gdispOpenFont("fixed_5x8");
  width = gdispGetWidth();
  height = gdispGetFontMetric(font, fontHeight);

  gdispClear(Black);

  chsnprintf(uiStr, sizeof(uiStr), "selected: %d", selected);
  gdispDrawStringBox(0, height * 2, width, height,
		     uiStr, font, White, justifyCenter);
  
  gdispFlush();
  orchardGfxEnd();
}

static uint32_t testlist_init(OrchardAppContext *context) {

  (void)context;

  return 0;
}

#define TEST_NUM_ITEMS  4
static void testlist_start(OrchardAppContext *context) {
  const OrchardUi *listUi;
  
  redraw_ui();
  listUi = getUiByName("list");
  
  listUiContext.total = 3;
  listUiContext.selected = 0;
  
  listUiContext.itemlist = (char **) chHeapAlloc(NULL, sizeof(char *) * 4);
  if( listUiContext.itemlist == NULL )
    return;
  listUiContext.itemlist[0] = title;
  listUiContext.itemlist[1] = item1;
  listUiContext.itemlist[2] = item2;
  listUiContext.itemlist[3] = item3;
  
  if( listUi != NULL ) {
    context->instance->uicontext = &listUiContext;
    context->instance->ui = listUi;
  }
  listUi->start(context);
}

void testlist_event(OrchardAppContext *context, const OrchardAppEvent *event) {

  (void)context;
  
  if (event->type == keyEvent) {
    if ( (event->key.flags == keyDown) && (event->key.code == keyLeft) ) {
      orchardAppExit();
      redraw_ui();
    }
  } else if( event->type == uiEvent ) {
    if(( event->ui.code == uiComplete ) && ( event->ui.flags == uiOK )) {
      selected = (uint8_t) context->instance->ui_result;
      context->instance->ui = NULL;
      context->instance->uicontext = NULL;
      redraw_ui();
    }
  }
}

static void testlist_exit(OrchardAppContext *context) {
  (void)context;

  // free all malloc'd list data
  chHeapFree(listUiContext.itemlist);
  
}

orchard_app("Test UI lists", testlist_init, testlist_start, testlist_event, testlist_exit);


