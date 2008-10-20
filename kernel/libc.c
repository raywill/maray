/*
	Name:	libc.c
	Copyright: GPL
	Author: Raywill
	Date: 22-12-06
	Description: Common Library Functions
*/


#include <libc.h>
#include <asmcmd.h>

int strlen(const char *buf)
{
	int i=0;
	while(*buf++)
		i++;
	return i;
}

int strnlen(const char *buf, int size)
{
	int n;
	for (n = 0; size > 0 && *buf++ != '\0'; size--)
		n++;
	return n;
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
void 
outportb(int port, unsigned char val)
{
	__asm__ __volatile__("outb %b0,%w1"
		:
		: "a"(val), "d"(port));
}
unsigned char
inportb(int port)
{
	unsigned char ret_val;

	__asm__ __volatile__("inb %w1,%0"
		: "=a"(ret_val)
		: "d"(port));
	return ret_val;
}

void
outportw(int port, unsigned short val)
{
    __asm__ __volatile("outw %0, %w1" : : "a" (val), "d" (port));
}

void
outportl(int port, unsigned long val)
{
    __asm__ __volatile("outl %0, %w1" : : "a" (val), "d" (port));
}

unsigned short
inportw(int port)
{
    unsigned short ret_val;
    __asm __volatile("inw %w1, %0" : "=a" (ret_val) : "d" (port));
    return ret_val;
}

unsigned long
inportl(int port)
{
    unsigned long ret_val;
    __asm __volatile("inl %w1, %0" : "=a" (ret_val) : "d" (port));
    return ret_val;
}

void
insl(int port, void *ptr, int cnt)
{
	__asm__ __volatile__("cld\n\trepne\n\tinsl"
			     : "=D" (ptr), "=c" (cnt)
			     : "d" (port), "0" (ptr), "1" (cnt)
			     : "memory", "cc");
}

void kmemcpy(const void *scr, void *dest,unsigned long size)
{
	const char *s = scr;
	char *d = dest;
	if(size<0) return;
	while(size--)
		*d++=*s++;
}

void memset(void *dest,char c, unsigned long size)
{
	char *d = dest;
	if(size<0) return;
	while(size--)
		*d++=c;
}

void kmemset(void *dest,unsigned long size,char c)
{
	char *d = dest;
	if(size<0) return;
	while(size--)
		*d++=c;
}
