#ifndef __I386_PCIIDE
#define __I386_PCIIDE

#include <i386/types.h>

void pciide_setup(uint16_t bus, uint16_t slot,
		  uint16_t fnc);

#endif /* __I386_PCIIDE */
