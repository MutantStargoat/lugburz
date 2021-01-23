#ifndef SERIAL_H_
#define SERIAL_H_

#include "hwregs.h"

/* dff030 is REG_SERDAT
 * dff018 is REG_SERDATR
 * bit 13 of SERDATR is TBE (transmit buffer empty)
 */
#define ser_putchar(c) \
	asm volatile( \
		"or.w #0x100, %0\n\t" \
		"0: btst #13, 0xdff018\n\t" \
		"beq 0b\n\t" \
		"move.w %0, 0xdff030\n\t" \
		:: "d"((int16_t)c))

void ser_init(int baud);
void ser_print(const char *s);

#endif	/* SERIAL_H_ */
