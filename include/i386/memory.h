#ifndef __I386_MM
#define __I386_MM

#include <i386/types.h>



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
