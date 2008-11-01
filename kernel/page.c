/*
	Name:	page.c
	Copyright: GPL
	Author: Raywill
	Date: 22-12-06
	Description: 
*/

#include <global.h>
#include <libc.h>
#include <stdio.h>

#include <i386/types.h>
#include <i386/memory.h>
#include <i386/page.h>

#define NULL_PAGE_LIST	(PAGE_LIST *)0;
#define NULL_PAGE	(PAGE *)0;
#define PAGE_STRUCT_SIZE (sizeof(page_t))

/* extern in asm */
int set_page_directory(uint32_t pte);

/*  extern  variable in asm  */
extern uint32_t* temp_kernel_page_table;
extern uint32_t* kernel_page_table;
extern uint32_t* page_directory;

#define max_page_count (32*1024) /* max 128M physical memory */

/* Note: Common page allocation starts from res_page_count<<PAGE_SHIFT
 * Low 4M ~ 8M is reserved for bootmem.
 * *Initial* PageTableEntry and PageTableDirectory are resident at this area.
 */

/* reserve low 8M memory */
#define res_page_count  (2*1024) 

/* within low 8M memory, reserve 0~4M for kernel */
#define res_kernel_page_count  (1*1024)
/* reserve 4M~8M area for page table (as page table pool). */
/* here, we make page table grow down from 8M space */
#define kptd_page_count  1
#define kpte_page_count  (max_page_count>>10)
/* ptd is lower than pte here */
#define kptd_page_base ((uint32_t)((res_page_count - kptd_page_count - kpte_page_count)<<12))
#define kpte_page_base ((uint32_t)((res_page_count - kpte_page_count)<<12))

/* we have mapped 4*4M=16M physical memory from the start of RAM. Should enough */
#define page_base  (res_page_count<<12) /* grow up from 8M memory */
#define page_pool_size  ((max_page_count * PAGE_STRUCT_SIZE)/4096) /* NOTE: 32 pages, not in bytes */

page_t* mem_map = (page_t*)page_base;



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

	pd_base[index] = (val & 0xFFFFF000) | 0x07;	/* User&Supervisor, R&W, Present */
}


void set_pt(uint32_t* pt_base, uint32_t index, uint32_t val)
{
	pt_base[index] = (val & 0xFFFFF000) | 0x07;	/* User&Supervisor, R&W, Present */
}

void unset_pd(uint32_t* pd_base, uint32_t index)
{
	pd_base[index] = 0;
}


void unset_pt(uint32_t* pt_base, uint32_t index)
{
	pt_base[index] = 0;
}


/* map physical page to virtual space */
uint32_t map_page(page_t* pg)
{
	uint32_t* pt_base = (uint32_t*)(kpte_page_base + PAGE_OFFSET);
	set_pt(pt_base, pg->index, pg->index<<PAGE_SHIFT);
	pg->virtual = _p2v(pg->index<<PAGE_SHIFT);
	return pg->virtual;
}


void unmap_first_dir_entry()
{
/*	uint32_t ptd = kptd_page_base;
	uint32_t* p;
	p = (uint32_t*)(ptd + 0 * 4);
	*p = 0;
	flush_tlb();
*/
//	unset_pd((uint32_t*)kptd_page_base, 0);
}
/* after new page framework has been setup
 * we will update paging using this function
 */
void update_paging()
{

	uint32_t pte = kpte_page_base;
	uint32_t cr0 = 0;
	uint32_t* p;
	uint32_t ptd = kptd_page_base;
	uint32_t* bad_addr = 0x00000;
	

	kprintf("pte = %x ",pte);	
	kprintf("ptd = %x\n",ptd);	
	set_page_directory(ptd);
	
#ifdef __KERNEL_DEBUG__
	{
	uint32_t* p1;
	uint32_t* p2;
	p1 = (uint32_t*)0xc07e0000;
	p2 = (uint32_t*)0x5000;	
	memory_compare(p1,p2,10);
	}
#endif
	/* This action still cause bugs, including
	 * IDT, GDT, TSS etc.
	 * All their base values needs adjust
	 **/
	unmap_first_dir_entry();
}


/* incomplete!!! */
#if 1
void* vm_alloc_zeroed_page()
{
	page_t* pg = NULL;
	void* vaddr = NULL;

	/* allocate a page */
	pg = alloc_page();

	/* map the page */
	vaddr = (void*)map_page(pg);	/* map physical page to virtual space */
	
	/* zero the page and return */
	memset(vaddr,0, PAGE_SIZE);

	return vaddr;
}

void* vm_alloc_page()
{
	page_t* pg = NULL;
	void* vaddr = NULL;

	/* allocate a page */
	pg = alloc_page();

	/* map the page */
	vaddr =(void*)map_page(pg);	/* map physical page to virtual space */

	return vaddr;
}

#endif

/* Allocate a free page.
 *
 * Note that the allocated page isn't yet mapped.
 * The user of this function is expected to do the mapping
 * before using this page.
 */
