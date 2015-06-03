#ifndef __HEX_H__
#define __HEX_H__

int print_hex_offset(BaseSequentialStream *chp,
                     const void *block, int count, int offset, uint32_t start);
int print_hex(BaseSequentialStream *chp,
              const void *block, int count, uint32_t start);

#endif /*__HEX_H__*/
