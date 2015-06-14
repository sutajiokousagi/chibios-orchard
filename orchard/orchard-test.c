#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "orchard.h"
#include "gfx.h"
#include "orchard-ui.h"
#include "captouch.h"

#include "orchard-test.h"
#include "test-audit.h"

orchard_test_end();

void orchardTestInit(void) {
  auditStart();  // start / initialize the test audit log
}

void orchardListTests(BaseSequentialStream *chp) {
  const TestRoutine *current;

  current = orchard_test_start();
  chprintf(chp, "Available tests:\n\r" );
  while(current->test_name) {
    chprintf(chp, "%s\r\n", current->test_name);
    current++;
  }
}

const TestRoutine *orchardGetTestByName(const char *name) {
  const TestRoutine *current;

  current = orchard_test_start();
  while(current->test_name) {
    if( !strncmp(name, current->test_name, 16) ) {
      return current;
    }
    current++;
  }
  return NULL;
}


void orchardTestRun(BaseSequentialStream *chp, OrchardTestType test_type) {
  const TestRoutine *cur_test;
  OrchardTestResult test_result;
  
  cur_test = orchard_test_start();
  while(cur_test->test_function) {
    test_result = cur_test->test_function(cur_test->test_name, test_type);
    if( test_type != orchardTestPoweron ) {
      auditUpdate(cur_test->test_name, test_type, test_result);
    } else {
      switch(test_result) {
      case orchardResultPass:
	break;
      case orchardResultFail:
	chprintf(chp, "TEST: %s subystem failed test with code %d\n\r",
           cur_test->test_name, test_result);
	break;
      case orchardResultNoTest:
	chprintf(chp, "TEST: reminder: write test for subystem %s\n\r",
           cur_test->test_name );
	break;
      case orchardResultUnsure:
	chprintf(stream, "TEST: %s subystem not testable with test type %d\n\r",
           cur_test->test_name, test_result, test_type);
	break;
      default:
	// lolwut?
	break;
      }
    }
    cur_test++;
  }
}

// used to draw UI prompts for tests to the screen
// print up to 2 lines of text
// interaction_delay specifies how long we should wait before we declare failure
//   0 means don't delay
OrchardTestResult orchardTestPrompt(char *line1, char *line2, uint8_t interaction_delay) {
  coord_t width;
  coord_t height;
  font_t font;
  uint32_t val;
  char timer[16];
  uint32_t starttime;
  uint32_t curtime, updatetime;
  OrchardTestResult result = orchardResultUnsure;
  uint8_t countdown;

  val = captouchRead();
  countdown = interaction_delay;
  
  orchardGfxStart();
  font = gdispOpenFont("ui2");
  width = gdispGetWidth();
  height = gdispGetFontMetric(font, fontHeight);
  
  gdispClear(Black);

  gdispDrawStringBox(0, height * 2, width, height,
                     line1, font, White, justifyCenter);
  
  gdispDrawStringBox(0, height * 3, width, height,
                     line2, font, White, justifyCenter);
  
  if( interaction_delay != 0 ) {
    chsnprintf(timer, sizeof(timer), "%d", countdown);
    gdispDrawStringBox(0, height * 4, width, height,
		       timer, font, White, justifyCenter);
    countdown--;
  }
  
  gdispFlush();
  
  starttime = chVTGetSystemTime();
  updatetime = starttime + 1000;
  if( interaction_delay != 0 ) {
    while(1) {
      curtime = chVTGetSystemTime();
      if( (val != captouchRead()) ) {
	result = orchardResultPass;
	break;
      }
      if( (curtime - starttime) > ((uint32_t) interaction_delay * 1000) ) {
	result = orchardResultFail;
	break;
      }

      if( curtime > updatetime ) {
	chsnprintf(timer, sizeof(timer), "%d", countdown);
	gdispFillArea(0, height * 4, width, height, Black);
	
	gdispDrawStringBox(0, height * 4, width, height,
			   timer, font, White, justifyCenter);
	gdispFlush();
	countdown--;
	updatetime += 1000;
      }
    }
  }

  if( result == orchardResultFail ) {
    chsnprintf(timer, sizeof(timer), "timeout!");
    gdispFillArea(0, height * 4, width, height, Black);
    
    gdispDrawStringBox(0, height * 4, width, height,
		       timer, font, White, justifyCenter);
    
    gdispFlush();
    chThdSleepMilliseconds(2000);
  }
  
  gdispCloseFont(font);
  orchardGfxEnd();

  return result;
}
