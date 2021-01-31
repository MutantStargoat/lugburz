#include <stdio.h>
#include <ctype.h>
#include "debug.h"

void memdump(void *ptr, int len)
{
	int i;
	unsigned char *p = ptr;

	while(len > 0) {
		printf("%06lx ", (unsigned long)p);
		for(i=0; i<16; i++) {
			if(len - i <= 0) {
				printf("   ");
			} else {
				printf(" %02x", (unsigned int)p[i]);
			}
			if((i & 7) == 7) putchar(' ');
		}
		printf(" |");
		for(i=0; i<16; i++) {
			if(len - i <= 0) {
				putchar(' ');
			} else {
				putchar(isprint(p[i]) ? p[i] : '.');
			}
		}
		printf("|\n");
		len -= 16;
		p += 16;
	}
}
