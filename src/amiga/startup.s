| vi:filetype=gas68k:
	.include "hwregs.inc"

	.global move_init_stack
	.global halt_cpu
	.global panic
	.extern main

	.section .startup,"a"

	| enter supervisor mode (assumes VBR=0)
	move.l #super, 0x80
	trap #0
super:
	or.w #0x0700, %sr	| disable interrupts

	| zero the .bss section
	move.l #_bss_start, %a0
	move.l #_bss_end, %a1
	cmp.l %a0, %a1
	beq.s 1f	| skip zeroing if the section is empty
0:	clr.b (%a0)+
	cmp.l %a0, %a1
	bne.s 0b
1:
	| setup a temporary stack
	move.l #0x40000, %sp
	and.w #0xf8ff, %sr	| enable interrupts

	| initialize early exception handlers
	jsr exc_init

	jsr main
0:	bra.b 0b

move_init_stack:
	move.l #0x40000, %a1	| a1 <- old stack
	move.l 4(%sp), %a0	| a0 <- new stack
0:	move.l -(%a1), %d0
	move.l %d0, -(%a0)
	cmp.l %sp, %a1
	bhi.s 0b
	move.l %a0, %sp
	rts

halt_cpu:
	stop #0x2700

panic:
	or.w #0x0700, %sr
	move.l #0xdff000, %a0
	move.w #0x7fff, DMACON(%a0)	| clear all DMACON bits
	move.w #0xf00, COLOR0(%a0)

	pea str_panic
	jsr printf
	add.l #8, %sp	| also get rid of the reutrn address

	jsr printf
	bra halt_cpu

str_panic: .asciz "Kernel panic!\n~~~~~~~~~~~~~\n"
