#include <i386/types.h>
#include <i386/pciide.h>
#include <libc.h>
#include <stdio.h>

uint32_t
read_pci_reg(uint16_t bus, uint16_t slot,
		uint16_t func, uint16_t offset)
{
	uint32_t address;
	uint32_t lbus = bus;
	uint32_t lslot = slot;
	uint32_t lfunc = func;

	address = (lbus << 16) | (lslot << 11) | (lfunc << 8) |
	    (offset & 0xfc) | 0x80000000;

	outportl(0xcf8, address);
	return inportl(0xcfc);
}

void 
scan_pci_device()
{
	int bus, dev, fnc, nr_fnc;
	uint32_t htype, vendor_id, device_id, class;
	uint32_t reg0;
	for (bus = 0; bus < 1; ++bus)
		for (dev = 0; dev < 64; ++dev) {
			htype = (read_pci_reg(bus, dev, 0, 0x0c) >> 16) & 0xff;
			reg0 = read_pci_reg(bus, dev, 0, 0);
			vendor_id = reg0 & 0xffff;

			/* kprintf(" 0x%x ", reg0); */

			if (vendor_id == 0 || vendor_id == 0xffff) continue;
			if (htype & 0x80) nr_fnc = 8;
			else nr_fnc = 1;

			/* kprintf("reg0=%x", reg0); */

			for (fnc = 0; fnc < nr_fnc; ++fnc) {
				vendor_id = read_pci_reg(bus, dev, fnc, 0);
				device_id = (vendor_id >> 16) & 0xffff;
				vendor_id &= 0xffff;
				class = read_pci_reg(bus, dev, fnc, 0x08);
				if (vendor_id != 0 && vendor_id != 0xffff) {
					kprintf("%02x:%02x.%01x: "
						"%04x:%04x %04x\n",
						bus, dev, fnc, vendor_id, 
						device_id, class);
					/*
					 * just attach the PIIX3 PCI IDE 
					 * interfaces for BOCHS and QEMU
					 */

					kprintf("%08x\n", class & 0x01010000);

					if ((class & 0x01010000) == 0x01010000 
					    && vendor_id == 0x8086 && 
					    device_id == 0x7010) {
						pciide_setup(bus, dev, fnc);
					}
				}
			}
		}
}

