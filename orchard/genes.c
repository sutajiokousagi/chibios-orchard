#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "orchard.h"

#include "storage.h"
#include "genes.h"
#include "orchard-math.h"

#include "orchard-shell.h"

#include <string.h>
#include <stdlib.h>

static const char *first_names[16] =
  {"Happy",
   "Dusty",
   "Sassy",
   "Sexy",
   
   "Silly",
   "Curvy",
   "Nerdy",
   "Geeky",
   
   "OMG",
   "Fappy",
   "Trippy",
   "Lovely",

   "Furry",
   "WTF",
   "Spacy",
   "Lacy",
  };

static const char *middle_names[16] =
  {"Playa",
   "OMG",
   "Hot",
   "Dope",
   
   "Pink",
   "Balla",
   "Sweet",
   "Cool",
   
   "Cute",
   "Nice",
   "Fun",
   "Soft",

   "Short",
   "Tall",
   "Huge",
   "Red",
  };

static const char *last_names[8] =
  {"Virus",
   "Brain",
   "Raver",
   "Hippie",
   
   "Profit",
   "Relaxo",
   "Phage",
   "Blinky",
  };

void generateName(char *result) {
  uint32_t r = rand();
  uint8_t i = 0;

  i = strlen(strcpy(result, first_names[r & 0xF]));
  strcpy(&(result[i]), middle_names[(r >> 8) & 0xF]);
  i = strlen(result);
  strcpy(&(result[i]), last_names[(r >> 16) & 0x7]);
  i = strlen(result);

  osalDbgAssert( i < GENE_NAMELENGTH, "Name generated exceeds max length, revisit name database!\n\r" );
}


// for testing, mostly
void cmd_gename(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void) argc;
  (void) argv;
  char genName[GENE_NAMELENGTH];

  generateName(genName);
  chprintf(chp, "%s\n\r", genName);
}
orchard_command("gename", cmd_gename);

void computeGeneExpression(const genome *hapM, const genome *hapP,
			   genome *expr) {

  expr->cd_period = 6 - satadd_8_limit(hapM->cd_period, hapP->cd_period, 6);
  expr->cd_rate = (uint8_t) ((uint16_t) hapM->cd_rate + (uint16_t) hapP->cd_rate) / 2;
  expr->cd_dir = satadd_8(hapM->cd_dir, hapP->cd_dir);
  expr->sat = satadd_8(hapM->sat, hapP->sat);
  expr->hue_ratedir = 9 - satadd_8_limit(hapM->hue_ratedir & 0xF, hapP->hue_ratedir & 0xF, 9);
  expr->hue_ratedir |= (satadd_8_limit( (hapM->hue_ratedir >> 8) & 0xF,
					(hapP->hue_ratedir >> 8) & 0xF, 15) << 8);
  expr->hue_base = satsub_8(hapM->hue_base, hapP->hue_base);
  expr->hue_bound = 255 - satsub_8(hapM->hue_bound, hapP->hue_bound);
  expr->lin = satadd_8(hapM->lin, hapP->lin);
  expr->strobe = satadd_8(hapM->strobe, hapP->strobe);
  expr->accel = satadd_8(hapM->accel, hapP->accel);
  expr->mic = satadd_8(hapM->mic, hapP->mic);
  // names come from the maternal side in this society
  strncpy(expr->name, hapM->name, GENE_NAMELENGTH);
}

static void generate_gene(struct genome *haploid) {
  char genName[GENE_NAMELENGTH];

  haploid->cd_period = map((int16_t) rand() & 0xFF, 0, 255, 0, 6);
  haploid->cd_rate = (uint8_t) rand() & 0xFF;
  haploid->cd_dir = (uint8_t) rand() & 0xFF;
  haploid->sat = (uint8_t) rand() & 0xFF;
  haploid->hue_base = (uint8_t) rand() & 0xFF;
  haploid->hue_ratedir = (uint8_t) rand() & 0xFF;
  haploid->hue_bound = (uint8_t) rand() & 0xFF;
  haploid->lin = (uint8_t) rand() & 0xFF;
  haploid->strobe = (uint8_t) rand() & 0xFF;
  haploid->accel = (uint8_t) rand() & 0xFF;
  haploid->mic = (uint8_t) rand() & 0xFF;
  
  generateName(genName);
  strncpy(haploid->name, genName, GENE_NAMELENGTH);
}

