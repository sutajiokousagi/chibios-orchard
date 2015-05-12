#include <stdio.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "orchard.h"
#include "orchard-app.h"

struct launcher_list_item {
  const char        *name;
  const OrchardApp  *entry;
};

struct launcher_list {
  unsigned int selected;
  unsigned int total;
  struct launcher_list_item items[0];
};

static void redraw_list(struct launcher_list *list) {

  char tmp[20];
  unsigned int i;

  chsnprintf(tmp, sizeof(tmp), "%d apps", list->total);

  coord_t width;
  coord_t height;
  coord_t header_height;
  font_t font;

  font = gdispOpenFont("UI2");
  width = gdispGetWidth();
  height = gdispGetFontMetric(font, fontHeight);
  header_height = height;

  gdispClear(Black);
  gdispFillArea(0, 0, width, height, White);
  gdispDrawStringBox(0, 0, width, height,
                     tmp, font, Black, justifyCenter);

  font = gdispOpenFont("fixed_5x8");
  width = gdispGetWidth();
  height = gdispGetFontMetric(font, fontHeight);

  for (i = 0; i < list->total; i++) {
    color_t draw_color = White;
    if (i == list->selected) {
      gdispFillArea(0, header_height + i * height, width, height, White);
      draw_color = Black;
    }
    gdispDrawStringBox(0, header_height + i * height,
                       width, height,
                       list->items[i].name, font, draw_color, justifyCenter);
  }
  gdispFlush();
}

static uint32_t launcher_init(OrchardAppContext *context) {

  (void)context;
  unsigned int total_apps = 0;
  const OrchardApp *current;

  /* Rebuild the app list */
  current = orchard_app_start();
  while (current->name) {
    total_apps++;
    current++;
  }

  return sizeof(struct launcher_list)
       + (total_apps * sizeof(struct launcher_list_item));
}

static void launcher_start(OrchardAppContext *context) {

  struct launcher_list *list = (struct launcher_list *)context->priv;
  const OrchardApp *current;

  /* Rebuild the app list */
  current = orchard_app_start();
  list->total = 0;
  while (current->name) {
    list->items[list->total].name = current->name;
    list->items[list->total].entry = current;
    list->total++;
    current++;
  }

  list->selected = 2;

  redraw_list(list);
}

void launcher_event(OrchardAppContext *context, const OrchardAppEvent *event) {

  struct launcher_list *list = (struct launcher_list *)context->priv;

  if (event->type == keyEvent) {
    if ((event->key.flags == keyDown) && (event->key.code == keyCW)) {
      list->selected++;
      if (list->selected >= list->total)
        list->selected = 0;
    }
    else if ((event->key.flags == keyDown) && (event->key.code == keyCCW)) {
      list->selected--;
      if (list->selected >= list->total)
        list->selected = list->total - 1;
    }
    else if ((event->key.flags == keyDown) && (event->key.code == keySelect)) {
      orchardAppRun(list->items[list->selected].entry);
    }
    redraw_list(list);
  }
}

static void launcher_exit(OrchardAppContext *context) {

  (void)context;
}

const OrchardApp _orchard_app_list_launcher
__attribute__((unused, aligned(4), section(".chibi_list_app_1_launcher"))) = {
  "Launcher", 
  launcher_init,
  launcher_start,
  launcher_event,
  launcher_exit,
};
