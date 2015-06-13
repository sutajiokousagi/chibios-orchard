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
} auditEntry;
  
typedef struct auditLog {
  uint32_t  entry_count; // implementation depends on this being the first structure entry
  uint32_t  signature;
  uint32_t  version;

  struct auditEntry firstEntry; // first entry in an array of entries that starts here
} auditLog;

void auditStart(void);
int32_t auditCheck(uint32_t test_type);
void auditUpdate(const char *name, OrchardTestType type, OrchardTestResult result );
void auditPrintLog(void);

#endif /* __ORCHARD_AUDIT__ */

/*
  test writing status

  touch   trivial; interactive
  accel   trivial   * <- "marble" app but rolling into four corners to pass the test.
  led     can't trivially test; comprehensive, interactive
  ble     trivial
  oled    can't trivially test; interactive
  charger trivial   **  <- plug in, plug out; check currents & charge rate
  gg      trivial
  gpiox   trivial
  radio   trivial   *** <- tx random # to peer that echoes the number back. Requires peer app.
  cpu     trivial
  usb     trivial
  mic     trivial   ** <- app-oscope launch, auto-kill with timer
*/
