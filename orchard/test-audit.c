#include "ch.h"
#include "chprintf.h"
#include "orchard.h"

#include "storage.h"
#include "test-audit.h"

static void init_audit(uint32_t block) {
  struct auditLog log;

  log.signature = AUDIT_SIGNATURE;
  log.version = AUDIT_VERSION;
    
  storagePatchData(block, (uint32_t *) &log, AUDIT_OFFSET, sizeof(struct auditLog));
}

void auditStart(void) {
  const struct auditLog *log;

  log = (const struct auditLog *) storageGetData(AUDIT_BLOCK);

  if( log->signature != AUDIT_SIGNATURE ) {
    init_audit(AUDIT_BLOCK);
  }
}

// check audit log to see if all tests of test_type have passed
int32_t auditCheck(uint32_t test_type) {
  return -1;
}

void auditUpdate(const char *name, OrchardTestType type, OrchardTestResult result ) {
  return;
}
