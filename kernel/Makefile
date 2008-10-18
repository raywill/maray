.SUFFIXES: .asm

# defines
MAKEFILE=Makefile
MAKEDEP=$(MAKEFILE)
INCDIR	=../include
LDSCRIPT=./ldscript.ld
NASM	=nasm -f elf -i$(INCDIR)
CC	=gcc -fno-leading-underscore -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -I$(INCDIR)
#LD	=ld -Ttext 0xC0000000 --oformat binary -e kernel_entry -nostdlib
#LD	=ld -Ttext 0xC0000000 --oformat coff-go32 -e kernel_entry -nostdlib
LD	=ld -Ttext 0xc0000000 -T $(LDSCRIPT)
OBJS	= kinit.o kernel.o tty.o kb.o timer.o irq.o libc.o intr.o clock.o func.o memory.o pagefault.o page.o task.o syscall.o \
	  kprintf.o pci.o pciide.o ata.o

# targets
all: kernel.bin

install:
	copy /b ..\boot\boot.bin+kernel.bin ..\new.bin

clean:
#	del	 *.o
	rm	*.o -fr

# implicit rules
.asm.o:
	$(NASM) -o$@ $<

.c.o:
	$(CC) -c -o$@ $<

#dependencies
kinit.o: kinit.asm
kernel.o:	kernel.c	../include/i386/irq.h
tty.o: tty.c
func.o: func.asm
kb.o:	kb.c
clock.o: clock.c
timer.o:	timer.c
irq.o:		irq.c
libc.o:		libc.c
intr.o:		intr.c		#$(MAKEDEP)
memory.o: memory.c
task.o: task.c
pagefault.o: pagefault.c
syscall.o: syscall.c
kprint.o: kprintf.c
pci.o: pci.c
pciide.o : pciide.c
ata.o: ata.c
# explicit rules
#$(LIBC):
#	make -C ../lib -f $(MAKEFILE)


kernel.bin: $(OBJS) $(MAKEDEP)
	$(LD) -o$@ $(OBJS) -M >ld.map
#	objdump --source $@ >krnl.lst
#	nm --line-numbers $@ | sort >krnl.sym
#	strip $@
