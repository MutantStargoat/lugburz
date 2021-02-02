#include <stdlib.h>
#include <string.h>
#include "mem.h"

void *malloc(size_t sz)
{
	return mem_alloc(sz, 0);
}

void *calloc(size_t n, size_t sz)
{
	void *p = mem_alloc(n * sz, 0);
	if(!p) return 0;
	memset(p, 0, n * sz);
	return p;
}

void free(void *p)
{
	mem_free(p);
}
