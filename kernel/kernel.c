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
#include <i386/pci.h>
#include <maray/kb.h>
#include <maray/tty.h>
#include <asmcmd.h>
#include <maray/clock.h>
#include <stdio.h>

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
   	kprintf( "TTY initialized\n" );
	
	init_mm();
   	kprintf( "Memory manager initialized\n" );		  	
	
	init_irq();	/*initialize irq,with all interrupte disabled.*/
   	kprintf( "IRQ initialized\n" );

	kprintf("\nHello from syscall\n");
	install_syscall();
	
	init_all_tasks();

	init_kb();	/* set keyboard IRQ,and enable it */
	kprintf( "\nKeyboard initialized\n" );
 
	//init_timer(); /* initialize time, enable timer irq */
	/* init_system_clock(&real_tm); */
	kprintf( "\nTimer initialized\n");
	init_system_clock(&start_tm);
	kprintf("\nSystem start time is \n");
	kprintf(timetostr(&start_tm, tmbuf));

	kprintf("\nstart scanning pci devices\n");
	scan_pci_device();
	halt();

	kprintf("\nStarting first process....\n");
	
	start_first_process();

	kprintf( "\nNow I am doing a loop ,waiting for interrupt :)\n" );
	
	while(1);
	halt();
}

