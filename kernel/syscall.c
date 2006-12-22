/*
	Name:	syscall.c
	Copyright: GPL
	Author: Raywill
	Date: 22-12-06
	Description: Init System calls for Ring 3 Program
*/


#include <i386/irq.h>			/*	enalbe_irq()	*/
#include <i386/vector.h>	/*	setvect()			*/
#include <libc.h>					/*	disable(),enable()		*/


void install_syscall(void){
	  vector_t v;
	  v.eip = (unsigned)enable;
	  v.access_byte = 0xEF; /* present, ring 3, '386 trap gate D=32bit gate*/
	  setvect(&v, 0x80);

	  v.eip = (unsigned)disable;
	  v.access_byte = 0xEF; /* present, ring 3, '386 trap gate D=32bit gate*/
	  setvect(&v, 0x81);


	  v.eip = (unsigned)debug_hlt;
	  v.access_byte = 0xEF; /* present, ring 3, '386 trap gate D=32bit gate*/
	  setvect(&v, 0x82);
	  
	  v.eip = (unsigned)sys_wait;
	  v.access_byte = 0xEF; /* present, ring 3, '386 trap gate D=32bit gate*/
	  setvect(&v, 0x83);
}