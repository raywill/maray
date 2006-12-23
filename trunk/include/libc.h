#ifndef __LIBC
#define __LIBC

int strlen(char *buf);
char *reverse(char *buf);
char *itoa(unsigned int i,char *buf,int x );
char	* strcat	(char *dest, const char *src);
char	* strcpy	(char *dest, const char *src);

void kmemcpy(unsigned char *scr,unsigned char *dest,unsigned long size);
void kmemset(void *dest,unsigned long size,char c);
/* do I/O read and write */
void outportb(unsigned port, unsigned val);
unsigned inportb(unsigned short port);

/* for syscall */
void enable(void);
void disable(void);
void debug_hlt(void);
void sys_wait(void);
#endif

