/*
	Name:	task.c
	Copyright: GPL
	Author: Raywill
	Date: 22-12-06
	Description: Init all tasks.
				 task scheduler also goes here. 
				 I want to move it to somewhere else later.
*/


#include <i386/task.h>
#include <i386/memory.h>
#include <libc.h>
#include <asmcmd.h>
#include <maray/tty.h>

extern void debug_hlt();

void sleep(void);

/*Alloc one page for each task.
 *put task_struct and its kernel-stack in it.
 *
 * ______________
 *|              | 4K
 *|--  Stack   --|
 *|-(Grow down)--|
 *|______________| sizeof(task_tss)
 *|              |
 *|--task_tss----|
 *|---(static)---|
 *|______________| 0(4K aligned)
 */

	#define KERNEL_DATA_GDT_INDEX	0x1
	#define KERNEL_CODE_GDT_INDEX	0x2
	#define USER_DATA_GDT_INDEX		0x3
	#define USER_CODE_GDT_INDEX		0x4 
  
	#define __KERNEL_DS ((unsigned short)(KERNEL_DATA_GDT_INDEX<<3))
	#define __KERNEL_CS ((unsigned short)(KERNEL_CODE_GDT_INDEX<<3))
	#define __USER_DS 	((unsigned short)(3+USER_DATA_GDT_INDEX<<3))
	#define __USER_CS 	((unsigned short)(3+USER_CODE_GDT_INDEX<<3))

	#define FAKE_SEG  (USER_DATA_GDT_INDEX<<3|0x7)  /* LDT,Privilege = 3 */
	#define KERNEL_STACK_OFFSET ((unsigned long)0x100000)
	#define _PAGE_OFFSET ((unsigned long)0xC0000000)
	#define PAGE_SIZE 0x1000

	#define FIRST_TSS_ENTRY 5
	#define FIRST_LDT_ENTRY (FIRST_TSS_ENTRY+1)


	#define INIT_TASK	{0,USER_DATA_GDT_SEL<<3+0,0,0,page_directory,0,0}
	/* first 0-the stack:this stack holds lots things,like eip,cs eflags etc */
	#define TIME_SLICE 0x50

extern ULONG  page_directory;

struct task_struct *current,*last;
void test_task0(void);
void test_task1(void);
void test_task2(void);
void schedule(void);

//struct task_tss tss;
//struct task_struct task_struct_list[3];

struct task_tss tss;
/*={	\
	0,	\
	KERNEL_STACK_OFFSET+PAGE_SIZE,/ *+(ULONG)&task_struct_list[0]* /		\
	USER_DATA_GDT_SEL<<3+0,	\
  0,0,0,0,	\
	page_directory,	0,	0,	\
  0,0,0,0,0,0,0,0,	\
	FAKE_SEG,FAKE_SEG,FAKE_SEG,FAKE_SEG,FAKE_SEG,FAKE_SEG,			\
	FIRST_LDT_ENTRY<<3,\
	0,0xffff																	\
};*/

/* As only 4M mapped,physical memory reference cann't be excessive than that */
/*
static struct task_struct task0;
static struct task_struct task1;
static struct task_struct task2;
*/

static struct task_struct * task0;
static struct task_struct * task1;
static struct task_struct * task2;

/*
struct task_struct *task_struct_list[3]={&task0,&task1,&task2};	
*/
struct task_struct *task_struct_list[3];

	 char buf[20];
	 char *s;

unsigned long task_list[]={	\
	(unsigned long)test_task0,(unsigned long)test_task1,(unsigned long)test_task2	\
};




void test_task0(void){	
	int i=0,j=1;
	char buf[20];
//	clrscr();
//	clrscr();
	print("Hello from task 0\n");

//	debug_halt();
	print("Again from 0\n");
//	print(itoa(i,buf) );
	while(1){
		;
	}

}
void test_task1(void){

	static int i=1,j=1;
	char buf[20];
	clrscr();
	print("You see this again.Opooooooooooooooooooooooooooooooos\n");
	while(1){
		//clrscr();
		//print(itoa(i++,buf,16) );
		//i++;
		print("Hello from task 1\n");
		sleep();
		//debug_halt();

		//	schedule();
	}
	
/*		debug_hlt();
		while(j++<10000000);
	print(itoa(i,buf) );*/
	while(1);
}

void test_task2(void){
	int i=2,j=1;
	char buf[20];
	clrscr();
	while(1){
		//clrscr();
		print("Hello from task 2\n");
		sleep();
//		debug_halt();
		//schedule();
	}
//
	while(1);
}
#define TASK_COUNT 3

