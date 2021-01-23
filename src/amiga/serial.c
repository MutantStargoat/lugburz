#include "hwregs.h"
#include "serial.h"

#define CLK		3546895
#define BVAL(b)	(CLK / (b) - 1)

static inline uint16_t baudval(int baud)
{
	switch(baud) {
	case 110: return BVAL(110);
	case 300: return BVAL(300);
	case 600: return BVAL(600);
	case 1200: return BVAL(1200);
	case 2400: return BVAL(2400);
	case 4800: return BVAL(4800);
	case 9600: return BVAL(9600);
	case 14400: return BVAL(14400);
	case 19200: return BVAL(19200);
	case 38400: return BVAL(38400);
	case 57600: return BVAL(57600);
	case 115200: return BVAL(115200);
	default:
		break;
	}
	return BVAL(baud);
}

void ser_init(int baud)
{
	REG_SERPER = baudval(baud) & 0x7fff;
}

/*
void ser_putchar(int c)
{
	REG_SERDAT = ((uint16_t)c & 0xff) | 0x100;
}
*/

void ser_print(const char *s)
{
	while(*s) {
		if(*s == '\n') {
			ser_putchar('\r');
		}
		ser_putchar(*s++);
	}
}
