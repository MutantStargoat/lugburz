#ifndef AMIGA_LIBC_STDLIB_H_
#define AMIGA_LIBC_STDLIB_H_

#include <stdint.h>

typedef int32_t ssize_t;
typedef uint32_t size_t;

int atoi(const char *str);
long atol(const char *str);
long strtol(const char *str, char **endp, int base);

void itoa(int val, char *buf, int base);
void utoa(unsigned int val, char *buf, int base);

void *malloc(size_t sz);
void *calloc(size_t n, size_t sz);
void free(void *p);

#endif	/* AMIGA_LIBC_STDLIB_H_ */
