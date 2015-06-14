#ifndef __ORCHARD_AUDIT__
#define __ORCHARD_AUDIT__

#include "storage.h"
#include "orchard-test.h"

/* 
   Test audit log.

   Situated at the top block in user storage space, it records a history of a device's
   testing. 
 */

#define AUDIT_SIGNATURE  0x41554454   // AUDT
#define AUDIT_BLOCK   (BLOCK_MAX)
#define AUDIT_OFFSET 0
#define AUDIT_VERSION 1

#define AUDIT_FAIL   0xFFFFFFFF   // value for a failing block

// note that the following two structures reflect data stored in nonvolatile memory
// changing this will break compatibility with existing data stored in memory

// this structure is carefully constructed to be word-aligned in size in memory
// we want to minimize size so we can fit as many audit entries into a 1k block as possible
typedef struct auditEntry {
  uint16_t runs;    // number of times the test has been run
  uint8_t  type;    // type of test (explicit cast from typdef to int32 to match native write size)
  uint8_t  result;  // last test result
  char testName[TEST_NAME_LENGTH];
} __attribute__((__packed__)) auditEntry;
  
typedef struct auditLog {
  uint32_t  entry_count; // implementation depends on this being the first structure entry
  uint32_t  signature;
  uint32_t  version;

  struct auditEntry firstEntry; // first entry in an array of entries that starts here
} __attribute__((__packed__)) auditLog;

void auditStart(void);
uint32_t auditCheck(uint32_t test_type);
void auditUpdate(const char *name, OrchardTestType type, OrchardTestResult result );
void auditPrintLog(BaseSequentialStream *chp);

#endif /* __ORCHARD_AUDIT__ */

/*
  test writing status

  touch   trivial; interactive
  accel   trivial; silicon works if trivial test passes. not tested: drop/bump interrupt pin
  led     can't trivially test; comprehensive, interactive
  ble     trivial
  oled    can't trivially test; interactive
  charger trivial; interactive
  gg      trivial; tested by charger interactive
  gpiox   trivial; deeper tests tested by other tests
  radio   trivial; interactive. Test is not yet stable, test peer tends to crash
  cpu     trivial; deeper tests tested by other tests
  usb     trivial; tested by charger interactive
  mic     trivial; interactive

*/
