UI Design document for Orchard
==============================

The input elements for Orchard include:

- Captouch jog dial pad with center button plus L/R auxilliary buttons
- Accelerometer
- Microphone
- Radio

Desired functionality:

- Cycle between multiple device modes
- Allow background task of light cycling at all times
- Enable interactive play with lights
- Passive radio pings for general awareness of peers
- Active pages for alerting peers of one's presence

Key UI Features
===============

Status Bar
----------
- Reports battery level, elapsed time, and current app mode
(note time is reported simply as elapsed due to the cultural
requirement at burning man to not wear a watch)

struct status_bar {
   uint8_t batteryPercent;
   uint32_t timeInSec;
   appDesc *currentApp;
};

Default UI
----------
In default mode, the jog dial remains active for interactive LED light
play. The OLED indicates the current light pattern, and lists nearby
radio peers in a matrix that tries to sort by signal strength and last
ping time. The center button sends a page.

Tapping the L/R buttons cycle through the light patterns.

Pressing and holding the L/R buttons simultaneously for one second enters
"top level menu".


Page Mode
---------

There is a "page" mode. When paged, the OLED will temporarily display
the name of the person sending the page. Pages are all-to-all, and
pages by default will override what is on the screen, but the behavior
can be disabled.


Top Level
---------
- The "top level" UI is a picker between "apps"
- Jog dial scrolls through app list
- Hitting the center button launches app-specific UI
- Pressing and holding L/R buttons for 1 second brings you back to default UI
- Idling in top level for one minute brings you back to default UI
- Double-tapping L/R buttons will adjust overall LED brightness (...maybe...)


App Behavior
------------
Each app can have its own UI behavior, but generally In "app" mode:
- Jog dial is used to scroll up and down item lists
- Center button is used to pick a list item
- Left button brings you back to the UI picker

Apps register themselves with the top-level module via a
orchard_app() call similar to orchard_command(). The app function
is an event handler based upon the UI input state. There is also
an init_app() callback and an uninit_app() callback provided.


Events
------
Captouch events go to a tier-1 event processor, which tracks the
last captouch state and extracts the following meta-events: clockwise,
counter-clockwise, enter, left, and right.

The meta-events go to a handler that consults the current active app and
routes the event to the corresponding app handler.


Sample "apps" for the system include:

- Light brightness and pattern picking
- Battery status and settings
  - force charging, charge rate programming
- Battery reporting method
  - time remaining, percentage capacity, amps/volts
- Shutdown mode -- turns system off until plugged into a USB pack
- Settings -- disable paging display, set auto-dim at low battery, etc.
- Lock mode
- Set your name -- alphabetic list + jog dial to enter your name
- Black list -- (un)block pages from IDs that match a particular string.
  Select from list of recent pages received.
- View neighbors -- scroll rapidly through all pings received, along
  with elapsed time since seen and signal strength (if available)
- Set radio parameters -- ping interval, Tx strength?
- Reset elapsed time


Radio Protocol
--------------
- All users are on the same radio PHY address (all-to-all comms)
- Every (20 + 15*(rand(0-1))) seconds, broadcast a ping packet. Packet
  contains user ID, which is a 16-byte string, plus one flag byte.
  Flag byte currently just uses one bit, which IDs packet as ping or page
- Pages consist of an immediate ping packet send with page bit set in flags.
  Pages packets are broadcast once every 100ms for up to 5 seconds of holding
  the paging button.

