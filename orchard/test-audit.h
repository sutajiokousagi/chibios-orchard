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

typedef struct auditEntry {
  const char testName[TEST_NAME_LENGTH];
  int32_t  type;    // type of test (explicit cast from typdef to int32 to match native write size)
  int32_t  runs;    // number of times the test has been run
  int32_t  result;  // last test result
  struct auditEntry *nextEntry;
} auditEntry;
  
typedef struct auditLog {
  uint32_t  signature;
  uint32_t  version;

  struct auditEntry *audit_entries;
} auditLog;

void auditStart(void);
int32_t auditCheck(uint32_t test_type);
void auditUpdate(const char *name, OrchardTestType type, OrchardTestResult result );

#endif /* __ORCHARD_AUDIT__ */

/*
  blocks to test 

  int32_t   touch_count;
  int32_t   accel_count;
  int32_t   led_count;
  int32_t   ble_count;
  int32_t   oled_count;
  int32_t   charger_count;
  int32_t   gg_count;
  int32_t   gpiox_count;
  int32_t   radio_count;
  int32_t   cpu_count;
  int32_t   usb_count;
  int32_t   mic_count;
*/
