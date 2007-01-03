/*
	Name:	libc.c
	Copyright: GPL
	Author: Raywill
	Date: 22-12-06
	Description: Common Library Functions
*/


#include <libc.h>
#include <asmcmd.h>

int strlen(char *buf)
{
	int i=0;
	while(*buf++)
		i++;
	return i;
}
char *reverse(char *buf)
{
	char k;
	int i;
	int len=strlen(buf)-1;
	for(i=len;i>(len/2);i--)
	{
		k=buf[i];
		buf[i]=buf[len-i];
		buf[len-i]=k;
	}
	return buf;
}


char *itoa(unsigned int i,char *buf,int x)
{
	// const int x=16;
	int j;
	char *sp=buf;
	do{
		j=i%x;
		i=i/x;
		if(j<10)
			*sp++=j+'0';
		else
			*sp++=j+'0'+7;			
	}while(i!=0);
	*sp='\0';
	reverse(buf);
	return buf;
}

char	* strcpy	(char *dest, const char *src)
{
	char *h=dest;
	while(*src)
		*dest++ = *src++;
	*dest='\0';
	return h;
}
char	* strcat	(char *dest, const char *src)
{
	char *h=dest;
	char *snd;
	while(*dest++);
	
	snd=--dest;
	strcpy(snd,src);
	
	return h;
}
//////////////////////////////////////////
/*   			Do_IO													*/
//////////////////////////////////////////
void outportb(unsigned port, unsigned val)
{
	__asm__ __volatile__("outb %b0,%w1"
		:
		: "a"(val), "d"(port));
}
unsigned inportb(unsigned short port)
{
	unsigned char ret_val;

	__asm__ __volatile__("inb %1,%0"
		: "=a"(ret_val)
		: "d"(port));
	return ret_val;
}


void kmemcpy(const void *scr, void *dest,unsigned long size)
{
	const char *s = scr;
	char *d = dest;
	if(size<0) return;
	while(size--)
		*d++=*s++;
}
void kmemset(void *dest,unsigned long size,char c)
{
	char *d = dest;
	if(size<0) return;
	while(size--)
		*d++=c;
}
