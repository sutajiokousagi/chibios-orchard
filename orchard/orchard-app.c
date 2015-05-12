#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "orchard.h"
#include "orchard-app.h"
#include "orchard-events.h"
#include "captouch.h"

orchard_app_end();

static const OrchardApp *orchard_app_list;
static const OrchardApp *current_app;
static const OrchardApp *next_app;
static OrchardAppContext *current_app_context;
static thread_t *app_tp = NULL;

event_source_t orchard_app_terminated;
event_source_t orchard_app_terminate;

static void keypress(eventid_t id) {

  (void)id;
  int val = captouchRead();
  int i;
  OrchardAppEvent evt;

  if (!current_app->event)
    return;

  for (i = 0; i < 16; i++) {
    if ((val & (1 << i)) && !(current_app_context->keymask & (1 << i))) {
      evt.type = keyEvent;
      evt.key.code = i;
      evt.key.flags = keyDown;
      current_app->event(current_app_context, &evt);
    }
    if (!(val & (1 << i)) && (current_app_context->keymask & (1 << i))) {
      evt.type = keyEvent;
      evt.key.code = i;
      evt.key.flags = keyUp;
      current_app->event(current_app_context, &evt);
    }
  }
  current_app_context->keymask = val;
}

static void terminate(eventid_t id) {

  (void)id;
  OrchardAppEvent evt;

  if (!current_app->event)
    return;

  evt.type = appEvent;
  evt.app.event = appTerminate;
  current_app->event(current_app_context, &evt);
  chThdTerminate(app_tp);
}

void orchardAppRun(const OrchardApp *app) {
  next_app = app;
  chThdTerminate(app_tp);
  chEvtBroadcast(&orchard_app_terminate);
}

static THD_WORKING_AREA(waOrchardAppThread, 0x800);
static THD_FUNCTION(orchard_app_thread, arg) {

  (void)arg;
  const OrchardApp *current = arg;
  struct evt_table orchard_app_events;
  OrchardAppContext app_context;

  memset(&app_context, 0, sizeof(app_context));
  current_app_context = &app_context;

  chRegSetThreadName("Orchard App");

  app_context.keymask = captouchRead();

  evtTableInit(orchard_app_events, 32);
  evtTableHook(orchard_app_events, captouch_changed, keypress);
  evtTableHook(orchard_app_events, orchard_app_terminate, terminate);

  if (current->init)
    app_context.priv_size = current->init(&app_context);
  else
    app_context.priv_size = 0;

  /* Allocate private data on the stack (word-aligned) */
  uint32_t priv_data[app_context.priv_size / 4];
  if (app_context.priv_size) {
    memset(priv_data, 0, sizeof(priv_data));
    app_context.priv = priv_data;
  }
  else
    app_context.priv = NULL;

  if (current->start)
    current->start(&app_context);
  if (current->event) {
    {
      OrchardAppEvent evt;
      evt.type = appEvent;
      evt.app.event = appStart;
      current_app->event(current_app_context, &evt);
    }
    while (!chThdShouldTerminateX())
      chEvtDispatch(evtHandlers(orchard_app_events), chEvtWaitOne(ALL_EVENTS));
  }
  if (current->exit)
    current->exit(&app_context);

  current_app_context = NULL;

  /* Set up the next app to run when the orchard_app_terminated message is
     acted upon.*/
  if (next_app)
    current_app = next_app;

  evtTableUnhook(orchard_app_events, orchard_app_terminate, terminate);
  evtTableUnhook(orchard_app_events, captouch_changed, keypress);

  /* Atomically broadcasting the event source and terminating the thread,
     there is not a chSysUnlock() because the thread terminates upon return.*/
  chSysLock();
  chEvtBroadcastI(&orchard_app_terminated);
  chThdExitS(MSG_OK);
}

void orchardAppInit(void) {

  orchard_app_list = orchard_app_start();
  current_app = orchard_app_list;
  chEvtObjectInit(&orchard_app_terminated);
  chEvtObjectInit(&orchard_app_terminate);
}

void orchardAppRestart(void) {

  /* Recovers memory of the previous application. */
  if (app_tp) {
    osalDbgAssert(chThdTerminatedX(app_tp), "App thread still running");
    chThdRelease(app_tp);
    app_tp = NULL;
  }

  app_tp = chThdCreateStatic(waOrchardAppThread, sizeof(waOrchardAppThread),
                    LOWPRIO + 2, orchard_app_thread, (void *)current_app);
}
