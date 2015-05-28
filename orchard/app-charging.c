#include "orchard-app.h"
#include "orchard-ui.h"

#include "analog.h"
#include "gasgauge.h"

#include <string.h>
#include <stdlib.h>

static int32_t celcius;
static uint16_t usbn, usbp;
static usbStat usbStatus = usbStatNC;

static void redraw_ui(uint8_t firsttime) {
  char tmp[] = "Charger status";
  char uiStr[32];
  
  coord_t width;
  coord_t height;
  font_t font;

  // draw the title bar
  font = gdispOpenFont("fixed_5x8");
  width = gdispGetWidth();
  height = gdispGetFontMetric(font, fontHeight);

  gdispClear(Black);
  gdispFillArea(0, 0, width, height, White);
  gdispDrawStringBox(0, 0, width, height,
                     tmp, font, Black, justifyCenter);

  // 1st line: CPU temp
  if( firsttime ) {
    chsnprintf(uiStr, sizeof(uiStr), "CPU Temp: measuring...");
  } else {
    chsnprintf(uiStr, sizeof(uiStr), "CPU Temp: %d.%dC", celcius / 1000, 
	       abs((celcius % 1000) / 100));
  }
  gdispDrawStringBox(0, height, width, height,
		     uiStr, font, White, justifyLeft);

  // 2nd line left: USB-
  if( firsttime ) {
    chsnprintf(uiStr, sizeof(uiStr), "USB-: measuring...");
  } else {
    chsnprintf(uiStr, sizeof(uiStr), "USB-: 0x%04x", usbn);
  }
  gdispDrawStringBox(0, height*2, width, height,
		     uiStr, font, White, justifyLeft);

  // 3rd line left: USB+
  if( firsttime ) {
    chsnprintf(uiStr, sizeof(uiStr), "USB+: measuring...");
  } else {
    chsnprintf(uiStr, sizeof(uiStr), "USB+: 0x%04x", usbp);
  }
  gdispDrawStringBox(0, height*3, width, height,
		     uiStr, font, White, justifyLeft);


  if( !firsttime ) {
    // 2nd line right
    chsnprintf(uiStr, sizeof(uiStr), "Detected:");
    gdispDrawStringBox(width / 2, height*2, width / 2, height,
		       uiStr, font, White, justifyRight);

    // 3rd line right
    switch(usbStatus) {
    case usbStatNC:
      chsnprintf(uiStr, sizeof(uiStr), "Unplugged");
      break;
    case usbStat500:
      chsnprintf(uiStr, sizeof(uiStr), "USB Host");
      break;
    case usbStat1500:
      chsnprintf(uiStr, sizeof(uiStr), "1.5A chgr");
      break;
    default:
      chsnprintf(uiStr, sizeof(uiStr), "Error");
    }
    gdispDrawStringBox(width / 2, height*3, width / 2, height,
		       uiStr, font, White, justifyRight);

  }
  
  // 4th line left
  chsnprintf(uiStr, sizeof(uiStr), "Draw: %dmA", ggAvgCurrent());
  gdispDrawStringBox(0, height*4, width, height,
		     uiStr, font, White, justifyLeft);

  // 4th line right
  chsnprintf(uiStr, sizeof(uiStr), "Volt: %dmV", ggVoltage());
  gdispDrawStringBox(width/2, height*4, width/2, height,
		     uiStr, font, White, justifyRight);

  // 5th line left
  chsnprintf(uiStr, sizeof(uiStr), "Pwr: %dmW", ggAvgPower());
  gdispDrawStringBox(0, height*5, width, height,
		     uiStr, font, White, justifyLeft);

  // 5th line right
  chsnprintf(uiStr, sizeof(uiStr), "Tank: %dmAh", ggRemainingCapacity());
  gdispDrawStringBox(width/2, height*5, width/2, height,
		     uiStr, font, White, justifyRight);

  // 6th line left
  chsnprintf(uiStr, sizeof(uiStr), "Capacity: %d%%", ggStateofCharge());
  gdispDrawStringBox(0, height*6, width, height,
		     uiStr, font, White, justifyLeft);

  // 6th line right
  //  chsnprintf(uiStr, sizeof(uiStr), "Cap: %d\%", ggRemainingCapacity());
  //  gdispDrawStringBox(width/2, height*6, width/2, height,
  //		     uiStr, font, White, justifyRight);

  gdispFlush();
}

static uint32_t charging_init(OrchardAppContext *context) {

  (void)context;

  return 0;
}

static void charging_start(OrchardAppContext *context) {
  
  analogUpdateTemperature(); 
  analogUpdateUsbStatus(); 
  orchardAppTimer(context, 500 * 1000 * 1000, true); // update UI every 500 ms
  
  redraw_ui(1);
  // all this app does is launch a text entry box and store the name
}

void charging_event(OrchardAppContext *context, const OrchardAppEvent *event) {

  (void)context;
  adcsample_t *buf;
  
  if (event->type == keyEvent) {
    if ( (event->key.flags == keyDown) && (event->key.code == keyLeft) ) {
      orchardAppExit();
    }
  } else if (event->type == timerEvent) {
    analogUpdateTemperature(); // the actual value won't update probably until the UI is redrawn...
    analogUpdateUsbStatus(); 
    redraw_ui(0);
  } else if( event->type == adcEvent) {
    if( event->adc.code == adcCodeTemp ) {
      celcius = analogReadTemperature();
    } else if( event->adc.code == adcCodeUsbdet ) {
      buf = analogReadUsbRaw();
      usbn = (uint16_t) buf[0];
      usbp = (uint16_t) buf[1];
      
      usbStatus = analogReadUsbStatus();
    }
  }
}

static void charging_exit(OrchardAppContext *context) {

  (void)context;
}

orchard_app("Charging settings", charging_init, charging_start, charging_event, charging_exit);


