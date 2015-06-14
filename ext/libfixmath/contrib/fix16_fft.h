/* Real-input FFT implementation using the libfixmath fix16_t datatype.
 * Not the fastest implementation ever, but has a small code size.
 *
 * Refer to http://www.dspguide.com/ch12/2.htm for information on the
 * algorithm.
 *
 * (c) 2012 Petteri Aimonen <jpa @ kapsi.fi>
 * This file is released to public domain.
 */

#ifndef __FIX16_FFT_H__
#define __FIX16_FFT_H__
#include <stdint.h>
#include <fix16.h>

// You can change the input datatype and intermediate scaling here.
// By default, the output is divided by the transform length to get
//a normalized FFT.
// Input_convert determines the scaling of intermediate values.
// Multiplication by 256 gives a nice compromise between precision
// and numeric range.
#ifndef FIX16_FFT_INPUT_TYPE
#define FIX16_FFT_INPUT_TYPE uint8_t
#endif

#ifndef FIX16_FFT_INPUT_CONVERT
#define FIX16_FFT_INPUT_CONVERT(x) ((x) << 8)
#endif

#ifndef FIX16_FFT_INPUT_INDEX
#define FIX16_FFT_INPUT_INDEX(x) (x)
#endif

#ifndef FIX16_FFT_OUTPUT_SCALE
#define FIX16_FFT_OUTPUT_SCALE(transform_size) \
  (fix16_one * 256 / transform_size)
#endif

void fix16_fft(FIX16_FFT_INPUT_TYPE *input, fix16_t *real, fix16_t *imag,
               unsigned transform_length);

#endif /* __FIX16_FFT_H__ */
