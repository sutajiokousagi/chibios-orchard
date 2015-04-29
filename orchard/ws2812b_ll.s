.text

  /***
    WS2812B Low-Level code
      WS2812B timing requirements:
      To transmit a "0": 400ns high, 850ns low  +/- 150ns  total = 1250ns
	19 cycles H / 41 cycles L
      To transmit a "1": 800ns high, 450ns low  +/- 150ns  total = 1250ns
	38 cycles H / 22 cycles L
      Clock balance to within +/-150ns = 7 cycles

      Reset code is 50,000ns of low

      Each instruction takes 20.8ns, 1250/20.8 = 60 instructions per cycle total
      
      Data transmit to chain is {G[7:0],R[7:0],B[7:0]}

      Data passed in shall be an array of bytes, GRB ordered, of length N specified as a parameter

   ***/

.global ledUpdate

.equ  mainled_bit0, 0x0
.equ  mainled_bit1, 0x1

eclr    .req r0  // register for clearing a bit
eset    .req r1  // register for setting a bit
bitmask .req r5  // mask to jam into register

stop    .req r3  // stop value for fbptr
curpix  .req r4  // current pixel value
bit     .req r6  // bit counter (shift loop per color)
fbptr   .req r7  // frame buffer pointer

	// r2 is GPIO dest location
	// r0 is "0" bit number code
	// r1 is "1" bit number code
	// r3 is loop counter for total number of pixels
	// r4 is the current pixel value
	// r5 is the test value for the top bit of current pixel
	// r6 is the loop counter for bit number in a pixel
	// r7 is current pointer to the pixel array
	
ledUpdate:	
	// r0  uint8_t *fb
	// r1  uint32_t	len
	push {r4,r5,r6,r7}

	mov fbptr, r0        // r7 gets to be the pointer to the pixel array

	mov stop, r1         // r3 gets number of pixels to go through
	mov r0, #3           // r0 already saved, use as temp for multiply
	mul stop, stop, r0   // mult by 3 for RGB
	add stop, stop, fbptr // add in the fptr to finalize stop computation value
	
	ldr eset, PORTESET
	ldr eclr, PORTECLR
	ldr bitmask, LEDBIT

	///////////////////////
looptop:
	str bitmask, [eset]   // start with bit set
	mov bit, #8
	ldrb curpix, [fbptr]      // load the word at the pointer location to r4
	lsl curpix, curpix, #24  // shift left by 24 so the carry pops out
	add fbptr, fbptr, #1
	// above is 5 cycles
	b pixloop_fromtop // 7 total
	
pixloop:
	str bitmask, [eset]   // start with bit set
	nop // equalize looptop path
	nop
	nop
	nop
	nop
	nop
pixloop_fromtop:	
	lsl curpix, #1
	bcs oneBranch
	
	// zero path -- so far, 5 cycles high
	nop
	nop
	nop
	nop
	nop
	
	nop
	nop
	nop
	nop
	
	str bitmask, [eclr]
	nop
	nop
	nop
	nop
	nop
	
	nop
	nop
	nop
	nop
	nop
	
	nop
	nop
	nop
	nop
	nop
	
	nop
	nop
	nop
	nop
	nop
	
	nop
	nop
	nop
	nop
	nop
	
	nop
	nop
	nop
	nop
	nop
	
	nop
	b  pixEpilogue

oneBranch:	
	// one path
	nop
	nop
	nop
	nop
	nop
	
	nop
	nop
	nop
	nop
	nop
	
	nop
	nop
	nop
	nop
	nop
	
	nop
	nop
	nop
	nop
	nop
	
	nop
	nop
	nop
	nop
	nop
	
	nop
	nop
	
	str bitmask, [eclr]
	nop
	nop
	nop
	nop
	nop
	
	nop
	nop
	nop
	nop
	nop
	
	nop
	nop
	nop
	nop
	nop
	
	nop
	
pixEpilogue:
	sub bit, bit, #1
	bne pixloop

	cmp fbptr, stop
	bne looptop
	
	b exit
	
exit:	
	
	pop {r4,r5,r6,r7}
	bx lr
	
.balign 4
PORTESET:	
.word 0xF8000104
PORTECLR:	
.word 0xF8000108
LEDBIT:	
.word 0x20000

.end
	
