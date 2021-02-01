| vi:filetype=gas68k:
	.include "hwregs.inc"

	.section .text

	.global exc_init
exc_init:
	move.l #exc_addr, 0xc
	rts

	.global exc_addr
exc_addr:
	ori.w #0x700, %sr
	move.l #0xdff000, %a0
	move.w #0x7fff, DMACON(%a0)	| clear all DMACON bits

	move.l #str_exc3beg, -(%sp)
	jsr ser_print
	add.l #4, %sp

	move.l 2(%sp), %d0
	move.l %d0, -(%sp)
	bsr print_hex
	add.l #4, %sp

	move.l #str_exc3end, -(%sp)
	jsr ser_print
	add.l #4, %sp

	move.l #3, -(%sp)

colbars_infloop:
	move.l #0xdff000, %a0
	move.w #0, COLOR0(%a0)
	move.b #128, %d1
	move.l (%sp), %d2

0:	move.b VHPOSR(%a0), %d0
	cmp.b %d1, %d0
	bne.s 0b
	add.b #4, %d1
	move.w #0xfff, COLOR0(%a0)
1:	move.b VHPOSR(%a0), %d0
	cmp.b %d1, %d0
	bne.s 1b
	add.b #8, %d1
	move.w #0, COLOR0(%a0)
	sub.b #1, %d2
	bne.s 0b

0:	cmp.b #0xff, VHPOSR(%a0)
	bne.s 0b
	bra.s colbars_infloop

print_hex:
	move.l #hexbuf, %a0
	move.l 4(%sp), %d0
	move.l #8, %d3
0:	rol.l #4, %d0
	move.b %d0, %d1
	and.b #0xf, %d1
	cmp.b #10, %d1
	bhs.s 1f
	add.b #'0', %d1		| d1 is in [0, 9]
	bra.s 2f
1:	add.b #'a'-10, %d1	| d1 is in [10, 15]
2:	move.b %d1, (%a0)+
	sub.l #1, %d3
	bne.s 0b

	move.b #0, (%a0)+
	move.l #hexbuf, -(%sp)
	jsr ser_print
	add.l #4, %sp
	rts

hexbuf: .fill 16
str_exc3beg: .asciz "Exception 3: address error ("
str_exc3end: .asciz ")\n"
