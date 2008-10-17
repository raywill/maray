#ifndef __I386_MM
#define __I386_MM

#include <i386/types.h>

#define PAGE_OFFSET 0xC0000000
#define PAGE_SIZE 0x1000
#define PAGE_SHIFT 12

typedef struct page{
	int32_t ref_count;		// reference count
	uint32_t index;		// page index
	uint32_t* virtual;	// where this page maps to
	uint32_t* padding;
}page_t;


void init_mm();
void kfree(void *free_ptr,unsigned size);
void * kmalloc(unsigned size);




struct hole {
	unsigned int base;
	unsigned int size;
	struct hole* h_next;
	struct hole* h_prev;
};
	
#endif
