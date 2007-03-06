#ifndef __I386_PCI
#define __I386_PCI

#include <i386/types.h>

struct pci_conf_regs {
	uint16_t device_id;
	uint16_t vendor_id;
	uint16_t class;
	uint32_t interface; /* program interface */
	uint8_t htype;
};

void scan_pci_device();

#endif /* maray i386/pci.h */

