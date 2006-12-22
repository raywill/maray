/*
*Phy Memory Layout:
* 0x00000 - 0x19000(100k) 		------Kernel(.text,initial data struct)
* 0x19000 - 0x20000(128k) 		------Reserved for kernel
* 0x20000 - 0x*****(unlimited)	 ------User space
* We share a same page table currently.
**/

#include <i386/vector.h>
#include <i386/memory.h>
#include <global.h>

#define NR_HOLES 1024

extern void page_fault_handler(long error,long address);

struct hole holes[NR_HOLES];
struct hole *free_hole_list;	// regist free hole in this list
struct hole *hole_head;			//the logical head of the holes

static void del_hole(struct hole *h_prev,struct hole *h);


static void init_hole(struct hole *holes);


void init_mm()
{
	init_hole(holes);

}

void init_hole(struct hole *holes)
{
	struct hole *h;
	for(h=&holes[0];h<&holes[NR_HOLES-1];h++)	h->h_next=h+1;
	h->h_next=NULL;
	holes[0].size=0xA00000; //10M
	holes[0].base=0x000000;	// Assume we totally have 10M phy memory :-) Just for experimental  purpose
	free_hole_list=&holes[0];
	hole_head=NULL;
}

const void * kmalloc(int size){
	struct hole *h=free_hole_list;
	struct hole *h_prev=NULL;
	int *free_ptr=NULL;
	while(h){
		if(size <= h->size){
			free_ptr=h->base;
			h->size-=size;
			h->base+=size;
			break;
		}
		h_prev=h;
		h=h->h_next;
	}
	if(h->size==0)
		del_hole(h_prev,h);
 
	return (void*)(free_ptr);
}

// Notice: this version does not recycle deleted holes!
static void del_hole(struct hole *h_prev,struct hole *h)
{
	if(h_prev==NULL){
		free_hole_list=h->h_next;
	}else{
		h_prev->h_next=h->h_next;
	}
}
void kfree(void *free_ptr,int size)
{
	
}
/* vector_t v;
install the mm handler
v.eip = (unsigned)page_fault_handler;
v.access_byte = ACCESS_TRAP_GATE; 	// present, ring 0, '386 interrupt gate D=32bit gate
setvect(&v,14);
//I want to probe the memory.
GetTotalMemory(); */

