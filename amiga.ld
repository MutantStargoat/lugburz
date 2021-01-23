OUTPUT_ARCH(m68k)

PROVIDE (_stacktop = 0x80000);

SECTIONS {
	/* bootblock will load us at 10000h */
	. = 0x10000;

	.startup : { * (.startup); }
	.text : { * (.text); }
	.rodata : { * (.rodata); }
	.data : { * (.data); }

	.dummy ALIGN(4): { LONG(42); }

	.bss ALIGN(4): {
		_bss_start = .;
		* (.bss);
		. = ALIGN(4);
		_bss_end = .;
	}
	_bss_size = SIZEOF(.bss);

	_mem_start = .;
}
