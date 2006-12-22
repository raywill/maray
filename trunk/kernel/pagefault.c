/*
	Name:	pagefault.c
	Copyright: GPL
	Author: Raywill
	Date: 22-12-06
	Description: Page fault handler
*/


#include <i386/irq.h>
#include <maray/type.h>
#include <asmcmd.h>
#include <i386/vector.h>		/*regs_t*/

extern void debug_hlt();

void page_fault_handler(regs_t *regs)
{
	char buf[20];
//	kprint("\nA page fault occur.Currently with no solution :(\nFault type:");
	/*
	if(regs->err_code&1)
	kprint("Access rights violation or the use of a reserved bit!\n");
	else
	kprint("Page not present!\n");
	*/
	kprint("Error Code:");
	kprint(itoa(regs->err_code,buf,16));
	kprint("\nAddress:");
	kprint(itoa(regs->eip,buf,16));
	/*I really need a printf() :! */
	debug_hlt();
	cli();

}
