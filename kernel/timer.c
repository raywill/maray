/*
	Name:	timer.c
	Copyright: GPL
	Author: Raywill
	Date: 22-12-06
	Description: Deal with timer initalization and timer interrupt
*/


#include <maray/type.h>
#include <i386/vector.h>
#include <i386/irq.h>
#include <i386/timer.h>
#include <maray/clock.h>
#include <maray/tty.h>
#include <asmcmd.h>
#include <libc.h>
/*'tick' time elapse*/
unsigned int timefly, last_timefly;
extern void schedule();		
void timer_irq();

extern sys_time real_tm;

/*
*how do we init timer:
*		1.init the timer frequency
*		2.set the timer irq routine.
*		3.enable timer
*/
void init_timer()
{
	int max;
	vector_t v;

	timefly = 0;
	
	/*00,use IRQ timer,00:read 2 bytes,low &high;
		  011,mode 3(square wave);0,binary mode*/
	max=SHED_RREQ;				/*set to 10ms */
	outportb(0x43,0x36);	/*0000110110b=0x36*/
	outportb(0x40, max & 0xff);//LSB
	outportb( 0x40,max >> 8 );//MSB

	enable_timer(); /* timer enable here, no longer use keyboard */
	
	//install the timer handler
   	//v.eip = (unsigned)timer_irq;
	//v.access_byte = 0x8E; /* present, ring 0, '386 interrupt gate D=32bit gate*/
	//setvect(&v, 0x20);
}

void enable_timer()
{
	init_system_clock(&real_tm);
	enable_irq(TIMER_IRQ);
	timefly = last_timefly = 0;
}

void disable_timer()
{
	disable_irq(TIMER_IRQ);
}


/*
*route of the timer irq.
*scheduler will goes from here.
*/	
void timer_irq()
{
	timefly++;
	if(timefly%2==0)
	{
		char buf[25];
		int oldx,oldy;
		getxy(&oldx,&oldy);
		gotoxy(60,23);
		kprint(itoa(timefly,buf,10));
		//kprint("ok\n");
		gotoxy(50,21);
		kprint( timetostr(update_sys_time(timefly),buf) );
		gotoxy(oldx,oldy);
	}
	schedule();
	
	outportb(0x20,0x20);
}
