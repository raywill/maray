#ifndef __I386_PAGEFAULT_
#define __I386_PAGEFAULT_

#include <i386/types.h>
#include <i386/vector.h>

void page_fault_handler(regs_t *regs);


#endif /* maray i386/pagefault.h */

