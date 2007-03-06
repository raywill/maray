#ifndef __I386_TASK_AAA
#define __I386_TASK_AAA

#ifndef ULONG
#define ULONG unsigned long
#endif

#ifndef UINT
#define UINT unsigned int
#endif

#ifndef USHORT
#define USHORT unsigned short
#endif

typedef struct
{
	char con[80*25*2];//80*25*2
	int x;
	int y;
} v_console;

struct  task_tss{
	ULONG link;
	ULONG esp0;
	USHORT ss0,__ss0;
	ULONG esp1;
	USHORT ss1,__ss1;
	ULONG esp2;
	USHORT ss2,__ss2;
	ULONG cr3;
	ULONG eip;
	ULONG eflags;
	ULONG eax;
	ULONG ecx;
	ULONG edx;
	ULONG ebx;
	ULONG esp;
	ULONG ebp;
	ULONG esi;
	ULONG edi;
	USHORT es,__es;
	USHORT cs,__cs;
	USHORT ss,__ss;
	USHORT ds,__ds;
	USHORT fs,__fs;
	USHORT gs,__gs;
	USHORT ldt,__ldt;
	USHORT T;
	USHORT IOBase;
}; 

struct task_struct{
	/*basic items for each task */
	ULONG ss;
	ULONG esp;
	ULONG esp0;
	ULONG kstack;/* process's kernel stack */
	ULONG ustack;/* process's user stack */
	ULONG cr3;
	ULONG pid;
	ULONG state;
	ULONG timelapse;
	ULONG timewait;
	/*more features to control the task ,like group,parent,etc...*/
	v_console console;
	
	/*should I give each task a task_tss structure ? No idea currently :( */
	
};



/*Functions:
 *Init all tasks and begin the task switch
 */
void init_all_tasks(void);

#endif
