#include <string.h>
#include <ctype.h>

void memset(void *s, int c, size_t n)
{
	char *ptr = s;
	while(n--) {
		*ptr++ = c;
	}
}

void *memcpy(void *dest, const void *src, size_t n)
{
	char *dptr = dest;
	const char *sptr = src;

	while(n--) {
		*dptr++ = *sptr++;
	}
	return dest;
}

void *memmove(void *dest, const void *src, size_t n)
{
	int i;
	char *dptr;
	const char *sptr;

	if(dest <= src) {
		/* forward copy */
		dptr = dest;
		sptr = src;
		for(i=0; i<n; i++) {
			*dptr++ = *sptr++;
		}
	} else {
		/* backwards copy */
		dptr = (char*)dest + n - 1;
		sptr = (char*)src + n - 1;
		for(i=0; i<n; i++) {
			*dptr-- = *sptr--;
		}
	}

	return dest;
}

size_t strlen(const char *s)
{
	size_t len = 0;
	while(*s++) len++;
	return len;
}

char *strchr(const char *s, int c)
{
	while(*s) {
		if(*s == c) {
			return (char*)s;
		}
		s++;
	}
	return 0;
}

char *strrchr(const char *s, int c)
{
	const char *ptr = s;

	/* find the end */
	while(*ptr) ptr++;

	/* go back checking for c */
	while(--ptr >= s) {
		if(*ptr == c) {
			return (char*)ptr;
		}
	}
	return 0;
}

char *strstr(const char *str, const char *substr)
{
	while(*str) {
		const char *s1 = str;
		const char *s2 = substr;

		while(*s1 && *s1 == *s2) {
			s1++;
			s2++;
		}
		if(!*s2) {
			return (char*)str;
		}
		str++;
	}
	return 0;
}

int strcmp(const char *s1, const char *s2)
{
	while(*s1 && *s1 == *s2) {
		s1++;
		s2++;
	}
	return *s1 - *s2;
}

int strcasecmp(const char *s1, const char *s2)
{
	while(*s1 && tolower(*s1) == tolower(*s2)) {
		s1++;
		s2++;
	}
	return tolower(*s1) - tolower(*s2);
}