void init_all_tasks()
{
	int t;

	tss.esp0=KERNEL_STACK_OFFSET+0*PAGE_SIZE-1+_PAGE_OFFSET;
	tss.ss0=__KERNEL_DS;
	
	kprint("\nLoading Task Register\n");
//	kprint("Task!init\n");
	
	k_ltr();

	//Alloc memory for task list:
	task0=kmalloc(sizeof(struct task_struct));
	task1=kmalloc(sizeof(struct task_struct));
	task2=kmalloc(sizeof(struct task_struct));


	
	task_struct_list[0]=task0;
	task_struct_list[1]=task1;
	task_struct_list[2]=task2;
	
	for(t=0;t<TASK_COUNT;t++){
		/*first patch their kernel stack 
		 *for testing,I put their stack to 0x100000(1M,4K aligned)
		 */
		ULONG *sp;
		task_struct_list[t]->pid=t;
//		task_struct_list[t]->esp=KERNEL_STACK_OFFSET+(t+1)*PAGE_SIZE-1;
//		task_struct_list[t]->ss=__KERNEL_DS;

//	kmemcpy(task_struct_list[t],KERNEL_STACK_OFFSET+t*PAGE_SIZE,sizeof(struct task_struct) );

		task_struct_list[t]->esp=0xC0010fff+t*0x1000;
		task_struct_list[t]->esp0=task_struct_list[t]->esp;
		task_struct_list[t]->ss=__KERNEL_DS;
		task_struct_list[t]->kstack=0x0;	//C0010fff-0x1000;		/* take care! not sure it is proper */
		task_struct_list[t]->ustack=0;
		task_struct_list[t]->cr3=0;
		task_struct_list[t]->state=0;
		task_struct_list[t]->timelapse=TIME_SLICE;
		task_struct_list[t]->timewait=0;
		/*
		 * i don't think it's nesscessary and
		 * kmemset() is not good for a two-byte-long 0x20
		 * 
		kmemset(task_struct_list[t]->console.con,80*25*2,0);
		*/
		task_struct_list[t]->console.x=0;
		task_struct_list[t]->console.y=0;		
//		task_struct_list[t]->vbuffer=0xC0070fff+t*80*25*2;
//		kmemset(task_struct_list[t]->vbuffer,80*25*2,'0');
		
		sp=(ULONG *)task_struct_list[t]->esp;
/****************************************************************/
/*************      Pushed by CPU            ********************/
/****************************************************************/
	  //*--sp=0;
		*--sp=0x1b;								//User space SS == __USER_DS
/* This is for debugging... */
		//*--sp=__KERNEL_DS;//__USER_DS;	  
		*--sp=0x100000+0x19000*(t+1);/*100K stack per task*/
		*--sp=0x0202;	/*EFlags,Interrupt flag enable ,bit one should set to be 1*/
		*--sp=0x23; /* __USER_CS; */

/* This is for debugging... */	  
		//*--sp=__KERNEL_CS;//__USER_CS;	  
		*--sp=task_list[t];				//User space EIP

/****************************************************************/
/*************			Pushed by user           ********************/
/****************************************************************/
		*--sp=0x1b;//__USER_DS;		//gs
		*--sp=0x1b;//__USER_DS;		//fs
		*--sp=0x1b;//__USER_DS;		//es
		*--sp=0x1b;//__USER_DS;		//ds

/* This is for debugging... 
		*--sp=__KERNEL_DS;//__USER_DS;		//gs
		*--sp=__KERNEL_DS;//__USER_DS;		//fs
		*--sp=__KERNEL_DS;//__USER_DS;		//es
		*--sp=__KERNEL_DS;//__USER_DS;		//ds	  
*/	  
		*--sp=0;		//ebp
		*--sp=0;		//esp
		*--sp=0;		//edi
		*--sp=0;		//esi
		*--sp=0;		//edx
		*--sp=0;		//ecx
		*--sp=0;		//ebx
		*--sp=0;		//eax

/* set correct value for process structure */
		task_struct_list[t]->esp=(ULONG)(sp);
	  }

//	tss.esp0=KERNEL_STACK_OFFSET+0*PAGE_SIZE-1+_PAGE_OFFSET;
//	tss.ss0=__KERNEL_DS;


	last=current=task_struct_list[0];
//  last=(void *)0; 
  
}

void schedule(void)
{
	static int i=0;
	int s;
check_wait:
	if(current->timewait!=0){
		current->timelapse =TIME_SLICE;
		i=i%TASK_COUNT;
		while(task_struct_list[i]->timewait!=0){	// we must asure at least there is one task does not waiting ,that's "idle"
			i++;
			i=i%TASK_COUNT;
		}
		current=task_struct_list[i];
		current->timelapse=TIME_SLICE;
		i++;
	}
	
	if(current->timelapse-- == 0){
		current->timelapse =TIME_SLICE;
		if(i==TASK_COUNT) i=0;
		current=task_struct_list[i];
		i++;
	}
	for(s=0;s<TASK_COUNT;s++){
		if(task_struct_list[s]->timewait!=0)
			task_struct_list[s]->timewait--;
	}
	return;
}


void sleep()
{
	unsigned int time=1110;
	current->timewait=time;
	current->timelapse=0;
//	print("----------------In Sleep---------------\n");
}










