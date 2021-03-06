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

unsigned short *vp;
static int cur_x;
static int cur_y;

static short color = 0x0200;

void init_tty(void)
{
	vp=VIDMEM;
	clear();
	set_cursor(0);
}

// Set blinking cursor postion
static void set_cursor(int cursorpos)
{
	outportb(0x3D4, 14);   // write to register 14 first
	outportb(0x3D5, (cursorpos>>8) & 0xFF);     // output high byte
	outportb(0x3D4, 15); // again to register 15
	outportb(0x3D5, cursorpos & 0xFF);  // low byte in this register
		
}


void clrscr(void)
{
	clear();
}

void getxy(int *x,int  *y)
{
	*x=cur_x;
	*y=cur_y;
}
void gotoxy(int x,int y)
{
	if(y<0||y>24||x<0||x>79)
		return;
	vp=VIDMEM+y*80+x;
	cur_x = x;
	cur_y = y;
}

static void clear( void )
{
    int i;
    for (i = 0; i < 80 * 25 ; i++)
        *vp++ = 0x20 | color;
    vp = VIDMEM;
    cur_x = cur_y = 0;
    /* set_cursor(0); */
    /* cant put it because this function is used under user privilege */
}

void save_console(struct task_struct *sel)
{
	//save the old console content
	int x,y;
	sel->console.x = cur_x;
	sel->console.y = cur_y;
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
	int tmp_color = color;
	color = 0x0600;
	
	while (*str != '\0')
		putc(*str++);
	color = tmp_color;
	set_cursor(vp-VIDMEM);
}

void print( const char* str )
{

	sys_lock();
	if( vp>=VIDMEM+80*25 )
	{
		clear();
		vp=VIDMEM;
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
	
	sys_unlock();
}

void 
scroll_screen()
{
	int i;
	short *p = VIDMEM;
	for (i = 0; i < 80 * 24; ++i)
		p[i] = p[i + 80];
	for (i = 0; i < 80; ++i)
		p[i + 80 * 24] = 0x20 | color;
}

void
putc(int c)
{
	int tmp;

	if (!(c & 0xff00))
		c = c | color;

	switch (c & 0x00ff) {
	case '\n':
		cur_y++;
		cur_x = 0;
		break;
	case '\t':
		cur_x += 8 & ~7;
		break;
	case '\b':
		if(cur_x) cur_x--;
		break;
	
	default:
		cur_x++;
		*vp = c & 0xffff;
	}


	if (cur_x >= 80) {
		cur_x = 0;
		cur_y++;
	}

	if (cur_y >= 25) {
		scroll_screen();
		cur_y = 24;
		cur_x = 0;
	}

	tmp = (cur_y << 6) + (cur_y << 4) + cur_x;
	set_cursor(tmp);
	vp = VIDMEM + tmp;
}