#if 0
void cmd_testmap(BaseSequentialStream *chp, int argc, char *argv[]) {
  int16_t i;

  for( i = 0; i < 16; i++ ) {
    chprintf( chp, "%d : %d\n\r", i, map(i, 0, 15, 0, 255) );
  }
  for( i = 0; i < 16; i++ ) {
    chprintf( chp, "%d : %d\n\r", i, map(i, 0, 15, 0, 5) );
  }
}
orchard_command("testmap", cmd_testmap);
#endif

void print_haploid(BaseSequentialStream *chp, const genome *haploid) {
  chprintf(chp, "Individual %s:\n\r", haploid->name );
  chprintf(chp, " %3d cd_period\n\r", haploid->cd_period );
  chprintf(chp, " %3d cd_rate\n\r", haploid->cd_rate );
  chprintf(chp, " %3d cd_dir\n\r", haploid->cd_dir );
  chprintf(chp, " %3d sat\n\r", haploid->sat );
  chprintf(chp, " %3d hue_base\n\r", haploid->hue_base );
  chprintf(chp, " %3d hue_ratedir\n\r", haploid->hue_ratedir );
  chprintf(chp, " %3d hue_bound\n\r", haploid->hue_bound );
  chprintf(chp, " %3d lin\n\r", haploid->lin );
  chprintf(chp, " %3d strobe\n\r", haploid->strobe );
  chprintf(chp, " %3d accel\n\r", haploid->accel );
  chprintf(chp, " %3d mic\n\r", haploid->mic );
}

void cmd_geneseq(BaseSequentialStream *chp, int argc, char *argv[]) {
  uint8_t which;
  const struct genes *family;
  struct genome diploid;

  family = (const struct genes *) storageGetData(GENE_BLOCK);
  
  if( argc != 1 ) {
    chprintf(chp, "Usage: geneseq <individual>, where <individual> is 0-%d\n\r", GENE_FAMILYSIZE - 1);
    return;
  }
  which = (uint8_t) strtoul( argv[0], NULL, 0 );
  if( which >= GENE_FAMILYSIZE ) {
    chprintf(chp, "Usage: geneseq <individual>, where <individual> is 0-%d\n\r", GENE_FAMILYSIZE - 1);
    return;
  }

  if( family->signature != GENE_SIGNATURE ) {
    chprintf(chp, "Invalid genome signature\n\r" );
    return;
  }
  if( family->version != GENE_VERSION ) {
    chprintf(chp, "Invalid genome version\n\r" );
    return;
  }
  chprintf(chp, "--Maternal Haploid--\n\r");
  print_haploid(chp, &(family->haploidM[which]));
  chprintf(chp, "--Paternal Haploid--\n\r");
  print_haploid(chp, &(family->haploidP[which]));

  chprintf(chp, "--Diploid expression--\n\r");
  computeGeneExpression(&(family->haploidM[which]), &(family->haploidP[which]), &diploid);
  print_haploid(chp, (const genome *) &diploid);
}
orchard_command("geneseq", cmd_geneseq);

static void init_genes(uint32_t block) {
  struct genes family;
  char genName[GENE_NAMELENGTH];
  int i;

  family.signature = GENE_SIGNATURE;
  family.version = GENE_VERSION;
    
  generateName(genName);
  strncpy(family.name, genName, GENE_NAMELENGTH);

  for( i = 0; i < GENE_FAMILYSIZE; i++ ) {
    generate_gene(&family.haploidM[i]);
    generate_gene(&family.haploidP[i]);
  }

  storagePatchData(block, (uint32_t *) &family, GENE_OFFSET, sizeof(struct genes));
}

void geneStart() {
  const struct genes *family;

  family = (const struct genes *) storageGetData(GENE_BLOCK);

  if( family->signature != GENE_SIGNATURE ) {
    init_genes(GENE_BLOCK);
  } else if( family->version != GENE_VERSION ) {
    init_genes(GENE_BLOCK);
  }
}

