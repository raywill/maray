/*
	Name:	tty.c
	Copyright: GPL
	Author: Raywill
	Date: 22-12-06
	Description: Console Driver
*/



#include <maray/tty.h>
#include <i386/task.h>
#include <asmcmd.h>
#include <libc.h>


#define TAB_SIZE 8
#define thisline(vp) ((vp -VIDMEM)%80)
#define thisrow(vp)	((vp -VIDMEM)/80)

unsigned short* const VIDMEM = ((unsigned short*) 0xB8000);
unsigned long* const XVIDMEM = ((unsigned long*) (0xB8000+0xC0000000));
static void clear( void );
static void set_cursor(int pos);

unsigned short * vp;

void init_tty(void)
{
	vp=VIDMEM;
	clear();
}

// Set blinking cursor postion
static void set_cursor(int cursorpos)
{
	
	outportb(0x3D4, 14);                       // write to register 14 first

	outportb(0x3D5, (cursorpos>>8) & 0xFF);     // output high byte

	outportb(0x3D4, 15);                       // again to register 15

	outportb(0x3D5, cursorpos & 0xFF);          // low byte in this register
		
}


void clrscr(void)
{
	clear();
}

void getxy(int *x,int  *y)
{
	*x=thisline(vp);
	*y=thisrow(vp);
}
void gotoxy(int x,int y)
{
	if(y<0||y>24||x<0||x>79)
		return;
	vp=VIDMEM+y*80+x;
}

static void clear( void )
{
    int i;
    for (i = 0; i < 80 * 25 ; i++)
        *vp++ = 0x20;
    vp = VIDMEM;
}

/*
void save_console(struct task_struct *sel)
{
	//save the old console content
	cli();
	int x,y;
	getxy(&x,&y);
	sel->console.x=x;
	sel->console.y=y;
	kmemcpy(VIDMEM,sel->vbuffer,80*25*2);
	sti();
	return;
}

void set_console(struct task_struct *sel)
{
	//set new console content
	cli();
	gotoxy(sel->console.x,sel->console.y);
	kmemcpy(sel->vbuffer,VIDMEM,80*25*2);
	sti();
	return;
}
*/
void save_console(struct task_struct *sel)
{
	//save the old console content
	int x,y;
	getxy(&x,&y);
	sel->console.x=x;
	sel->console.y=y;
	kmemcpy(VIDMEM,sel->console.con,80*25*2);
	return;
}

void set_console(struct task_struct *sel)
{
	//set new console content
	gotoxy(sel->console.x,sel->console.y);
	kmemcpy(sel->console.con,VIDMEM,80*25*2);
	return;
}

void kprint( const char* str )
{
    if( vp>=VIDMEM+80*25 )
    	{
    		clear();
    		vp=VIDMEM	;
    	}
    while (*str != '\0')
    {
        if(*str=='\n')
       	 	{
        		vp=vp + 80 - (vp -VIDMEM) % 80;
        		str++;
        		continue;
        	}
        if(*str=='\t')
        	{
        		vp=vp+TAB_SIZE-thisline(vp)%TAB_SIZE;
        		str++;
        		continue;	
        	}
        *vp++ = (0x0600 | ((unsigned short) *str++));
    }
	set_cursor(vp-VIDMEM);

}

void print( const char* str )
{
		 //

		//__asm volatile("int %0" ::"i" (0x81));
  
  	sys_lock();
  	
    if( vp>=VIDMEM+80*25 )
    	{
    		clear();
    		vp=VIDMEM	;
    	}
    while (*str != '\0')
    {
        if(*str=='\n')
       	 	{
        		vp=vp + 80 - (vp -VIDMEM) % 80;
        		str++;
        		continue;
        	}
        if(*str=='\t')
        	{
        		vp=vp+TAB_SIZE-thisline(vp)%TAB_SIZE;
        		str++;
        		continue;	
        	}
        *vp++ = (0x0600 | ((unsigned short) *str++));
    }
	//set_cursor((int)(vp-VIDMEM));
// __asm volatile("int %0" ::"i" (0x80));
		sys_unlock();


}

