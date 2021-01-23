| vi:filetype=gas68k:
	.equ CMD_READ, 2
	.equ EXEC_DO_IO, -0x1c8
	.equ LOADADDR,	0x10000

start:
| starting with trackdisk device I/O request pointer in a1
| load the program at 0x100 and jump there
| program length is patched by mk_adf just before start
	move.l -6(%pc), %d0		| get program size
	move.w #0x080, 0xdff180
	move.l %d0, 0x24(%a1)		| I/O length
	move.l #LOADADDR, 0x28(%a1)	| I/O data pointer
	move.l #512, 0x2c(%a1)		| I/O offset (skip first sector)
	move.w #CMD_READ, 0x1c(%a1)	| I/O command
	jsr EXEC_DO_IO(%a6)
	move.w #0xf0f, 0xdff180

	jmp LOADADDR
	.align 4
