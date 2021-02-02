#ifndef MEM_H_
#define MEM_H_

enum {
	MEM_FAST	= 1,
	MEM_SLOW	= 2,
	MEM_CHIP	= 4,
	MEM_ANY		= MEM_FAST | MEM_SLOW | MEM_CHIP
};

int init_mem(void);

void *mem_alloc(unsigned int sz, unsigned int attr);
void mem_free(void *ptr);

#endif	/* MEM_H_ */
