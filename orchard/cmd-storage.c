#include "ch.h"
#include "shell.h"
#include "chprintf.h"

#include "flash.h"
#include "storage.h"
#include "orchard-shell.h"
#include <string.h>

#define TD_LEN 4
void cmd_storage_test(BaseSequentialStream *chp, int argc, char *argv[]) {
  uint32_t block;
  uint32_t testDataA[TD_LEN] = {0xfeedface, 0xdeadbeef, 0x340dbabe, 0x696955aa};
  uint32_t testDataB[TD_LEN] = {0x12345678, 0xaaaa5555, 0x3333cccc, 0x66660000};
  uint32_t testDataC[TD_LEN] = {0xffffffff, ORFS_SIG, 0x31415926, 0x00000000};
  uint32_t testDataGen[TD_LEN];
  const uint32_t *data;
  uint32_t i;
  uint32_t offset;

  (void)argv;
  if (argc != 1) {
    chprintf(chp, "Usage: storetest 3. You must pass the argument 3 to confirm the test. Warning: this will erase all user storage.\r\n");
    return;
  }
  if( strtoul(argv[0], NULL, 0) != 3 ) {
    chprintf(chp, "You must pass the argument 3 to confirm the test. This test erases all memory\r\n");
    return;
  }
  chprintf( chp, "  INFO: BLOCK_TOTAL is %d, should be 7\n\r", BLOCK_TOTAL );
  chprintf( chp, "  INFO: STORAGE_SIZE is 0x%x, should be 0x1c00\n\r", STORAGE_SIZE );
  chprintf( chp, "  INFO: BLOCK_SIZE is %d, should be 1024\n\r", BLOCK_SIZE );
  chprintf( chp, "  INFO: SECTOR_MIN is %d, should be 120\n\r", SECTOR_MIN );
  chprintf( chp, "  INFO: SECTOR_MAX is %d, should be 127\n\r", SECTOR_MAX );
  chprintf( chp, "  INFO: SECTOR_COUNT is %d, should be 8\n\r", SECTOR_COUNT );

  // testing routines for storage, this will erase all memory to start from a known state
  flashErase(F_USER_SECTOR_START, SECTOR_COUNT);

  chprintf( chp, "Test 0: basic store and fetch\n\r" );
  // test getting data sector on an empty FS
  block = 0;
  data = storageGetData(block);
  for( i = 0; i < 4; i++ ) {
    if( data[i] != 0xFFFFFFFF ) {
      chprintf( chp, "FAIL: empty sector isn't empty\n\r" );
      return;
    }
    
  }
  chprintf( chp, "  INFO: sizeof(testDataA) = %d (should be 16)\n\r", sizeof(testDataA) );
  storagePatchData(block, testDataA, 0, sizeof(testDataA));

  data = storageGetData(block);
  for( i = 0; i < 4; i++ ) {
    if( data[i] != testDataA[i] ) {
      chprintf( chp, "FAIL: test0: data was not stored correctly\n\r" );
      return;
    }
  }
  
  chprintf( chp, "Test 1: offset store and fetch\n\r" );
  block = BLOCK_TOTAL - 1;
  offset = 72;  // just pick an offset for testing
  // this time test storagePatchData without a get data prior -- this should
  // force an automatic allocation of a new block
  storagePatchData(block, testDataB, offset, sizeof(testDataB));
  // now, offset 72 has testDataB
  data = storageGetData(block);
  for( i = 0; i < 4; i++ ) {
    if( data[i + (offset >> 2)] != testDataB[i] ) {
      chprintf( chp, "FAIL: test1: data was not stored correctly\n\r" );
      return;
    }
  }

  // test patching data dataset B to A over existing data
  chprintf( chp, "Test 2: patch data over exising data\n\r" );
  storagePatchData(block, testDataA, offset, sizeof(testDataA));
  // now, offset 72 has testDataA
  data = storageGetData(block);
  for( i = 0; i < 4; i++ ) {
    if( data[i + (offset >> 2)] != testDataA[i] ) {
      chprintf( chp, "FAIL: test2: data was not stored correctly\n\r" );
      chprintf( chp, "    data: %x, &data: %x, testData: %x\n\r", data[i + (offset >> 2)], (uint32_t) data,  testDataA[i] );
      return;
    }
  }
  
  chprintf( chp, "Test 3: patch data over empty data, with existing data after the patch\n\r" );
  offset = 8;
  storagePatchData(block, testDataB, offset, sizeof(testDataB));
  // now, offset 8 has testDataB, 72 has testDataA
  data = storageGetData(block);
  for( i = 0; i < 4; i++ ) {
    if( data[i + (offset >> 2)] != testDataB[i] ) {
      chprintf( chp, "FAIL: test3: data was not stored correctly\n\r" );
      return;
    }
  }
  
  chprintf( chp, "Test 4: patch data over empty data, with existing data around the patch\n\r" );
  offset = 24;
  storagePatchData(block, testDataC, offset, sizeof(testDataC));
  // now, offset 8 has testDataB, 72 has testDataA, 24 has testDataC
  data = storageGetData(block);
  for( i = 0; i < 4; i++ ) {
    if( data[i + (8 >> 2)] != testDataB[i] ) {
      chprintf( chp, "FAIL: test4: data at 8 was not stored correctly\n\r" );
      return;
    }
  }
  
  for( i = 0; i < 4; i++ ) {
    if( data[i + (24 >> 2)] != testDataC[i] ) {
      chprintf( chp, "FAIL: test4: data at 24 was not stored correctly\n\r" );
      return;
    }
  }
  
  for( i = 0; i < 4; i++ ) {
    if( data[i + (72 >> 2)] != testDataA[i] ) {
      chprintf( chp, "FAIL: test4: data at 72 was not stored correctly\n\r" );
      return;
    }
  }

  chprintf( chp, "Test 5: make sure our original block 0 is intact\n\r" );
  block = 0;
  data = storageGetData(block);
  for( i = 0; i < 4; i++ ) {
    if( data[i] != testDataA[i] ) {
      chprintf( chp, "FAIL: test5: original data disturbed\n\r" );
      return;
    }
  }
  for( i = 4; i < (BLOCK_SIZE / sizeof(uint32_t)) - sizeof(orfs_head) / sizeof(uint32_t) + 1; i++ ) {
    if( data[i] != 0xFFFFFFFF ) {
      chprintf( chp, "FAIL: test5: original data disturbed, blank area isn't blank\n\r" );
      chprintf( chp, "  index: %d, data: %x\n\r", i, data[i] );
      return;
    }
  }
  
  // now, patch data on all blocks to ensure we're full-up allocated
  chprintf( chp, "Test 6: fill up all available blocks with unique data\n\r" );
  offset = 4;   // pick an offset to work with that's not 0 to exercise stuff a bit
  for( block = 0; block < BLOCK_TOTAL; block++ ) {
    for( i = 0; i < TD_LEN; i++ ) {
      testDataGen[i] = block + i;
    }
    storagePatchData(block, testDataGen, offset, sizeof(testDataGen));
  }

  // confirm the patches all went through
  for( block = 0; block < BLOCK_TOTAL; block++ ) {
    for( i = 0; i < TD_LEN; i++ ) {
      testDataGen[i] = block + i;
    }
    data = storageGetData(block);
    for( i = 0; i < TD_LEN; i++ ) {
      if( data[i + (offset >> 2)] != testDataGen[i] ) {
	chprintf( chp, "FAIL: test6: memory fill-up test fail\n\r" );
	chprintf( chp, "   data: %x, generated data: %x\n\r",
		  data[i + (offset >> 2)], testDataGen[i] );
	return;
      }
    }
  }

  chprintf( chp, "Test 7: patch and re-patch a single block a few times\n\r" );
  offset = 512;
  block = 2;
  for( i = 0; i < (BLOCK_TOTAL * 3); i++ ) { // force us to precess through the free block
    testDataGen[0] = i + 0xface;
    storagePatchData(block, testDataGen, offset, 4); // just patch the one word
    data = storageGetData(block);
    if( data[offset >> 2] != (i + 0xface) ) {
      chprintf( chp, "FAIL: test7: memory rewrite\n\r" );
      return;
    }
  }

  chprintf( chp, "Test 8: reboot, and run storetest2 to check resident image\n\r" );
  
}
orchard_command("storetest", cmd_storage_test);

