/*
  Name:	intr.c
  Copyright: GPL
  Author: Raywill
  Date: 22-12-06
  Description: Common interrupt procedure
*/

#include <i386/vector.h>		/*regs_t*/
#include <i386/intr.h>
#include <asmcmd.h>
#include <maray/type.h>
#include <stdio.h>

extern void debug_hlt();

void common_interrupt(regs_t *regs)
{
	char buf[20]={0};
	static const char * const msg[] =
	{
		"divide error", "debug exception", "NMI", "INT3",
		"INTO", "BOUND exception", "invalid opcode", "no coprocessor",
		"double fault", "coprocessor segment overrun",
			"bad TSS", "segment not present",
		"stack fault", "GPF", "page fault", "xxx",
		"coprocessor error", "alignment check", "xxx", "xxx",
		"xxx", "xxx", "xxx", "xxx",
		"xxx", "xxx", "xxx", "xxx",
		"xxx", "xxx", "xxx", "xxx",
		"IRQ0", "IRQ1", "IRQ2", "IRQ3",
		"IRQ4", "IRQ5", "IRQ6", "IRQ7",
		"IRQ8", "IRQ9", "IRQ10", "IRQ11",
		"IRQ12", "IRQ13", "IRQ14", "IRQ15",
		"syscall"
	};
/**/

		
	switch(regs->which_int)
	{
/* this handler installed at compile-time
Keyboard handler is installed at run-time (see below) */
	case 0x20:	/* timer IRQ 0 */
		kprintf("Timer\n");
/* reset hardware interrupt at 8259 chip */
		outportb(0x20, 0x20);
		break;
	case 14:	/* Page Fault */
		kprintf("\nA Page Fault Occurs!CR2=");
		kprintf("%d\n", getCR2());
/*
		kprint("\nUSER_ESP=");
		buf[0]='\0';
		kprint( itoa(regs->user_esp,buf) );
		kprint("\nUSER_SS=");
			buf[0]='\0';
		kprint( itoa((long)regs->user_ss,buf) );
*/		
		debug_hlt();	
/* reset hardware interrupt at 8259 chip */
		break;
	default:
		clrscr();
		kprintf("\nException occured:");
		if(regs->which_int <20)
			kprintf(msg[regs->which_int]);
		else
			kprintf("%d\n", regs->which_int );
		cli();
		halt();
		while(1)
			kprintf("In Exception Handle.");
		break;
	}
}
