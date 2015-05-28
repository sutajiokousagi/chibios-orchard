Orchard App Design
==================

Orchard supports the concept of "Apps".  Each app is provided a low-priority
thread and a set amount of stack (around 2kb).  It can be fed events using
an event loop similar to how SDL or Windows main loops behave.

Apps cannot be killed, and must exit gracefully when asked to do so.  Only
one app may run at a time.


Synposis
--------

 #include "orchard-app.h"

 static void example(OrchardAppContext *context) {
   (void)context;
   int i = 0;
   while (!chThdShouldTerminateX()) {
     chprintf(stream, "i: %d\r\n", i++);
   }
   return;
 }
 orchard_app("example", NULL, example, NULL, NULL);


API
----

The Orchard App API consists of four functions plus a name.  All functions
are optional.

The init function indicates how much stack space you want, and completes
any early initialization you may require.  It returns the number of bytes
you want the "priv" structure to be.  An example usage might be:

 static uint32_t init(OrchardAppContext *context) {
   return sizeof(struct example_data);
 }

The start function is run once at the start of the app, and for some
situations may be the only function that you run.  The context->priv
structure will contain an allocated pointer to an area of memory that's
guaranteed to be the size of the data requested by your init() function.

 static void start(OrchardAppContext *context) {
   struct example_data *example = context->priv;
   chprintf(stream, "Example pointer located at %p\r\n", example);
 }

The event function is called whenever a system event is delivered.  Events
include keypresses, lifetime indicators, UI elements, and timers.  See
the file orchard-events.h for a list of supported event types.

  static void event(OrchardAppContext *context, const OrchardAppEvent *event) {
    return;
  }

The exit function is called after the final event has been delivered, and
just before the app thread terminates.  Use it to free up any memory you
may have allocated from the global heap.

  static void exit(OrchardAppContext *context) {
    return;
  }


Examples
--------

There are several good example apps, depending on what you want to do.


Example: Continuously-Running "Mandelbrot"
------------------------------------------

The "Mandelbrot" app (app-mandelbrot.c) stays in its start() function
until directed to exit.  When the "exit" flag is set, it returns right
away.


Example: Interactive "Pew"
--------------------------

The "Pew" app (app-pew.c) is a partial implementation of "Asteroids".  It
uses the event loop to read key presses, and uses an Orchard timer for the
redraw loop.


Example: Event "test"
---------------------

The "Test" app (app-test.c) is useful for testing the app infrastructure
and possible evnt codes.  It prints its data to the serial port, so it
is not very exciting to use with the OLED.
