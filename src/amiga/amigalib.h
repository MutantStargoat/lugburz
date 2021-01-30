/* structures and functions necessary to access amiga ROM services */
#ifndef AMIGALIB_H_
#define AMIGALIB_H_

#include <stdint.h>

#define ALIB_MEMF_ANY		0L
#define ALIB_MEMF_PUBLIC	0x0001L
#define ALIB_MEMF_CHIP		0x0002L
#define ALIB_MEMF_FAST		0x0004L
#define ALIB_MEMF_CLEAR		0x0100L
#define ALIB_MEMF_LARGEST	0x0200L
#define ALIB_MEMF_REVERSE	0x0400L
#define ALIB_MEMF_TOTAL		0x0800L

#define ALIB_NODE_COMMON(NODE_TYPE) \
	NODE_TYPE *n_next; \
	NODE_TYPE *n_prev; \
	uint8_t n_type; \
	int8_t n_prio; \
	char *n_name

struct alib_memchunk {
	struct alib_memchunk *next;
	uint32_t size;
};

struct alib_memnode {
	ALIB_NODE_COMMON(struct alib_memnode);
	uint16_t attrib;
	struct alib_memchunk *freelist;
	void *start, *end;
	uint32_t freesz;
};

struct alib_memlist {
	struct alib_memnode *head, *tail, *tailpred;
	uint8_t type;
	uint8_t pad;
};

struct alib_library {
	ALIB_NODE_COMMON(struct alib_library);
	uint8_t flags;
	uint8_t pad;
	uint16_t negsz, possz;
	uint16_t ver_major, ver_minor;
	char *idstr;
	uint32_t csum;
	uint16_t nref;
};

struct alib_intvec {
	void *data;
	void (*code)();
	ALIB_NODE_COMMON(struct alib_intvec);
};

struct alib_execbase {
	struct alib_library lib;
	uint16_t softver;
	int16_t lowmem_csum;
	uint32_t chkbase;
	void *coldcap_vect, *coolcap_vect, *warmcap_vect;
	void *sysstack_upper, *sysstack_lower;
	uint32_t chipmem_top;
	void *dbg_entry, *dbg_data, *alert_data;
	void *extmem_top;
	uint16_t csum;

	struct alib_intvec intvec[16];

	void *curtask;
	uint32_t idle_count, disp_count;
	uint16_t tmslice, nticks;
	uint16_t sysflags;
	int8_t intr_dis_nest, task_dis_nest;
	uint16_t attn_flags, attn_resched;
	void *resmod;
	void *tasktrap, *taskexcept, *taks_exit;
	uint32_t task_sig_alloc;
	uint16_t task_trap_alloc;

	struct alib_memlist memlist;
	/* ... more ... */
};

struct alib_execbase *execbase;

int alib_init(void);

#endif	/* AMIGALIB_H_ */
