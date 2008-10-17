/*
	Name:	page.c
	Copyright: GPL
	Author: Raywill
	Date: 22-12-06
	Description: ????
*/



/*		思考方式：我们只有一个进程！他拥有整个内存空间		*/

#include <i386/types.h>
#include <i386/memory.h>
	

#define NULL_PAGE_LIST	(PAGE_LIST *)0;
#define NULL_PAGE	(PAGE *)0;


const uint32_t max_page_count = 32*1024; /* max 128M physical memory */

/* Note: Common page allocation starts from res_page_count<<PAGE_SHIFT
 * Low 4M ~ 8M is reserved for bootmem.
 * *Initial* PageTableEntry and PageTableDirectory are resident at this area.
 */
const uint32_t res_page_count = 2*1024; /* reserve low 8M memory */
const uint32_t res_kernel_page_count = 1*1024;	/* reserve 4M for kernel */
/* reserve 4M~8M area for page table. */

/* we have mapped 4*4M=16M physical memory from the start of RAM. Should enough */
const uint32_t page_base = res_page_count << 12; /* grow up from 8M memory */
const uint32_t page_pool_size = 32; /* NOTE: 32 pages, not in bytes */


/* convert between virtual address and physical address */
uint32_t _v2p(uint32_t vaddr)
{
	return (vaddr-(uint32_t)PAGE_OFFSET);
}

uint32_t _p2v(uint32_t paddr)
{
	return (paddr+(uint32_t)PAGE_OFFSET);
}


/* operations on page directory and page table */
void set_pd(uint32_t* pd_base, uint32_t index, uint32_t val)
{
	pd_base[index] = val<<12 | 0x07;	/* User&Supervisor, R&W, Present */
}


void set_pt(uint32_t* pt_base, uint32_t index, uint32_t val)
{
	pt_base[index] = val<<12 | 0x07;	/* User&Supervisor, R&W, Present */
}

void unset_pd(uint32_t* pd_base, uint32_t index)
{
	pd_base[index] = 0;
}


void unset_pt(uint32_t* pt_base, uint32_t index)
{
	pt_base[index] = 0;
}

/* incomplete!!! */
void* alloc_zeroed_page()
{
	page_t* pg = NULL;
	uint32_t paddr;
	uint32_t vaddr = NULL;

	/* allocate a page */
	pg = (page_t*)alloc_page();

	/* map the page */
	paddr = pg->index << 12;
	vaddr = _p2v(paddr);	/* default operation in kernel space */
	map_page(paddr, vaddr);	/* map physical page to virtual space */
	
	/* zero the page and return */
	memset(vaddr,0, PAGE_SIZE);
	return vaddr;
}

/* Allocate a free page.
 *
 * Note that the allocated page isn't yet mapped.
 * The user of this function is expected to do the mapping
 * before using this page.
 */
page_t* alloc_page()
{
	page_t* pg = NULL;
	
	/* search in the page pool and find a free page */
	// put your code here
	
	/**/
	for (i = res_page_count + page_pool_size; i < max_page_count; i++)
	{
		pg = (page_t*)(page_base + i<<4);
		if (0 == pg->ref_count)
		{
			break;	/* got a free page */
		}
	}
	return pg;	
}

/* Initialize all physical pages
 * This function should only run once at startup time
 */
void init_all_pages()
{
	int i;
	/* All pages are initialized here
	 * However, they won't function until
	 * the address of real pages is filled 
	 * in page table entrys
	 */

	/* Initialize reserved pages */
	for (i = 0; i < res_page_count + page_pool_size ; i++)
	{
		pg = (page_t*)(page_base + i<<4);
		pg->ref_count = 1;
		pg->index = i;
		pg->virtual = PAGE_OFFSET + i << PAGE_SHIFT;
	}
	/* initialize free pages */
	for (i = res_page_count + page_pool_size; i < max_page_count; i++)
	{
		pg = (page_t*)(page_base + i<<4);
		pg->ref_count = 0;
		pg->index = i;
		pg->virtual = 0;
	}
	return;
}

/* As designed, we put page tables just after 
 * the location of the 'page pool'
 */
void init_page_tables()
{
	uint32_t start = PAGE_OFFSET + res_kernel_page_count << PAGE_SHIFT;
	uint32_t pte;	/* page entry */
	uint32_t ptd;  /* page directory */
	uint32_t* p = NULL;

	int i = 0, j = 0, k = 0;
	int dir_cnt = 0;
	int index = 0;	/* ptd array index */
	
	
	ptd = start;
	pte = start + PAGE_SIZE;
	
	/* fill ptd */
	memset(ptd,0,PAGE_SIZE);

	dir_cnt = res_page_count >> 10;	/* assume that res_page_count mod 1K==0 */
	index = (uint32_t)PAGE_OFFSET>>22;

	for (i = 0; i < dir_cnt ; i++)
	{	
		p = (uint32_t*)(ptd + index*4);
		*p = pte & (~0xFFF)  | 0x07;
		index++;
		pte += PAGE_SIZE;

		/*
		set_pd((uint32_t*)ptd, index, pte);
		index++;
		pte += PAGE_SIZE;
		*/
	}

	
	/* fill pte */
	pte = start + PAGE_SIZE;
	for (i = 0; i < res_page_count; i++)
	{
		p = (uint32_t*)(pte + i * 4);
		*p = (i << PAGE_SHIFT) | 0x07;

		/*
		set_pt((uint32_t*)pte, i, i<<PAGE_SHIFT);
		 */
	}

	/* Conclution:
	 *   'start' tell us where our initial page table locates.
	 *   In order to init these tables. we should have a temporal
	 *   page table. We do have such table.
	 *
	 *   Note that 'page pool' is not resident at res_page_count area
	 *   we should also map it in initial code!!!
	 *
	 *   How much should we map?
	 *   1. first 8M (2 pages)
	 *   2. page pool 128k (1 page)
	 *
	 *   Where should we map to?
	 *   1. phy + PAGE_OFFSET
	 */
}
