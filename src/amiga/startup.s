| vi:filetype=gas68k:
	.global halt_cpu
	.extern main

	.section .startup,"a"

	| enter supervisor mode (assumes VBR=0)
	move.l #super, 0x80
	trap #0
super:
	ori.w #0x0300, %sr	| disable interrupts

	| zero the .bss section
	move.l #_bss_start, %a0
	move.l #_bss_end, %a1
	cmp.l %a0, %a1
	beq.s 1f	| skip zeroing if the section is empty
0:	clr.b (%a0)+
	cmp.l %a0, %a1
	bne.s 0b
1:
	| setup the stack at the top of (traditional) chipmem for now
	move.l #0x80000, %sp
	andi.w #0xf8ff, %sr	| enable interrupts

	| initialize early exception handlers
	jsr exc_init

	jsr main
0:	bra.b 0b

halt_cpu:
	stop #0x2700
