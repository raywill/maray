/*
	Name:	kernel.c
	Copyright: GPL
	Author: Raywill
	Date: 22-12-06
	Description: Main procedure of the system initalization
*/

#include <maray/tty.h>
#include <i386/timer.h>
#include <i386/task.h>
#include <i386/irq.h>
#include <i386/memory.h>
#include <maray/kb.h>
#include <maray/tty.h>
#include <asmcmd.h>
#include <maray/clock.h>

/*extern void enable_paging(void);*/
void install_syscall(void);
void start_first_process();

sys_time start_tm;
sys_time real_tm;

void osmain(void);


void osmain( void )
{
	int i;
	int j=0;
	char nb[10];
	
	char tmbuf[30];
	
	
	/* ini screen first,so that we can output info as early as possible */
	init_tty();	/*	initialize the screen*/	
   	kprint( "TTY initialized\n" );
	
	init_mm();
   	kprint( "Memory manager initialized\n" );		  	
	
	init_irq();	/*initialize irq,with all interrupte disabled.*/
   	kprint( "IRQ initialized\n" );
   
	kprint("\nHello\n");
	install_syscall();
	
	init_all_tasks();

	init_kb();	/* set keyboard IRQ,and enable it */
	kprint( "\nKeyboard initialized\n" );
 
	init_timer(); /* initialize time, enable timer irq */
	/* init_system_clock(&real_tm); */
	kprint( "\nTimer initialized\n");
	init_system_clock(&start_tm);
	kprint("\nSystem start time is \n");
	kprint(timetostr(&start_tm, tmbuf));

	kprint("\nStarting first process....\n");
	start_first_process();

	kprint( "\nNow I am doing a loop ,waiting for interrupt :)\n" );
	
	while(1);
	halt();
}



