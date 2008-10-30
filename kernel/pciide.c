/*
 * experiment code for bus-master DMA HardDisk READ
 * 
 * Compatibility Mode, Multiword DMA Mode
 * Channel 0, Drive 0
 *
 * Mar 1, 2007
 */

#include <i386/pciide.h>
#include <i386/types.h>
#include <maray/ata.h>
#include <stdio.h>

struct prdt {
	uint32_t base;
	uint32_t count;
} p_prd;

uint32_t pciide_bar;
uint32_t pciide_base;

/*
static void
get_conf_reg(struct pci_conf_regs *pcr, uint16_t bus,
	     uint16_t slot, uint16_t fnc)
{
	uint32_t reg, addr;

	addr = ((uint32_t)bus << 16) | ((uint32_t)slot << 11) | 
	    ((uint32_t)fnc << 8) | 0x80000000;

	outportl(0xcf8, addr);
	reg = inportl(0xcfc);
	pcr->device_id = (reg >> 16) & 0xffff;
	pcr->vendor_id = reg & 0xffff;

	outportl(0xcf8, addr | 0x08);
	reg = inportl(0xcfc);
	pcr->class = (reg >> 16) & 0xffff;
	pcr->interface = (reg >> 8) & 0xfffff;

	outportl(0xcf8, addr | 0x0c);
	reg = inportl(0xcfc);
	pcr->htype = (reg >> 16) & 0xff;
}
*/
uint32_t
pci_conf_read(uint32_t base, uint32_t offset)
{
	outportl(0xcf8, base | (offset & 0xfc));
	return inportl(0xcfc);
}

void
pci_conf_write(uint32_t base, uint32_t offset, uint32_t data)
{
	outportl(0xcf8, base | (offset & 0xfc));
	outportl(0xcfc, data);
	outportl(0xcf8, 0);
}

void
pciide_setup(uint16_t bus, uint16_t slot,
	     uint16_t fnc)
{
	uint32_t addr, tr;
	uint32_t bar, idetim;

	addr = ((uint32_t)bus << 16) | ((uint32_t)slot << 11) | 
	    ((uint32_t)fnc << 8) | 0x80000000;
	pciide_base = addr;


	/* 
	 * only for PIIX3
	 * so just do it
	 */

	kprintf("setup piix\n");

	/* check IOSE */
	tr = pci_conf_read(addr, 0x04);
	if ((tr & 0x01) == 0)
		kprintf("piix ide i/o space disabled\n");
	else
		kprintf("piix ide i/o space enable\n");

	tr |= 4;
	pci_conf_write(addr, 0x04, tr);

	/* get base address register */
	tr = pci_conf_read(addr, 0x20);
	bar = tr & 0xfffffffc;
	pciide_bar = bar;

	/* enable IDE registers decode */
	idetim = pci_conf_read(addr, 0x40);

	/* 
	 * setup dma physical region descriptor table
	 * physical address 0x19000-0x1a000 only 4k
	 */

	if ((int)&p_prd & 0x03 != 0)
		kprintf("physical region descriptor should align 4 byte");
	p_prd.base = 0x19000;
	p_prd.count = 0x1000 | 0x80000000;
	outportl(bar + 0x4, (int)&p_prd - 0xc0000000); /* physical or ? */

	/* 
	 * XXX probe ata drive first 
	 * 
	 * actually do nothing
	 * all parameters are hard-coded
	 */
	kprintf("ata_identify_device()\n");
	ata_identify_device();

	/* setup ISP, PPE, IE, TIME filed */
	idetim &= ~0x00003300; /* clear */
	idetim |= 0x3107; /* ISP=2, RTC=3, enable ALL */
	pci_conf_write(addr, 0x40, idetim);

	/* 
	 * enable IDE status register
	 * bit 5: drive 0 dma capable
	 */
	outportb(bar + 0x02, 0x20);

	kprintf("ata_set_feature()\n");
	ata_set_feature();

	kprintf("ata_readdma_test()\n");
	/* gogogo */
	ata_readdma_test();
}

