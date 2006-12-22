#ifndef __I386_MM
#define __I386_MM

void init_mm();
void kfree(void *free_ptr,unsigned size);
const void * kmalloc(unsigned size);




struct hole {
	unsigned int base;
	unsigned int size;
	struct hole* h_next;
	struct hole* h_prev;
};
	
#endif
