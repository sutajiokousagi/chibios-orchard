#ifndef __ORCHARD_STORAGE__
#define __ORCHARD_STORAGE__

#include "flash.h"

// naming conventions:
// SECTORS refer to the physical sector number of a page of Flash
// BLOCKS refer to a logical enumeration of page-sized data
// STORAGE refer to physical addresses in memory

#define STORAGE_START (uint8_t *) (__storage_start__)
#define SECTOR_SIZE   (uint32_t) FTFx_PSECTOR_SIZE
#define BLOCK_SIZE    SECTOR_SIZE

// one sector is reserved for swapping in for updates
#define STORAGE_SIZE  ((uint32_t) (__storage_size__) - (uint32_t) SECTOR_SIZE)

#define BLOCK_TOTAL   (STORAGE_SIZE / SECTOR_SIZE)  // number of blocks, 1-based
#define BLOCK_MAX     (BLOCK_TOTAL - 1)   // max block index, 0-based
#define SECTOR_NUM_ERASE  1   // number of sectors reserved for erasing/junking
#define SECTOR_MAX ( (uint32_t) __storage_end__ / SECTOR_SIZE)
#define SECTOR_MIN ((uint32_t) __storage_start__ / SECTOR_SIZE)
#define SECTOR_COUNT (SECTOR_MAX - SECTOR_MIN + 1)
#define SECTOR_INVALID   0xFFFFFFFF  // return coode for errors

#define JOURNAL_INVALID  0xFFFFFFFF
#define JOURNAL_YOUNGEST 0xFFFFFFFE  // journal numbers start from 0xFFFFFFFE and count down

#define ORFS_SIG   0x4F524653
#define ORFS_REV   1

typedef struct orfs_head {
  // note all elements are word-aligned because the data patching algorithm can only
  // patch on a 32-bit *word* basis
  uint32_t  signature;  // set to 'ORFS'; if invalid, assumed the sector can be erased
  uint32_t   version;    // set to 1
  uint32_t  block;     // set to the block number
  uint32_t  journalrev; // decrement every time a block is updated
  uint32_t   firstData;  // first data word
  // no checksum because we want to be able to do efficient patching without having
  // to nuke the structure to reprogram a new checksum
} orfs_head;

// Hardware restrictions:
// * Pages are 1k in size
// * You can't write 0's over 0's -- it reduces flash lifetime
// * Minimum patch size is 4 bytes (one word)

// General strategy:
// * Virtualize blocks, so that there is (sectors-1) blocks available
// * Reserve at least one sector for erasing and updating
// * Allocate blocks in empty sectors on demand
// * When all sectors are full, look for the duplicated block with the lowest journal number;
//   erase that sector that holds that block, and blast the new data into it

// returns a read-only pointer to the data of the current sector
// the data is directly in FLASH so you can't write to it
const void *storageGetData(uint32_t block);

// updating a sector happens by "patching"
// the function automatically handles migrating the non-patch data to the new sector copy
// offset and size are in bytes, but should be word-aligned
int8_t storagePatchData(uint32_t block, uint32_t *data, uint32_t offset, uint32_t size);

// When laying out storage structures using ORFS, make sure the total size of the structure
// aligns to a 4-byte boundary!

#endif // __ORCHARD_STORAGE__
