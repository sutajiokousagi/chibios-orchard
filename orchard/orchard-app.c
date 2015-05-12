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

static struct orchard_app_instance {
  const OrchardApp      *app;
  const OrchardApp      *next_app;
  OrchardAppContext     *context;
  thread_t              *thr;
  uint32_t              keymask;
  virtual_timer_t       timer;
  uint32_t              timer_usecs;
  bool                  timer_repeating;
} instance;

event_source_t orchard_app_terminated;
event_source_t orchard_app_terminate;
event_source_t timer_expired;

static void keypress(eventid_t id) {

  (void)id;
  int val = captouchRead();
  int i;
  OrchardAppEvent evt;

  if (!instance.app->event)
    return;

  for (i = 0; i < 16; i++) {
    if ((val & (1 << i)) && !(instance.keymask & (1 << i))) {
      evt.type = keyEvent;
      evt.key.code = i;
      evt.key.flags = keyDown;
      instance.app->event(instance.context, &evt);
    }
    if (!(val & (1 << i)) && (instance.keymask & (1 << i))) {
      evt.type = keyEvent;
      evt.key.code = i;
      evt.key.flags = keyUp;
      instance.app->event(instance.context, &evt);
    }
  }
  instance.keymask = val;
}

static void terminate(eventid_t id) {

  (void)id;
  OrchardAppEvent evt;

  if (!instance.app->event)
    return;

  evt.type = appEvent;
  evt.app.event = appTerminate;
  instance.app->event(instance.context, &evt);
  chThdTerminate(instance.thr);
}

static void timer_event(eventid_t id) {

  (void)id;
  OrchardAppEvent evt;

  if (!instance.app->event)
    return;

  evt.type = timerEvent;
  evt.timer.usecs = instance.timer_usecs;
  instance.app->event(instance.context, &evt);

  if (instance.timer_repeating)
    orchardAppTimer(instance.context, instance.timer_usecs, true);
}

static void timer_do_send_message(void *arg) {

  (void)arg;
  chSysLockFromISR();
  chEvtBroadcastI(&timer_expired);
  chSysUnlockFromISR();
}

void orchardAppRun(const OrchardApp *app) {
  instance.next_app = app;
  chThdTerminate(instance.thr);
  chEvtBroadcast(&orchard_app_terminate);
}

void orchardAppTimer(const OrchardAppContext *context,
                     uint32_t usecs,
                     bool repeating) {

  if (!usecs) {
    chVTReset(&context->instance->timer);
    context->instance->timer_usecs = 0;
    return;
  }

  context->instance->timer_usecs = usecs;
  context->instance->timer_repeating = repeating;
  chVTSet(&context->instance->timer, US2ST(usecs), timer_do_send_message, NULL);
}

static THD_WORKING_AREA(waOrchardAppThread, 0x800);
static THD_FUNCTION(orchard_app_thread, arg) {

  (void)arg;
  struct orchard_app_instance *instance = arg;
  struct evt_table orchard_app_events;
  OrchardAppContext app_context;

  memset(&app_context, 0, sizeof(app_context));
  instance->context = &app_context;
  app_context.instance = instance;

  chRegSetThreadName("Orchard App");

  instance->keymask = captouchRead();

  evtTableInit(orchard_app_events, 32);
  evtTableHook(orchard_app_events, captouch_changed, keypress);
  evtTableHook(orchard_app_events, orchard_app_terminate, terminate);
  evtTableHook(orchard_app_events, timer_expired, timer_event);

  if (instance->app->init)
    app_context.priv_size = instance->app->init(&app_context);
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

  if (instance->app->start)
    instance->app->start(&app_context);
  if (instance->app->event) {
    {
      OrchardAppEvent evt;
      evt.type = appEvent;
      evt.app.event = appStart;
      instance->app->event(instance->context, &evt);
    }
    while (!chThdShouldTerminateX())
      chEvtDispatch(evtHandlers(orchard_app_events), chEvtWaitOne(ALL_EVENTS));
  }

  chVTReset(&instance->timer);

  if (instance->app->exit)
    instance->app->exit(&app_context);

  instance->context = NULL;

  /* Set up the next app to run when the orchard_app_terminated message is
     acted upon.*/
  if (instance->next_app)
    instance->app = instance->next_app;
  else
    instance->app = orchard_app_list;
  instance->next_app = NULL;

  evtTableUnhook(orchard_app_events, timer_expired, timer_event);
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
  instance.app = orchard_app_list;
  chEvtObjectInit(&orchard_app_terminated);
  chEvtObjectInit(&orchard_app_terminate);
  chEvtObjectInit(&timer_expired);
  chVTReset(&instance.timer);
}

void orchardAppRestart(void) {

  /* Recovers memory of the previous application. */
  if (instance.thr) {
    osalDbgAssert(chThdTerminatedX(instance.thr), "App thread still running");
    chThdRelease(instance.thr);
    instance.thr = NULL;
  }

  instance.thr = chThdCreateStatic(waOrchardAppThread,
                                   sizeof(waOrchardAppThread),
                                   ORCHARD_APP_PRIO,
                                   orchard_app_thread,
                                   (void *)&instance);
}
