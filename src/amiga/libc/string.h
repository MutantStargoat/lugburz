#ifndef AMIGA_LIBC_STRING_H_
#define AMIGA_LIBC_STRING_H_

#include <stdlib.h>

void memset(void *dest, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);

size_t strlen(const char *s);

char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);

char *strstr(const char *str, const char *substr);

int strcmp(const char *s1, const char *s2);
int strcasecmp(const char *s1, const char *s2);

#endif	/* AMIGA_LIBC_STRING_H_ */
