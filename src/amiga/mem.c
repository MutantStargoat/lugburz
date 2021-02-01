#include <stdio.h>
#include "serial.h"
#include "mem.h"
#include "amigalib.h"
#include "debug.h"

#define MAGIC	0xa55a
#define STACK_SIZE	4096

enum {
	MEM_FAST,
	MEM_SLOW,
	MEM_CHIP,
	MEM_USED	= 0x8000
};

struct memrange {
	uint16_t magic;
	uint16_t attr;
	uint32_t size;
	struct memrange *next;
	unsigned char start[];
};

#define NUM_POOLS	3
static struct memrange *pool[NUM_POOLS];
static uint32_t stack_base, stack_top;

extern int _mem_start;

void move_init_stack(uint32_t newtop);	/* in startup.s */

int init_mem(void)
{
	int i;
	struct memrange *mr;
	struct alib_memnode *mem;
	uint32_t mem_start = (uint32_t)&_mem_start;

	printf("mem_start: %lu\n", mem_start);

	printf("Memory ranges:\n");
	mem = execbase->memlist.head;
	while(mem->n_next) {
		unsigned long start, end, size;
		char *stype;
		int type;

		/* assume we're dealing with at least 64k blocks */
		start = (unsigned long)mem->start & 0xff0000;
		end = ((unsigned long)mem->end + 0xffff) & 0xff0000;
		size = end - start;

		if(mem->attrib & ALIB_MEMF_CHIP) {
			stype = "chip";
			type = MEM_CHIP;
		} else if(mem->attrib & ALIB_MEMF_FAST) {
			if(start >= 0xc00000 && end <= 0xd80000) {
				stype = "slow";
				type = MEM_SLOW;
			} else {
				stype = "fast";
				type = MEM_FAST;
			}
		} else {
			printf("ignoring memory range %06lx - %06lx of unknown type: %u\n",
					start, end, mem->attrib);
			mem = mem->n_next;
			continue;
		}

		printf(" %06lx - %06lx: %s (%ldk)\n", start, end, stype, size >> 10);
		mem = mem->n_next;

		if(mem_start >= start && mem_start < end) {
			size -= mem_start - start;
			start = mem_start;
		}

		mr = (struct memrange*)start;
		mr->magic = MAGIC;
		mr->attr = type;
		mr->size = size - sizeof *mr;
		mr->next = pool[type];
		pool[type] = mr;
	}

	/* allocate stack space at the top of a memory range, order of preference:
	 * fast, slow, chip
	 */
	for(i=0; i<NUM_POOLS; i++) {
		mr = pool[i];
		if(mr && mr->size >= STACK_SIZE) {
			stack_top = (uint32_t)mr->start + mr->size;
			stack_base = stack_top - STACK_SIZE;
			mr->size -= STACK_SIZE;
			break;
		}
	}

	if(!stack_base) {
		panic("Failed to allocate stack, no suitable memory ranges found!\n");
	}

	printf("Allocated %dk stack space: %06lx - %06lx\n", STACK_SIZE >> 10, stack_base, stack_top);
	move_init_stack(stack_top);

	return 0;
}
