/* Real-input FFT implementation using the libfixmath fix16_t datatype.
 * Not the fastest implementation ever, but has a small code size.
 *
 * Refer to http://www.dspguide.com/ch12/2.htm for information on the
 * algorithm.
 *
 * (c) 2012 Petteri Aimonen <jpa @ kapsi.fi>
 * This file is released to public domain.
 */

#include <stdint.h>
#include <fix16.h>
#include <fix16_fft.h>

#ifndef FIX16_FFT_INPUT_TYPE
#error "No FFT input type defined"
#endif

#ifndef FIX16_FFT_INPUT_CONVERT
#error "No FFT input conversion defined"
#endif

#ifndef FIX16_FFT_INPUT_INDEX
#error "No FFT input index function defined"
#endif

// Fast calculation of DFT for a 4-point signal. Based on the simplicity
// of 4-point sinewave.
static void four_point_dft(FIX16_FFT_INPUT_TYPE *input, unsigned input_stride,
                           fix16_t *real, fix16_t *imag)
{
  fix16_t x0 = FIX16_FFT_INPUT_CONVERT(input[0 * input_stride]);
  fix16_t x1 = FIX16_FFT_INPUT_CONVERT(input[1 * input_stride]);
  fix16_t x2 = FIX16_FFT_INPUT_CONVERT(input[2 * input_stride]);
  fix16_t x3 = FIX16_FFT_INPUT_CONVERT(input[3 * input_stride]);
  
  real[0] = x0 + x1 + x2 + x3;
  imag[0] = 0;
  real[1] = x0 - x2;
  imag[1] = -x1 + x3;
  real[2] = x0 - x1 + x2 - x3;
  imag[2] = 0;
  real[3] = x0 - x2;
  imag[3] = x1 - x3;
}

// Mix N blocksize-sized transforms pairwise together to get:
// N/2 2*blocksize-sized transforms.
static void butterfly(fix16_t *real, fix16_t *imag,
                      unsigned blocksize, unsigned blockpairs)
{
  unsigned i, j;
  for (i = 0; i < blocksize; i++) {
    fix16_t angle = fix16_pi * i / blocksize;
    fix16_t c = fix16_cos(angle);
    fix16_t s = -fix16_sin(angle);

    fix16_t *rp = real + i;
    fix16_t *ip = imag + i;
    for (j = 0; j < blockpairs; j++) {
      // Get the odd-indexed tranform and multiply by sine
      fix16_t re = fix16_mul(rp[blocksize], c) - fix16_mul(ip[blocksize], s);
      fix16_t im = fix16_mul(ip[blocksize], c) + fix16_mul(rp[blocksize], s);

      // Update the transforms
      rp[blocksize] = rp[0] - re;
      ip[blocksize] = ip[0] - im;
      rp[0] += re;
      ip[0] += im;

      rp += blocksize * 2;
      ip += blocksize * 2;
    }
  }
}

// Reverse bits in a 32-bit number
static uint32_t rbit_32(uint32_t x)
{
#if defined(__GNUC__) && defined(__ARM_ARCH_7M__)
  __asm__("rbit %0,%0" : "=r"(x) : "0"(x));
  return x;
#else
  x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
	x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
	x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
	x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
	return((x >> 16) | (x << 16));
#endif
}

// Reverse bits in an n-bit number.
static uint32_t rbit_n(uint32_t x, unsigned n)
{
  return rbit_32(x << (32 - n));
}

// Base-2 integer logarithm
static int ilog2(unsigned x)
{
  int result = -1;
  while (x) {
    x >>= 1;
    result++;
  }
  return result;
}

// Compute a transform of the real-valued input array, and store results
// in two arrays.
// Size of each array is the same as transform_length.
// Transform length must be a power of two and atleast 4.
void fix16_fft(FIX16_FFT_INPUT_TYPE *input, fix16_t *real, fix16_t *imag,
               unsigned transform_length)
{
  int log_length = ilog2(transform_length);
  unsigned i;
  int j;

  transform_length = 1 << log_length;

  for (i = 0; i < transform_length / 4; i++)
    four_point_dft(input + FIX16_FFT_INPUT_INDEX(rbit_n(i, log_length - 2)),
                   transform_length / 4, real + 4*i, imag + 4*i);

  for (j = 2; j < log_length; j++)
    butterfly(real, imag, 1 << j, transform_length / (2 << j));
    
#ifdef FIX16_FFT_OUTPUT_SCALE
  fix16_t scale = FIX16_FFT_OUTPUT_SCALE(transform_length);
  for (i = 0; i < transform_length; i++) {
    real[i] = fix16_mul(real[i], scale);
    imag[i] = fix16_mul(imag[i], scale);
  }
#endif
}

/* Just some test code
#include <stdio.h>
int main()
{
  FIX16_FFT_INPUT_TYPE input[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  fix16_t real[16], imag[16];
    
  fix16_fft(input, real, imag, 16);
    
  int count = 16;
  int i;
  for (i = 0; i < count; i++) {
    printf("%d: %0.4f, %0.4f\n", i,
           fix16_to_float(real[i]), fix16_to_float(imag[i]));
  }
  return 0;
}
*/
