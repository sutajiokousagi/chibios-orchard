#include "ch.h"
#include "chprintf.h"
#include "orchard.h"

#include "storage.h"
#include "genes.h"

static void init_genes(uint32_t block) {
  struct genes family;
  uint32_t id = 0;

  family.signature = GENE_SIGNATURE;
  family.version = GENE_VERSION;
    
  id = SIM->UIDL + SIM->UIDML + SIM->UIDMH; // TODO: turn this into a hash
  // TODO: pick a name based upon a name lookup table so it's a more intuitive name
  chsnprintf(family.name, 16, "anon%x", id);

  storagePatchData(block, (uint32_t *) &family, GENE_OFFSET, sizeof(struct genes));
}

void geneStart() {
  const struct genes *family;

  family = (const struct genes *) storageGetData(GENE_BLOCK);

  if( family->signature != GENE_SIGNATURE ) {
    init_genes(GENE_BLOCK);
  }
}
