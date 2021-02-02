#include <stdio.h>
#include "serial.h"
#include "mem.h"
#include "amigalib.h"
#include "debug.h"

#define MAGIC		0xa55a
#define STACK_SIZE	4096

#define MEMTYPE_MASK	7
#define MEM_USED		0x8000

struct memrange {
	uint16_t magic;
	uint16_t attr;
	uint32_t size;
	struct memrange *next;
	unsigned char start[];
};

enum {
	POOL_FAST,
	POOL_SLOW,
	POOL_CHIP,
	NUM_POOLS
};

static struct memrange *pool[NUM_POOLS];
static uint32_t stack_base, stack_top;

extern int _mem_start;

void move_init_stack(uint32_t newtop);	/* in startup.s */

static void add_range(int pidx, struct memrange *mr);


int init_mem(void)
{
	int i;
	struct memrange *mr, *smr;
	struct alib_memnode *mem;
	uint32_t mem_start = (uint32_t)&_mem_start;

	printf("mem_start: %06lx\n", mem_start);

	printf("Memory ranges:\n");
	mem = execbase->memlist.head;
	while(mem->n_next) {
		unsigned long start, end, size;
		char *stype;
		int type, pidx;

		/* assume we're dealing with at least 64k blocks */
		start = (unsigned long)mem->start & 0xff0000;
		end = ((unsigned long)mem->end + 0xffff) & 0xff0000;
		size = end - start;

		if(mem->attrib & ALIB_MEMF_CHIP) {
			stype = "chip";
			type = MEM_CHIP;
			pidx = POOL_CHIP;
		} else if(mem->attrib & ALIB_MEMF_FAST) {
			if(start >= 0xc00000 && end <= 0xd80000) {
				stype = "slow";
				type = MEM_SLOW;
				pidx = POOL_SLOW;
			} else {
				stype = "fast";
				type = MEM_FAST;
				pidx = POOL_FAST;
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
		add_range(pidx, mr);
	}

	/* allocate stack space at the top of a memory range, order of preference:
	 * fast, slow, chip
	 */
	smr = 0;
	for(i=0; i<NUM_POOLS; i++) {
		mr = pool[i];

		/* ranges in the list are sorted, so keep updating the potential stack
		 * allocation until we reach the end of the list, and use the last one
		 * we found that fits. Note: at this point it's not expected that any
		 * one of the lists will have more than a single range, but this should
		 * work regardless.
		 */
		while(mr) {
			if(mr->size >= STACK_SIZE) {
				stack_top = (uint32_t)mr->start + mr->size;
				stack_base = stack_top - STACK_SIZE;
				smr = mr;
			}
			mr = mr->next;
		}

		if(smr) {
			smr->size -= STACK_SIZE;
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

#define MIN_MEMRANGE	8

void *mem_alloc(unsigned int sz, unsigned int attr)
{
	int i;
	struct memrange *mr, *prev, dummy;
	struct memrange *mem = 0;

	if(!(attr & MEM_ANY)) {
		attr |= MEM_ANY;
	}

	for(i=0; i<NUM_POOLS; i++) {
		if(!(attr & (1 << i))) continue;

		dummy.next = pool[i];
		prev = &dummy;
		mr = dummy.next;
		while(mr) {
			if(mr->size >= sz) {
				if(mr->size >= sz + sizeof *mr + MIN_MEMRANGE) {
					/* we have enough space for a leftover piece */
					mem = mr;

					mr = (struct memrange*)(mem->start + sz);
					*mr = *mem;
					mr->size -= sz + sizeof *mr;
					prev->next = mr;

					mem->size = sz;
					mem->attr |= MEM_USED;
					mem->next = 0;
				} else {
					/* not enough leftover space, just allocate the whole range */
					mem = mr;
					mr->attr |= MEM_USED;
					prev->next = mr->next;
					mr->next = 0;
				}
				break;
			}
			prev = mr;
			mr = mr->next;
		}
		pool[i] = dummy.next;

		if(mem) {
			return mem->start;
		}
	}

	return 0;
}

void mem_free(void *ptr)
{
	struct memrange *mr = (struct memrange*)ptr - 1;
	int pidx = 0;

	if(mr->magic != MAGIC || !(mr->attr & MEM_ANY)) {
		goto fail;
	}

	if(mr->attr & MEM_FAST) {
		pidx = 0;
	} else if(mr->attr & MEM_SLOW) {
		pidx = 1;
	} else if(mr->attr & MEM_CHIP) {
		pidx = 2;
	} else {
		goto fail;
	}

	add_range(pidx, mr);
	return;

fail:
	printf("mem_free(%p): invalid pointer or magic corrupted\n", ptr);
	memdump(mr, sizeof *mr);
	panic("");
}

static void add_range(int pidx, struct memrange *mr)
{
	struct memrange *prev, *next, dummy;

	printf("DBG adding free range: %06lx - %06lx to pool %d\n", (unsigned long)mr,
			(unsigned long)(mr->start + mr->size - (unsigned char*)mr), pidx);

	dummy.next = pool[pidx];
	prev = &dummy;

	while(prev->next && prev->next < mr) {
		prev = prev->next;
	}

	mr->next = prev->next;
	prev->next = mr;

	/* coalesce */
	if((next = mr->next) && mr->start + mr->size >= (unsigned char*)mr->next) {
		mr->size = next->start + next->size - mr->start;
		mr->next = next->next;
		next->magic = 0;
		printf("  DBG coalescing up\n");
	}
	if(prev != &dummy && prev->start + prev->size >= (unsigned char*)mr) {
		prev->size = mr->start + mr->size - prev->start;
		prev->next = mr->next;
		mr->magic = 0;
		printf("  DBG coalescing down\n");
	}

	pool[pidx] = dummy.next;
}
