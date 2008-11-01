/*
	Name:	tty.c
	Copyright: GPL
	Author: Raywill
	Date: 22-12-06
	Description: Console Driver
*/


#include <i386/types.h>
#include <i386/memory.h>
#include <maray/tty.h>
#include <i386/task.h>
#include <asmcmd.h>
#include <libc.h>


#define TAB_SIZE 8
#define thisline(vp) ((vp -VIDMEM)%80)
#define thisrow(vp)	((vp -VIDMEM)/80)

uint16_t* const VIDMEM = ((unsigned short*) 0xB8000);
uint32_t* const XVIDMEM = ((unsigned long*) (0xB8000+0xC0000000));



static void clear( void );
static void set_cursor(int pos);

uint16_t *vp;
static int32_t cur_x;
static int32_t cur_y;

uint8_t* up_stack_p;
uint8_t* up_stack_top;
uint32_t up_stack_size;
uint8_t* down_stack_p;
uint8_t* down_stack_top;
uint32_t down_stack_size;


#define LINE_SIZE 80
#define BUF_MAX_ROW_SIZE 50	/* our buffer can hold at most 50 lines */

uint32_t inited = 0;

static uint16_t color = 0x0200;

void init_tty(void)
{
	/* init video buffer */
	vp=VIDMEM;
	clear();
	set_cursor(0);
	
	/* init shadow buffer, used for recording history */
	/* the buffer is orgnized as two stacks, very cute design! */
	up_stack_p = (uint8_t*)vm_alloc_zeroed_page();	
	up_stack_top = up_stack_p;
	up_stack_size = 0;

	down_stack_p = (uint8_t*)vm_alloc_zeroed_page();
	down_stack_top = down_stack_p;
	down_stack_size = 0;

	/* used to prevent the early output before tty initialization */
	inited = 1;
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

//    sys_lock();
    for (i = 0; i < 80 * 25 ; i++)
        *vp++ = 0x20 | color;
    vp = VIDMEM;
    cur_x = cur_y = 0;
    /* set_cursor(0); */
    /* cant put it because this function is used under user privilege */
//    sys_unlock();
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
	kprint(str);
#if 0
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
#endif	
	sys_unlock();
}


uint8_t* line_stack_push(uint8_t* p, uint8_t* top, uint32_t *size, uint8_t* data, uint32_t linewidth)
{
	if(top >= p + linewidth * BUF_MAX_ROW_SIZE)
		top = p;
	kmemcpy(top, data, linewidth);
	if(BUF_MAX_ROW_SIZE > *size)
		*size = *size + 1;
	top += linewidth;
	return top;
}


uint8_t* line_stack_pop(uint8_t* p, uint8_t* top, uint32_t *size, uint8_t* data, uint32_t linewidth)
{
	if(*size > 0)
	{
		if(top <= p)
			top = p  + linewidth * BUF_MAX_ROW_SIZE;

		kmemcpy(data, top - linewidth, linewidth);
		*size = *size - 1;
		top -= linewidth;
	}
	return top;
}


void 
scroll_screen_down()
{
	int i;
	uint16_t* p = VIDMEM;

	/* push scrolled out line to screen_up_stack */
	up_stack_top = line_stack_push(up_stack_p, up_stack_top, &up_stack_size, (uint8_t*)(&p[0]), 80*2);

	/* scroll screen */
	for (i = 0; i < 80 * 24; ++i)
		p[i] = p[i + 80];
	
	/* fill the last line */
	if(down_stack_size > 0)
	{
		/*pop a line from screen_down_stack */
		down_stack_top = line_stack_pop(down_stack_p, down_stack_top, &down_stack_size, (uint8_t*)(&p[i]), 80*2);
	}
	else
	{
		/* fill blank */
		for (i = 0; i < 80; ++i)
			p[i + 80 * 24] = 0x20 | color;
	}
}

/* 往上滚 */
void 
scroll_screen_up()
{
	int i;
	short *p = VIDMEM;

	/* push scrolled out line to screen_down_stack */
	down_stack_top = line_stack_push(down_stack_p, down_stack_top, &down_stack_size, (uint8_t*)(&p[80*24]), 80*2);

	/* scroll screen */
	for (i = 80 * 24 - 1; i>=0; --i)
		p[i+80] = p[i];
	
	/* fill the first line */
	if(up_stack_size > 0)
	{
		/*pop a line from screen_up_stack */
		up_stack_top = line_stack_pop(up_stack_p, up_stack_top, &up_stack_size, (uint8_t*)(&p[0]), 80*2);
	}
	else
	{
		/* fill blank */
		for (i = 0; i < 80; ++i)
			p[i] = 0x20 | color;
	}
}

void
putc(int c)
{
	int tmp;

	if(!inited)
	{
		return;
	}

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
		/* this is the right time for us to fill the scrolling buffer */
		scroll_screen_down();
		cur_y = 24;
		cur_x = 0;
	}

	tmp = (cur_y << 6) + (cur_y << 4) + cur_x;
	set_cursor(tmp);
	vp = VIDMEM + tmp;
}

