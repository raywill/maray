#ifndef __ASMCMD
#define __ASMCMD


#define halt() __asm__ __volatile__("hlt")
#define sti() __asm__ __volatile__("sti")
#define cli() __asm__ __volatile__("cli")

#define FIRST_TSS_ENTRY 5
/*
#define _TSS(n) ((((unsigned long) n)<<4)+(FIRST_TSS_ENTRY<<3))
#define ltr(n) __asm__("ltr %%ax"::"a" (_TSS(n)))
*/
	
long getCR2(void);
long getCR3(void);
void sys_ltr(void);
void sys_lock(void);
void sys_unlock(void);
void debug_halt(void);


#endif
