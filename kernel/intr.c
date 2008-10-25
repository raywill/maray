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

		
	switch(regs->which_int)
	{
	/* this handler installed at compile-time
	 * Keyboard handler is installed at run-time (see below) 
	 */
	case 0x20:	/* timer IRQ 0 */
		kprintf("Timer\n");
		outportb(0x20, 0x20);	/* reset hardware interrupt at 8259 chip */
		break;
	case 14:	/* Page Fault */
		/* following code will be deleted later */
#if 0
		kprintf("Error Code:");
		kprintf("%x", regs->err_code);
		kprintf("\nAddress:");
		kprintf("%x\n", regs->eip);
		kprintf("\nDetails: A Page Fault Occurs! CR2=");
		kprintf("%x\n", getCR2());
#endif
		/* and be replaced by the following code */
		page_fault_handler(regs);	/* inefficient? Oh, just let it be! */
		
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

