OUTPUT_FORMAT("binary")
ENTRY(kernel_entry)
phys = 0xc0000000;

SECTIONS {
        .text  phys : AT(phys)
        { 
                code = . - phys + 0x100000;
                *(.text)
                . = ALIGN(4096);
        }
        .data : 
        {
                data = . - phys + 0x100000;
                *(.data)
                *(.rodata)
                . = ALIGN(4096);
        }
        .bss :
        {
                bss = . - phys + 0x100000;
                *(.bss)
                *(COMMON)
                . = ALIGN(4096);
        }
        ends = . - phys + 0x100000;
}

