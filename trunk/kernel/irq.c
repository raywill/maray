/*
  Name:	irq.c
  Copyright: GPL
  Author: Raywill
  Date: 22-12-06
  Description: init,disable and enable interrupt procedure
*/


#include <i386/irq.h>
#include <maray/type.h>

#ifndef IRQ1_PORT
	#define IRQ1_PORT 0x21
	#define IRQ2_PORT 0xA1
#endif

void init_irq()
{
	static const unsigned irq0_int = 0x20, irq8_int = 0x28;
/**/

/* Initialization Control Word #1 (ICW1) */
	outportb(0x20, 0x11);
	outportb(0xA0, 0x11);
/* ICW2:
route IRQs 0-7 to INTs 20h-27h */
	outportb(0x21, irq0_int);
/* route IRQs 8-15 to INTs 28h-2Fh */
	outportb(0xA1, irq8_int);
/* ICW3 */
	outportb(0x21, 0x04);
	outportb(0xA1, 0x02);
/* ICW4 */
	outportb(0x21, 0x01);
	outportb(0xA1, 0x01);
/* enable IRQ0 (timer) and IRQ1 (keyboard) */
	outportb(0x21, ~0x00);/*only keyboard for testing*/
	outportb(0xA1, ~0x00);
	
	kprint("8259 init OK!\n");	
}

void enable_irq(int irq)
{
	/*two case:irq<8,irq>=8*/
	byte bit=1;
	
	if(irq>(15+IRQ_OFFSET)||irq<(0+IRQ_OFFSET)) return;
	
	bit<<=irq%8;
	if( irq < (8+IRQ_OFFSET) ){
		outportb(IRQ1_PORT,~bit & inportb(IRQ1_PORT) );
	}else{
		outportb(IRQ2_PORT,~bit & inportb(IRQ2_PORT));
	}
}

void disable_irq(int irq)
{
	/*two case:irq<8,irq>=8*/
	byte bit=1;
	
	if(irq>(15+IRQ_OFFSET)||irq<(0+IRQ_OFFSET)) return;

	bit<<=irq%8;
	if( irq < (8+IRQ_OFFSET) ){
		outportb(IRQ1_PORT,bit|inportb(IRQ1_PORT));
	}else{
		outportb(IRQ2_PORT,bit|inportb(IRQ2_PORT));
	}
}
