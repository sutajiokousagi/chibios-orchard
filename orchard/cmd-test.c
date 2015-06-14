#include "ch.h"
#include "shell.h"
#include "chprintf.h"

#include "orchard-shell.h"

#include "orchard-test.h"
#include "test-audit.h"

#include "orchard-app.h"

void cmd_test(BaseSequentialStream *chp, int argc, char *argv[])
{
  const TestRoutine *test;
  OrchardTestResult  test_result;
  OrchardTestType  test_type;
  const OrchardApp *test_app;
  
  if( argc != 2 ) {
    chprintf(chp, "Usage: test <testname> <testtype>, where testname is one of:\n\r");
    orchardListTests(chp);
    chprintf(chp, "And testtype is according to the following table:\n\r" );
    chprintf(chp, "  0 - Power On\n\r" );
    chprintf(chp, "  1 - Trivial\n\r" );
    chprintf(chp, "  2 - Comprehensive\n\r" );
    chprintf(chp, "  3 - Interactive\n\r" );
    return;
  }

  test_app = orchardAppByName("~testmode");
  if (test_app)
    orchardAppRun(test_app);
  
  test = orchardGetTestByName(argv[0]);
  test_type = (OrchardTestType) strtoul(argv[1], NULL, 0);

  if (test == NULL) {
    chprintf(chp, "Test %s was not found in the test table.\n\r", argv[0]);
    return;
  }

  test_result = test->test_function(argv[0], test_type);

  chprintf(chp, "Test result code is %d\n\r", (int8_t) test_result);
}

orchard_command("test", cmd_test);

void cmd_printaudit(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void) chp;
  (void) argc;
  (void) argv;
  
  auditPrintLog(chp);
}
orchard_command("auditlog", cmd_printaudit);

void cmd_testall(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void) chp;
  OrchardTestType  test_type;
  const OrchardApp *test_app;

  test_app = orchardAppByName("~testmode");
  if( test_app ) {
    orchardAppRun(test_app);
  }
  
  if( argc != 1 ) {
    chprintf(chp, "Usage: testall <testtype>\n\r");
    chprintf(chp, "Testtype is a code denoting the test type (see orchard-test.h)\n\r" );
    return;
  }
  test_type = (OrchardTestType) strtoul(argv[0], NULL, 0);

  orchardTestRun(chp, test_type);
  
}
orchard_command("testall", cmd_testall);
