/*
	Name:	memory.c
	Copyright: GPL
	Author: Raywill
	Date: 22-12-06
	Description: Physical Memory manager which Manage the whole RAM
*/


/*
*Phy Memory Layout:
* 0x00000 - 0x19000(100k) 		------Kernel(.text,initial data struct)
* 0x19000 - 0x20000(128k) 		------Reserved for kernel
* 0x20000 - 0x*****(unlimited)	 ------User space
* We share a same page table currently.
**/


#include <i386/memory.h>
#include <global.h>

#define NR_HOLES 1024
//#define NR_HOLES 20		//For testing


struct hole holes[NR_HOLES];
struct hole *free_hole_list=NULL;	// regist free hole in this list
struct hole *hole_head;			//the logical head of the holes
struct hole *hole_rear;			//the logical rear of the holes


/*
Data Structure about this Memoy Manager:
Two lists are maintained:
Busy list : It records current free space
Free list : Empty holes, with no memory infomation in it .
	    When the user returned a block of memory to MM , we need a hole from this FreeList.
		
Busy list is a bidirection list
Free list is a monodirection list . Because bidirection is not needed :-)
*/
static void init_hole(struct hole *holes);
static void del_hole(struct hole *h);
static void merge(struct hole *h);
static void put_free_hole(struct hole *hole);
static struct hole *get_free_hole();


void init_mm()
{
	init_hole(holes);

}

void init_hole(struct hole *holes)
{
	struct hole *h;
	int i;
	
	/*     H[NR_HOLES-1]<-H[0]<->H[1]<->......<->H[NR_HOLES-1]->NULL	*/
	for(i=1;i<NR_HOLES;i++){
		holes[i].h_next=&holes[i+1];
	}
	holes[NR_HOLES-1].h_next=NULL;		// holes[i-1]! NOT holes[i]  .   This is fixed by OSTestingProject.
	free_hole_list=&holes[1];			// insert the holes after hole[1] into free_hole list!
										// Remark: You'd better not use put_free_hole() in this function.
	// adjust front and tail pointer
	holes[0].h_prev = holes[0].h_next = &holes[0];

	holes[0].size=0xA00000; 	//	=10M
	holes[0].base=0xC0019000;	// Assume we totally have 10M phy memory :-) Just for experimental  purpose	
								// 19000 = 100K ,reserved for the .text seg of kernel itself
								// this 100k will never be freed
	hole_head=&holes[0];
	hole_rear=hole_head->h_prev;
}

/*
Search from the beginning of the free space list
Use FIRST_FIT_ALGROTHIM
*/
void * kmalloc(unsigned int  size){
	struct hole *h=hole_head;
	struct hole *front=h;
	struct hole *h_prev=NULL;
	unsigned int *free_ptr=NULL;
	int found = 0;
	
	do{
		if(size <= h->size){
			free_ptr=(unsigned int *)h->base;
			h->size-=size;
			h->base+=size;		// alloc from the front , would be faster if from the tail. :-)  Learned this in OS class
			found = 1;
			break;
		}
		h=h->h_next;
	}while(h!=front);	// If h=front , search have reached to the end.
	
	if(h==front && found==0){
		//	panic();
		return NULL;
	}
	
	if(h->size==0)
		del_hole(h);		// whole free space malloced.
	return (void*)(free_ptr); 

 /*
	static int mem=0xC0019000;
	mem+=0x1000;
	return mem;
	*/

}


static void del_hole(struct hole *h)
{
	struct hole *prev = h->h_prev;
	struct hole *temp = h;

	// delete h from current link
	if(h==hole_head){
		hole_head=h->h_next;
	}else if(h==hole_rear){
		hole_rear=h->h_prev;
	}
	prev->h_next = h->h_next;
	h->h_next->h_prev = prev;
	
	// link h to the free_hole_list
	put_free_hole(h);

	// printf("h=%p\n",h);

}


void kfree(void *free_ptr,unsigned int size)
{
	struct hole *h=hole_head;
	struct hole *front=h;
	struct hole *new;
	int found = 0;
	
	// Insert this free block to the free memory list . Need to obtain a free hole from the free_hole_list
	new = get_free_hole();	// Get a free hole
	if(new == NULL ){
		kprint("Not enough free memory holes!");
		//panic();
	}
	// fill the hole with returned space info
	new->base = (unsigned int)free_ptr;
	new->size = size;
	
	if((unsigned int)free_ptr < h->base) hole_head = new;
	
	// Search for a insertion point
	// Note: KEEP 'BASE'  IN SEQUENCE .
	while((unsigned int)free_ptr > h->base) h=h->h_next;

	new->h_next = h;
	new->h_prev = h->h_prev;
	h->h_prev->h_next = new;
	h->h_prev = new;

	if(hole_rear == h) hole_rear = new;
	
	// We are almost done!
	// BUT, Rember to merge 'neighbour' holes 
	if((unsigned int)free_ptr == new->h_prev->base + new->h_prev->size)
		merge(new->h_prev);
	else if((unsigned int)free_ptr + size == new->h_next->base)
		merge(new);
		
}
/*
Meger concecutive holes :-) Recursively!
About parameters:
	merge(h) acturally means merge(h,h->next),
	instead of merge(h,h->prev)

*/
void merge(struct hole *h)
{
	struct hole *hnext;
	hnext=h->h_next;
	// reset hole size
	h->size+=hnext->size;
	h->h_next=hnext->h_next;
	hnext->h_next->h_prev=h;
	// adjust pointer
	if(hnext==hole_rear) hole_rear=h;
	if(hnext==hole_head) hole_head=hnext->h_next;
	
	// put the new hole to the free_list
	put_free_hole(hnext);
	
	// Can we merge more?
	if((unsigned int)h->base+h->size == (unsigned int)h->h_next->base)
		merge(h);

}

// get a hole from free list
struct hole * get_free_hole()
{
	struct hole *fhole;
	if(!free_hole_list){
		kprint("No free slots!");
		//	panic();
		//	return;
	}
	fhole=free_hole_list;
	free_hole_list = free_hole_list->h_next;
	return fhole;
	
}


// put a hole into free list
static void put_free_hole(struct hole *hole)
{
	hole->h_next=free_hole_list;
	free_hole_list=hole;
}
