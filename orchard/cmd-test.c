#include "ch.h"
#include "shell.h"
#include "chprintf.h"

#include "orchard-shell.h"

#include "orchard-test.h"
#include "test-audit.h"

void cmd_test(BaseSequentialStream *chp, int argc, char *argv[])
{
  const TestRoutine *test;
  OrchardTestResult  test_result;
  OrchardTestType  test_type;
  
  if( argc != 2 ) {
    chprintf(chp, "Usage: test <testname> <testtype>, where testname is one of:\n\r");
    orchardListTests();
    chprintf(chp, "And testtype is a code denoting the test type (see orchard-test.h)\n\r" );
    return;
  }

  test = orchardGetTestByName(argv[0]);
  test_type = (OrchardTestType) strtoul(argv[1], NULL, 0);

  if( test == NULL ) {
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
  
  auditPrintLog();
}
orchard_command("auditlog", cmd_printaudit);
