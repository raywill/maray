#include <i386/types.h>
#include <i386/irq.h>
#include <i386/vector.h>
#include <i386/pciide.h>
#include <maray/ata.h>
#include <asmcmd.h>
#include <libc.h>
#include <stdio.h>

extern uint32_t pciide_bar, pciide_base;

static void
hdirq(void)
{
	uint8_t status;

	kprintf("got a ata0 irq\n");
	outportb(pciide_bar, 0x0); /* reset Start/Stop bit */

	status = inportb(pciide_bar + 0x02);
	if ((status & 0x02) != 0)
		kprintf("error: controller status\n");

	status = inportb(0x1f7);
	if ((status & 0x01) != 0)
		kprintf("error: device status\n");

	outportb(0xA0, 0x20);
	outportb(0x20, 0x20);
}

void
init_hdirq(void)
{
	vector_t hdirq_vector;
	hdirq_vector.eip = (unsigned)hdirq;
	hdirq_vector.access_byte = 0x8E;
	setvect(&hdirq_vector, HDD_IRQ);
	enable_irq(HDD_IRQ);
	enable_irq(X2_IRQ); /* necessary ? */
}

static void
waitdisk(void)
{
	while ((inportb(0x1f7) & 0xc0) != 0x40);
}

void
ata_identify_device(void)
{
	uint16_t info[256];
	int i, j, t;

	/* bit4 DEV = 0 */
	outportb(0x1f6, 0x00); 
	/* identify device */
	outportb(0x1f7, 0xec);

	/* wait for disk to be ready */
	waitdisk();
	
	/* read outputs */
	insl(0x1f0, info, 128);

	kprintf("\nidentify device\n");
	if ((info[0] & 0x8000) == 0) {
		kprintf("no ata0-0\n");
		return;
	} else
		kprintf("ata0-0:\n");
	kprintf("\tSerial No: ");
	/* big-endian */
	for (i = 10; i < 20; ++i) {
		t = info[i];
		kprintf("%c%c", (t >> 8) & 0xff, t & 0xff);
	}
	kprintf("\n\tModel No: ");
	for (i = 27; i < 47; ++i) {
		t = info[i];
		kprintf("%c%c", (t >> 8) & 0xff, t & 0xff);
	}

#ifdef DEBUG
	t = info[53];
	if (t & 0x02)
		kprintf("\nword 88 are valid");
	else
		kprintf("\nword 88 are not valid");
	if (t & 0x01)
		kprintf("\nwords (70:64) are valid");
	else
		kprintf("\nwords (70:64) are not valid");

	kprintf("\nword 49: %04x\n", info[49]);
	kprintf("word 63: %04x\n", info[63]);
	kprintf("word 80: %04x\n", info[80]);
	kprintf("word 88: %04x\n", info[88]);
#endif
}

void
ata_set_feature(void)
{
	uint8_t error;

	waitdisk();
	kprintf("\nata0-0:set feature\n");
	outportb(0x1f1, 0x03); /* Set transfer mode */
	outportb(0x1f2, 0x22); /* MWDMA 00100b, MODE2 010b */
	outportb(0x1f6, 0x00); /* device 0 */
	outportb(0x1f7, 0xef);
	waitdisk();
	error = inportb(0x1f2);
	if ((error & 0x04) != 0)
		kprintf("set feature error\n");
	else
		kprintf("set feature Multiword DMA2 done\n");
}

void
ata_readdma_test()
{
	uint8_t error;
	uint16_t *p;
	int i, j;

	init_hdirq();

	i = pci_conf_read(pciide_base, 0x04);
	kprintf("pci configuration space status %08x\n", i);

	sti();
	waitdisk();

	/* set control block register */
	outportb(0x3f6, 0x00); /* enable IEN, set to 0 */

	outportb(pciide_bar + 2, 0x26);
	error = inportb(pciide_bar + 2);
	kprintf("1-status %02x:", error);

	p = (uint16_t *)0xc0019000;
	kprintf("old value = %04x", p[256]);

	outportb(0x1f2, 1); /* count */
	outportb(0x1f3, 0); /* LBA begins from 0 */
	outportb(0x1f4, 0);
	outportb(0x1f5, 0);
	outportb(0x1f6, 0x40); /* LBA 1, DEV 0 */
	outportb(0x1f7, 0xc8); /* command to disk */

	outportb(pciide_bar, 0x09); /* read control and start bit. GO */

	/* i need waste some time ? */
	kprintf("\tstart wasting");
	j = 0;
	for (i = 0; i < 0xfff; ++i)
		j += i % 13;
	kprintf("\twaste some time\n");

	error = inportb(pciide_bar + 0x2);
	kprintf("status: %02x\n", error);

	error = inportb(0x1f7);
	kprintf("status = %02x\n", error);
	if ((error & 0x1) != 0)
		kprintf("read sectors error\n");
	else {
		kprintf("read sectors ok\n");
		p = (uint16_t *)0xc0019000;
		kprintf("%04x\n", *p);
	}
}
