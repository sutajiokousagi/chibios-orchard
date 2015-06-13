#ifndef __ORCHARD_EVENTS__
#define __ORCHARD_EVENTS__

/* Orchard event wrappers.
   These simplify the ChibiOS eventing system.  To use, initialize the event
   table, hook an event, and then dispatch events.

  static void shell_termination_handler(eventid_t id) {
    chprintf(stream, "Shell exited.  Received id %d\r\n", id);
  }

  void main(int argc, char **argv) {
    struct evt_table events;

    // Support up to 8 events
    evtTableInit(events, 8);

    // Call shell_termination_handler() when 'shell_terminated' is emitted
    evtTableHook(events, shell_terminated, shell_termination_handler);

    // Dispatch all events
    while (TRUE)
      chEvtDispatch(evtHandlers(events), chEvtWaitOne(ALL_EVENTS));
   }
 */

struct evt_table {
  int size;
  int next;
  evhandler_t *handlers;
  event_listener_t *listeners;
};

#define evtTableInit(table, capacity)                                       \
  do {                                                                      \
    static evhandler_t handlers[capacity];                                  \
    static event_listener_t listeners[capacity];                            \
    table.size = capacity;                                                  \
    table.next = 0;                                                         \
    table.handlers = handlers;                                              \
    table.listeners = listeners;                                            \
  } while(0)

#define evtTableHook(table, event, callback)                                \
  do {                                                                      \
    if (CH_DBG_ENABLE_ASSERTS != FALSE)                                     \
      if (table.next >= table.size)                                         \
        chSysHalt("event table overflow");                                  \
    chEvtRegister(&event, &table.listeners[table.next], table.next);        \
    table.handlers[table.next] = callback;                                  \
    table.next++;                                                           \
  } while(0)

#define evtTableUnhook(table, event, callback)                              \
  do {                                                                      \
    int i;                                                                  \
    for (i = 0; i < table.next; i++) {                                      \
      if (table.handlers[i] == callback)                                    \
        chEvtUnregister(&event, &table.listeners[i]);                       \
    }                                                                       \
  } while(0)

#define evtHandlers(table)                                                  \
    table.handlers

#define evtListeners(table)                                                 \
    table.listeners

extern event_source_t ble_rdy;
extern event_source_t rf_pkt_rdy;
extern event_source_t gpiox_rdy;

// adc-related events
extern event_source_t celcius_rdy;
extern event_source_t mic_rdy;
extern event_source_t usbdet_rdy;

// BM radio protocol events
extern event_source_t radio_page;
extern event_source_t radio_sex;
extern event_source_t radio_app;

void orchardEventsStart(void);

/// Orchard App events

typedef enum _OrchardAppEventType {
  keyEvent,
  appEvent,
  timerEvent,
  uiEvent,
  adcEvent,
  radioEvent,
} OrchardAppEventType;

/* ------- */

typedef enum _OrchardAppEventKeyFlag {
  keyUp = 0,
  keyDown = 1,
} OrchardAppEventKeyFlag;

typedef enum _OrchardAppEventKeyCode {
  keyLeft = 0x80,
  keyRight = 0x81,
  keySelect = 0x82,
  keyCW = 0x83,
  keyCCW = 0x84,
} OrchardAppEventKeyCode;

typedef struct _OrchardUiEvent {
  uint8_t   code;
  uint8_t   flags;
} OrchardUiEvent;

typedef enum _OrchardUiEventCode {
  uiComplete = 0x01,
} OrchardUiEventCode;

typedef enum _OrchardUiEventFlags {
  uiOK = 0x01,
  uiCancel,
  uiError,
} OrchardUiEventFlags;

typedef struct _OrchardAdcEvent {
  uint8_t   code;
  uint8_t   flags;
} OrchardAdcEvent;

typedef enum _OrchardAdcEventCode {
  adcCodeTemp = 0x01,
  adcCodeMic,
  adcCodeUsbdet,
} OrchardAdcEventCode;

// note: no ADC flags yet

typedef struct _OrchardAppKeyEvent {
  uint8_t   code;
  uint8_t   flags;
} OrchardAppKeyEvent;

/* ------- */

typedef enum _OrchardAppLifeEventFlag {
  appStart,
  appTerminate,
} OrchardAppLifeEventFlag;

typedef struct _OrchardAppLifeEvent {
  uint8_t   event;
} OrchardAppLifeEvent;

/* ------- */

typedef struct _OrchardAppTimerEvent {
  uint32_t  usecs;
} OrchardAppTimerEvent;

/* ------- */

typedef struct _OrchardAppEvent {
  OrchardAppEventType     type;
  union {
    OrchardAppKeyEvent    key;
    OrchardAppLifeEvent   app;
    OrchardAppTimerEvent  timer;
    OrchardUiEvent        ui;
    OrchardAdcEvent       adc;
  };
} OrchardAppEvent;

#endif /* __ORCHARD_EVENTS__ */
