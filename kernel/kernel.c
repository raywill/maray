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
extern void install_syscall(void);



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
	init_tty();	/*	initialize the screen											*/		
   	kprint( "TTY initialized\n" );
	
	init_mm();
   	kprint( "Memory manager initialized\n" );		  	
	
	init_irq();	/*initialize irq,with all interrupte disabled.*/
   	kprint( "IRQ initialized\n" );
	
	init_kb();	/* set keyboard IRQ,and enable it							*/
    kprint( "Keyboard initialized\n" );
    
	init_timer();	/* initialize time,enalbe timer irq					*/
	kprint( "Timer initialized\n" );
	
	init_system_clock(&start_tm);
	init_system_clock(&real_tm);	
	kprint("System start time is ");
	kprint(timetostr(&start_tm,tmbuf));
	
	kprint("\nHello");
	install_syscall();
	
	init_all_tasks();
	
	kprint( "\nNow I am doing a loop ,waiting for interrupt :)\n" );
	
	while(1);
	halt();
}



