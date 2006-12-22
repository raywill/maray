#ifndef __I386_VECTOR
#define __I386_VECTOR

/* Set vector(Gate,IRQ,TRAP)*/
#define ACCESS_TASK_GATE 0x85
#define ACCESS_INTERRUPT_GATE 0x8E
#define ACCESS_TRAP_GATE 0x8F

typedef struct
{
	/* Value for access_byte:
	 * task gate = 0x85
	 * interrupt gate = 0x8E
	 * trap gate = 0x8F
	*/
	unsigned access_byte;	/*	access_byte=0x8E for sysuse */
	unsigned eip;					/*	eip=handler address					*/
} vector_t;

typedef struct
{
/* pushed by pusha */
	unsigned edi, esi, ebp, esp, ebx, edx, ecx, eax;
/* pushed separately */
	unsigned ds, es, fs, gs;
	unsigned which_int, err_code;
/* pushed by exception. Exception may also push err_code.
user_esp and user_ss are pushed only if a privilege change occurs. */
	unsigned eip, cs, eflags, user_esp, user_ss;
} regs_t;

void setvect(vector_t *v, unsigned vect_num);/* in asm */

#endif