void cmd_storage_test2(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void) argc;
  (void) argv;
  
  uint32_t block;
  uint32_t offset;
  uint32_t testDataGen[4];
  const uint32_t *data;
  uint32_t i;

  offset = 4;   // pick same offset as test 6 to check persistence
  
  chprintf( chp, "Test 9: checking for signatures in all blocks\n\r" );
  // confirm the patches all went through
  for( block = 0; block < BLOCK_TOTAL; block++ ) {
    for( i = 0; i < TD_LEN; i++ ) {
      testDataGen[i] = block + i;
    }
    data = storageGetData(block);
    for( i = 0; i < TD_LEN; i++ ) {
      if( data[i + (offset >> 2)] != testDataGen[i] ) {
	chprintf( chp, "FAIL: test8: persistence fail part A\n\r" );
	return;
      }
    }
  }

  chprintf( chp, "Test 10: checking for signatures in repatched block\n\r" );
  offset = 512;
  block = 2;
  data = storageGetData(block);
  if( (data[offset >> 2] != (BLOCK_TOTAL * 3 - 1) + 0xface) ) {
    chprintf( chp, "FAIL: test8: persistence fail part B\n\r" );
    return;
  }
}
orchard_command("storetest2", cmd_storage_test2);

// to blank memory after the tests, use:
// flasherase 120,8, this will blank the whole user memory area to a virgin state
