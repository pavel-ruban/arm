.thumb

.global custom_asm

@ Define interrupt vectors table.
.include "vectors.s"

.section .text

@ Reset Handler.
.type	reset_asm	"function"
.align 2
.global reset_asm
reset_asm:
		@bl	Reset_Handler
		bl	_start
		bl	main
		b	reset_asm

@ Custom routines.
.type	custom_asm	"function"
custom_asm:
		ldr	r4,	=0x40010800
		ldr	r1,	[r4]
		bfc	r1,	#0,	#8
		ldr	r2,	=0b00010001
		bfi	r1,	r2,	#0,	#8
		str	r1,	[r4]
		bx lr;
