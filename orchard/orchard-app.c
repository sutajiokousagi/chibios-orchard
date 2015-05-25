#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

typedef enum _DirIntent {
  dirNone = 0x0,
  dirCW = 0x1,
  dirCCW = 0x2,
} DirIntent;

static struct jogdial_state {
  int8_t lastpos;
  DirIntent direction_intent;
  uint32_t lasttime;
} jogdial_state;
#define DWELL_THRESH  500  // time to spend in one state before direction intent is null

event_source_t orchard_app_terminated;
event_source_t orchard_app_terminate;
event_source_t timer_expired;

static virtual_timer_t keycollect_timer;
static event_source_t keycollect_timeout;
static uint16_t  captouch_collected_state = 0;

#define DEBOUNCE_INTERVAL 20  // time to debounce key press in ms
#define TRACK_INTERVAL 1  // trackpad debounce in ms
static unsigned long track_time;

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

// hex codes from top, going clockwise
// 80 40 10 08 04 02 400 200 100 
static int8_t jog_raw_to_position(uint32_t raw) {
  switch(raw) {
  case 0x80:
    return 0;
  case (0x80 | 0x40):
    return 1;
  case 0x40:
    return 2;
  case (0x40 | 0x10):
    return 3;
  case 0x10:
    return 4;
  case (0x10 | 0x08):
    return 5;
  case 0x08:
    return 6;
  case (0x08 | 0x04):
    return 7;
  case 0x04:
    return 8;
  case (0x04 | 0x02):
    return 9;
  case 0x02:
    return 10;
  case (0x02 | 0x400):
    return 11;
  case 0x400:
    return 12;
  case (0x400 | 0x200):
    return 13;
  case 0x200:
    return 14;
  case (0x200 | 0x100):
    return 15;
  case 0x100:
    return 16;
  case (0x100 | 0x80):
    return 17;
    
  default:
    return -1; // error case
  }
}

// current state + previous state => direction
// problem is if we jitter back and forth, direction intent is wrong

// so compute using:
// current state + previous state + direction intent + time => new direction intent
// direction intent can be none, CW, or CCW.

// time is used to retire direction state, if you stay in a quadrant for
// longer than a certain amount of time, direction intent should be reset to none

// returns 0 if there is no dial tracking event
static uint8_t track_dial(uint32_t raw) {
  uint32_t curtime = chVTGetSystemTime();
  int8_t curpos = jog_raw_to_position(raw);
  int8_t posdif;

  if( raw == 0 ) {
    // reset to untouched state
    jogdial_state.lastpos = -1;
    jogdial_state.lasttime = curtime;
    jogdial_state.direction_intent = dirNone;
    return 0;
  }
  
  if( curpos == -1 ) {
    // still touched, but can't make sense of it. clear direction intent,
    // set the time, but don't change from the last known position
    jogdial_state.lasttime = curtime;
    jogdial_state.direction_intent = dirNone;
    return 0;
  }

  if( jogdial_state.lastpos == -1 ) {
    // we're starting from untouched state
    jogdial_state.lastpos = curpos;
    jogdial_state.lasttime = curtime;
    jogdial_state.direction_intent = dirNone;
    return 0;
  }
  
  posdif = curpos - jogdial_state.lastpos;
  
  // check to see if we've made a significant state change from before
  if( abs(posdif) <= 1 ||
      abs(posdif) == 17 ) {
    // we haven't changed since before
    if (curtime - jogdial_state.lasttime > DWELL_THRESH)
      jogdial_state.direction_intent = dirNone;

    // don't update lastpos either, so we can't "drift" around the clock
    return 0; // no event update
    
  } else {
    // now check if we're going CW or CCW
    // (16 -> 17, 16 -> 0), (17 -> 0, 17 -> 1) are both CW
    if( ((posdif > 1) && (posdif < 14)) ||
	(posdif <= -14) ) {
      jogdial_state.direction_intent = dirCW;
      jogdial_state.lastpos = curpos;
      jogdial_state.lasttime = curtime;
      return 1;
    }

    if( (posdif < -1) ||
	(posdif >= 14) ) {
      jogdial_state.direction_intent = dirCCW;
      jogdial_state.lastpos = curpos;
      jogdial_state.lasttime = curtime;
      return 1;
    }
  }
  
  // all cases already handled with a return
  return 0;
}

static void run_keycollect_timer(void *arg) {
  (void)arg;
  
  chSysLockFromISR();
  chEvtBroadcastI(&keycollect_timeout);
  chSysUnlockFromISR();
}

static void key_event_timer(eventid_t id) {
  (void)id;
  captouch_collected_state |= captouchRead(); // accumulate events

  // set a timer to collect accumulated events...
  chVTSet(&keycollect_timer, MS2ST(DEBOUNCE_INTERVAL), run_keycollect_timer, NULL);
}

static void key_event(eventid_t id) {
  (void)id;
  uint32_t val = captouch_collected_state;
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

    if( (val & MAIN_MENU_MASK) == MAIN_MENU_VALUE ) // don't send main menu requests on to the app
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
  
  instance.keymask = val;

  // reset the collective state to 0
  captouch_collected_state = 0;
}

// handle jogdial events (in parallel to key events)
static void dial_event(eventid_t id) {
  (void)id;
  uint32_t val = captouchRead();
  unsigned int curtime;
  OrchardAppEvent evt;

  if (!instance.app->event)
    return;

  /* No key changed */
  if (instance.keymask == val)
    return;

  curtime = chVTGetSystemTime();
  // track dial above the debouncer for smooth scrolling
  if( track_dial(val) ) {
    // debounce this slightly, to avoid adding lag to the track dial responsivitiy
    if( !((curtime - track_time) < TRACK_INTERVAL) ) {
      track_time = curtime;
    
      evt.type = keyEvent;
      evt.key.flags = keyDown;
      if( jogdial_state.direction_intent == dirCW )
	evt.key.code = keyCW;
      else
	evt.key.code = keyCCW;
      
      instance.app->event(instance.context, &evt);
    }
  }
  
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
  evtTableHook(orchard_app_events, captouch_changed, key_event_timer);
  evtTableHook(orchard_app_events, captouch_changed, dial_event);
  evtTableHook(orchard_app_events, keycollect_timeout, key_event);
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
  evtTableUnhook(orchard_app_events, keycollect_timeout, key_event);
  evtTableUnhook(orchard_app_events, captouch_changed, dial_event);
  evtTableUnhook(orchard_app_events, captouch_changed, key_event_timer);

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
  chEvtObjectInit(&keycollect_timeout);
  chVTReset(&instance.timer);

  /* Hook this outside of the app-specific runloop, so it runs even if
     the app isn't listening for events.*/
  evtTableHook(orchard_events, captouch_changed, poke_run_launcher_timer);

  jogdial_state.lastpos = -1; 
  jogdial_state.direction_intent = dirNone;
  jogdial_state.lasttime = chVTGetSystemTime();
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
