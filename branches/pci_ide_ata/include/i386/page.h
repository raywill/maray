#ifndef __I386_PAGE
#define __I386_PAGE

#include <i386/types.h>


#define PAGE_OFFSET 0xC0000000
#define PAGE_SIZE 0x1000
#define PAGE_SHIFT 12

typedef struct page{
	int32_t ref_count;	// reference count
	uint32_t index;		// page index
	uint32_t virtual;	// where this page maps to
	uint32_t* padding;
}page_t;


uint32_t _v2p(uint32_t vaddr);
uint32_t _p2v(uint32_t paddr);
void set_pd(uint32_t* pd_base, uint32_t index, uint32_t val);
void set_pt(uint32_t* pt_base, uint32_t index, uint32_t val);
void unset_pd(uint32_t* pd_base, uint32_t index);
void unset_pt(uint32_t* pt_base, uint32_t index);
void update_paging();

page_t* alloc_page();
void init_all_pages();
void init_page_tables();


/*
 * following TLB functions is migrated from  linux-2.2.26
 * TLB flushing:
 *
 *  - flush_tlb() flushes the current mm struct TLBs
 *  - flush_tlb_one flush only one page, only supported by i486+.
 *
 */

#define flush_tlb() \
do { unsigned long tmpreg; __asm__ __volatile__("movl %%cr3,%0\n\tmovl %0,%%cr3":"=r" (tmpreg) : :"memory"); } while (0)

#ifndef CONFIG_X86_INVLPG
#define flush_tlb_one(addr) flush_tlb()
#else
#define flush_tlb_one(addr) \
__asm__ __volatile__("invlpg %0": :"m" (*(char *) addr))
#endif
/* end of tlb functions */

#endif
