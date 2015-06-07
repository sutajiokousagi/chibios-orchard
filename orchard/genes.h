#define GENE_SIGNATURE  0x424D3135  // BM15
#define GENE_BLOCK  0
#define GENE_OFFSET 0
#define GENE_VERSION 1

#define GENE_NAMELENGTH 16    // null terminated, so 15 char name max

typedef struct genes {
  uint32_t  signature;
  uint32_t  version;
  char      name[GENE_NAMELENGTH];
} genes;

void geneStart(void);
