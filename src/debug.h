#ifndef DEBUG_H_
#define DEBUG_H_

void memdump(void *ptr, int len);

void panic(const char *fmt, ...);	/* defined in amiga/startup.s */

#endif	/* DEBUG_H_ */