page_t* alloc_page()
{
	int i=0;
	page_t* pg = NULL;
	
	/* search in the page pool and find a free page */
	// put your code here
	
	/**/
	for (i = res_page_count + page_pool_size; i < max_page_count; i++)
	{
		pg = (page_t*)(_p2v(page_base) + i*PAGE_STRUCT_SIZE);
		if (0 == pg->ref_count)
		{
			break;	/* got a free page */
		}
	}
	pg->ref_count++;
	return pg;	
}

/* Initialize all physical pages
 * This function should only run once at startup time
 */
void init_all_pages()
{
	int i;
	page_t* pg = NULL;

	/* All pages are initialized here
	 * However, they won't function until
	 * the address of real pages is filled 
	 * in page table entrys
	 */
	
	uint32_t* pt;
#if 0
	/* follow two loops are only for debug purpose */

	pt =(uint32_t*)( PAGE_OFFSET + 0x4000);
	for(i=0;i<1024;i++)
	{
		if (*pt != 0)
			kprintf("ptd[%d]=%x ",i,*pt);
		pt++;
	}
	
	kprintf("\n");

	pt = (uint32_t*)(PAGE_OFFSET + 0x5000);
	for(i=0;i<1024;i++)
	{
		if (i%256 == 0)
			kprintf("pte[%d]=%x ",i,*pt);
		pt++;
	}

	kprintf("\n");
#endif
	kprintf("init_all_pages()->\n");
	kprintf("\tinit reserved pages\n");
	/* Initialize reserved pages */
	for (i = 0; i < res_page_count + page_pool_size ; i++)
	{
		pg = (page_t*)(_p2v(page_base) + i*PAGE_STRUCT_SIZE);
		pg->ref_count = 1;
		pg->index = i;
		pg->virtual = PAGE_OFFSET + (i << PAGE_SHIFT) ;
	}

	kprintf("\tinit free pages\n");
	/* initialize free pages */
	for (i = res_page_count + page_pool_size; i < max_page_count; i++)
	{
		pg = (page_t*)(_p2v(page_base) + i*PAGE_STRUCT_SIZE);
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
	uint32_t pte = PAGE_OFFSET + kpte_page_base;	/* page entry */
	uint32_t ptd = PAGE_OFFSET + kptd_page_base;  /* page directory */
	uint32_t pt = 0;
	uint32_t* p = NULL;

	int i = 0, j = 0, k = 0;
	int dir_cnt = 0; /* directory entrys needs to be set */
	int entry_cnt = 0; /* page entrys needs to be set */
	int total_entry_cnt = 0; /*all page entry count */
	int index = 0;	/* ptd array index */

	kprintf("init_page_tables()->\n");

	/* fill pte */
	memset((void*)pte, 13, kpte_page_count * PAGE_SIZE);

	kprintf("\t pte clear, Page Table Starts At %x\n",pte);
	/* only init low 8M and the page_t mem_map array */
	entry_cnt = (res_page_count + page_pool_size);
	total_entry_cnt = max_page_count;
	for (i = 0; i < entry_cnt; i++)
	{
		set_pt((uint32_t*)pte, i, i<<PAGE_SHIFT);
	}
	for(;i < total_entry_cnt; i++)
	{
		unset_pt((uint32_t*)pte,i);
	}
	


	/* fill ptd */
	kprintf("\tfill ptd, Page Table Directory Starts At %x\n",ptd);
	memset((void*)ptd,0,PAGE_SIZE);


	/******Special NOTE!**********/
	/* Identity mapping still needed!
	 * FUCK INTEL!
	 */
#if 0
	p = (uint32_t*)(ptd + 0 * 4);
	*p = (pte - PAGE_OFFSET) & (~0xFFF)  | 0x07;	/* write *physical* address* to ptd. */
	p = (uint32_t*)(ptd + 1 * 4);
	*p = (pte - PAGE_OFFSET + PAGE_SIZE) & (~0xFFF)  | 0x07;	/* write *physical* address* to ptd. */
#endif
	
	pt = kpte_page_base;	/* note: physical address! */
	/* identify mapping first 16M */
	for (i = 0; i < 4 ; i++)
	{
		set_pd((uint32_t*)ptd, i, pt);
		pt += PAGE_SIZE;
	}

	/**** end identity mapping ***/
	
	
	//dir_cnt = (res_page_count + page_pool_size + 1023) >> 10;
	/* Map all directory entrys above 3G! 
	 * However, page table entrys not mapped yet
	 * so that all invalid access to pages can still be captured
	 */
	dir_cnt = max_page_count >> 10;
	index = (uint32_t)PAGE_OFFSET>>22;
	pt = kpte_page_base;
	/* map our pages to 3G space */
	for (i = index; i < index + dir_cnt ; i++)
	{
		set_pd((uint32_t*)ptd, i, pt);
		pt += PAGE_SIZE;
	}

	kprintf("end of init\n");

	/* Conclution:
	 *   'start' tell us where our initial page table locates.
	 *   In order to init these tables, we should have a temporal
	 *   page table. We DO have such table.
	 *
	 *   Note that 'page pool' is not resident at res_page_count area
	 *   we should also map it in initialization code!!!
	 *
	 *   How much should we map?
	 *   1. first 8M (2 page table)
	 *   2. page pool 128k (1 page table)
	 *
	 *   Where should we map to?
	 *   1. phy + PAGE_OFFSET
	 */
}

