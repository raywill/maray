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
#include <stdio.h>
#include <pagefault.h>


extern void debug_hlt();

void page_fault_handler(regs_t *regs)
{
	char buf[20];
	uint32_t vaddr = 0;

	if(regs->err_code&1)
	{
		kprint("Access rights violation or the use of a reserved bit!\n");
	}
	else
	{
		kprint("Page not present!\n");
	}

	kprintf("Error Code: %x, Address: %x\n", regs->err_code, regs->eip);
	kprintf("Details: A Page Fault Occurs! CR2=%x\n", getCR2());
	
	kprintf("Dont think we should work around this? Yep.\n");

	/* get the voilated address */
	vaddr = getCR2();
	/* validate the address and do possible mapping works */
	if(access_ok(vaddr))
	{
		/* do the so called 'demand paging' */
		/* in KERNEL MODE, this takes the following order:
		 * (1) lookup the page table through page directory
		 * (2) if page table missed, allocate a page table first
		 * (3) allocate a page for the demand, record its physical
		 *     address in a page entry, with could be indexed by
		 *     vaddr.
		 * (4) flush TLB
		 * (5) return.
		 */
		
	}
	else
	{
		kprintf("Bad access and can't fix it!\n");
		debug_hlt();
		cli();
	}

	/* page mapped, let return~ */
	return;
}

