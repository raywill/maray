/*
	Name:	kb.c
	Copyright: GPL
	Author: Raywill
	Date: 22-12-06
	Description: Keyboard Functions
*/

#include <maray/keymap.h>	/* 	nothing				*/
#include <maray/type.h>			/* 	define 'byte' */
#include <maray/kb.h>			/* 	init_kb()			*/
#include <i386/irq.h>			/*	enalbe_irq()	*/
#include <i386/vector.h>	/*	setvect()			*/
#include <libc.h>					/*	inportb()	,outportb()		*/
#include <i386/timer.h>
#include <maray/tty.h>

#define LED_SRL 1
#define LED_NUM 2
#define LED_CAP 4

static void kb_irq(void);
static void set_leds(unsigned char TYPE);

byte Scan_Tab[ ] =
{
	// '1', '2', '3', '4', '5',   '6',  '7', '8', '9', '0',
	0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,0x09,0x0a,0x0b,    
		//'Q','W','E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
		0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,
		//'A','S','D', 'F', 'G', 'H', 'J', 'K', 'L', ';',''',
		0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
		//'Z', 'X',  'C', 'V', 'B', 'N', 'M', '\,','.','/',
		0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32,0x33,0x34,0x35,
		//' '(space bar),'\n'(enter),'\t'(Tab)
		0x39,             0x1C,				0x0f
		
};
byte Disp_Tab[ ] =
{
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
		'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'',
		'Z', 'X',  'C', 'V', 'B', 'N', 'M', ',','.','/',
		' ','\n','\t'
};

void init_kb(void)
{
	vector_t v;
	v.eip = (unsigned)kb_irq;
	v.access_byte = 0x8E; /* present, ring 0, '386 interrupt gate D=32bit gate*/
	setvect(&v, 0x21);
	enable_irq(KB_IRQ);
	set_leds(LED_NUM);
}

static void kb_irq(void)
{
	char buf[2]={0};
	char numbuf[6];/*five digit will be enough for a int*/
	int i;
	
	
	byte scancode=inportb(0x60);
	//really ineffective
	for(i=0;i<sizeof(Scan_Tab);i++)
	{
		if(scancode==Scan_Tab[i]) break;
	}
	
	if(i<sizeof(Scan_Tab) )
	{
		buf[0]=Disp_Tab[i];
		buf[1]='\0';
		kprint(buf);//we'd better implement a putchar();
		if(buf[0]=='X') enable_timer();
		if(buf[0]=='R') disable_timer();
		if(buf[0]=='S') reset_sys();
	}
	outportb(0x20,0x20);
}


static void kb_wait()
{
/* while(1) will do a infinite loop if ret value is not equal to 0x02
this is really not nice.
while(1)
if( inportb(0x64)==0x02 )
return;
	*/
	int i;
	for(i=0;i<0xFFFFF;i++)
	{
		if( (inportb(0x64)& 0x02)==0 )/* keyboard buffer empty? */
			break;
	}
	
}
static void kb_ack()
{
	int i;
	for(i=0;i<0xFFFFF;i++)
	{
		/*Keyboard acknowledge to keyboard commands other than resend or ECHO.*/
		if (inportb(0x60) == 0xfa)
			break;
	}
}

static void set_leds(unsigned char TYPE)
{
/*
bits 3-7: Must be zero.
bit 2: Capslock LED (1 = on, 0 = off).
bit 1: Numlock LED (1 = on, 0 = off).
bit 0: Scroll lock LED (1 = on, 0 = off).
	*/
	static unsigned char leds=0;/*All leds turned off*/
	kb_wait();
	outportb(0x60,0xED);
	kb_wait();
	outportb(0x60, leds | TYPE & 0x07 );
}

void reset_sys(void)
{
	//outportb(0x64,0xD1);
	kb_wait();
	outportb(0x64,0xfc);
}
