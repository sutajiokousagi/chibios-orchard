#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "orchard.h"

#include "orchard-test.h"
#include "test-audit.h"

orchard_test_end();

void orchardTestInit(void) {
  auditStart();  // start / initialize the test audit log
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


void orchardTestRun(OrchardTestType test_type) {
  const TestRoutine *cur_test;
  OrchardTestResult test_result;
  
  cur_test = orchard_test_start();
  while(cur_test->test_function) {
    test_result = cur_test->test_function(cur_test->test_name, test_type);
    auditUpdate(cur_test->test_name, test_type, test_result);
    cur_test++;
  }
}
