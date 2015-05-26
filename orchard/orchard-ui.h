#ifndef __ORCHARD_UI_H__
#define __ORCHARD_UI_H__

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "shell.h"
#include "orchard.h"
#include "gfx.h"

// used to define lists of items for passing to UI elements
// for selection lists, the definition is straightfoward
// for dialog boxes, the first list item is the message; the next 1 or 2 items are button texts
// lists are linked lists, as we don't know a priori how long the lists will be
typedef struct UiListItem {
  char        *name;
  struct UiListItem *next;
} UiListItem;

typedef struct UiItemList {
  unsigned int selected;
  unsigned int total;
  struct UiListItem *items;
} UiItemList;

typedef struct OrchardUiContext {
  struct orchard_app_instance *instance;  // instance of the initiating app
  struct UiItemList *itemlist;
} OrchardUiContext;

typedef struct _OrchardUi {
  uint32_t (*init)(OrchardUiContext *ui);
  void (*start)(OrchardUiContext *ui);
  void (*event)(OrchardUiContext *ui, const OrchardAppEvent *event);
  void (*exit)(OrchardUiContext *ui);
} OrchardUi;

#define orchard_ui_start()                                                   \
({                                                                            \
  static char start[0] __attribute__((unused,                                 \
    aligned(4), section(".chibi_list_ui_1")));                               \
  (const OrchardUi *)&start;                                                 \
})

#define orchard_ui(_name, _init, _start, _event, _exit)                        \
  const OrchardUi _orchard_ui_list_##_init##_start##_event##_exit           \
  __attribute__((unused, aligned(4), section(".chibi_list_ui_2_" # _init # _start # _event # _exit))) =  \
     { _name, _init, _start, _event, _exit }

#define orchard_ui_end()                                                     \
  const OrchardUi _orchard_ui_list_final                                    \
  __attribute__((unused, aligned(4), section(".chibi_list_ui_3_end"))) =     \
     { NULL, NULL, NULL, NULL, NULL }


#endif /* __ORCHARD_UI_H__ */
