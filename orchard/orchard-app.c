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
static virtual_timer_t run_launcher_timer;
static bool run_launcher_timer_engaged;
#define RUN_LAUNCHER_TIMEOUT MS2ST(500)

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

#define MAIN_MENU_MASK  ((1 << 11) | (1 << 0))
#define MAIN_MENU_VALUE ((1 << 11) | (1 << 0))

static int captouch_to_key(uint8_t code) {
  if (code == 11)
    return keyLeft;
  if (code == 0)
    return keyRight;
  if (code == 5)
    return keySelect;
  return code;
}

static void run_launcher(void *arg) {

  (void)arg;

  chSysLockFromISR();
  /* Launcher is the first app in the list */
  instance.next_app = orchard_app_list;
  instance.thr->p_flags |= CH_FLAG_TERMINATE;
  chEvtBroadcastI(&orchard_app_terminate);
  run_launcher_timer_engaged = false;
  chSysUnlockFromISR();
}

static void poke_run_launcher_timer(eventid_t id) {

  (void)id;

  uint32_t val = captouchRead();
  if (run_launcher_timer_engaged) {
    /* Timer is engaged, but both buttons are still held.  Do nothing.*/
    if ((val & MAIN_MENU_MASK) == MAIN_MENU_VALUE)
      return;

    /* One or more buttons was released.  Cancel the timer.*/
    chVTReset(&run_launcher_timer);
    run_launcher_timer_engaged = false;
  }
  else {
    /* Timer not engaged, and the magic sequene isn't held.  Do nothing.*/
    if ((val & MAIN_MENU_MASK) != MAIN_MENU_VALUE)
      return;

    /* Start the sequence to go to the main menu when we're done.*/
    run_launcher_timer_engaged = true;
    chVTSet(&run_launcher_timer, RUN_LAUNCHER_TIMEOUT, run_launcher, NULL);
  }
}

static void key_event(eventid_t id) {

  (void)id;
  uint32_t val = captouchRead();
  uint32_t i;
  OrchardAppEvent evt;

  if (!instance.app->event)
    return;

  /* No key changed */
  if (instance.keymask == val)
    return;

  for (i = 0; i < 16; i++) {
    uint8_t code = captouch_to_key(i);

    /* Code is a wheel event */
    if (code < 0x80)
      continue;

    if ((val & (1 << i)) && !(instance.keymask & (1 << i))) {
      evt.type = keyEvent;
      evt.key.code = code;
      evt.key.flags = keyDown;
      instance.app->event(instance.context, &evt);
    }
    if (!(val & (1 << i)) && (instance.keymask & (1 << i))) {
      evt.type = keyEvent;
      evt.key.code = code;
      evt.key.flags = keyUp;
      instance.app->event(instance.context, &evt);
    }
  }
  {
#define only_one_bit_set(x) (x && !(x & (x - 1)))
  /* Super cheesy spin detection */
    uint32_t this_spin = (val & ~((1 << 0) | (1 << 11) | (1 << 5)));
    uint32_t last_spin = (instance.keymask & ~((1 << 0) | (1 << 11) | (1 << 5)));
    if (only_one_bit_set(this_spin) && only_one_bit_set(last_spin)) {
      evt.type = keyEvent;
      evt.key.flags = keyDown;
      if (this_spin < last_spin) {
        if (!this_spin)
          evt.key.code = keyCCW;
        else
          evt.key.code = keyCW;
      }
      else {
        if (!this_spin)
          evt.key.code = keyCW;
        else
          evt.key.code = keyCCW;
      }
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
  evtTableHook(orchard_app_events, captouch_changed, key_event);
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

  chVTReset(&run_launcher_timer);
  run_launcher_timer_engaged = false;

  evtTableUnhook(orchard_app_events, timer_expired, timer_event);
  evtTableUnhook(orchard_app_events, orchard_app_terminate, terminate);
  evtTableUnhook(orchard_app_events, captouch_changed, key_event);

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

  /* Hook this outside of the app-specific runloop, so it runs even if
     the app isn't listening for events.*/
  evtTableHook(orchard_events, captouch_changed, poke_run_launcher_timer);
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
