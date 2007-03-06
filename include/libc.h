#ifndef __LIBC
#define __LIBC

int strlen(const char *buf);
int strnlen(const char *buf, int len);
char *reverse(char *buf);
char *itoa(unsigned int i,char *buf,int x );
char *strcat(char *dest, const char *src);
char *strcpy(char *dest, const char *src);

void kmemcpy(const void *scr,void *dest,unsigned long size);
void kmemset(void *dest,unsigned long size,char c);
/* do I/O read and write */
void outportb(int port, unsigned char val);
void outportw(int port, unsigned short val);
void outportl(int port, unsigned long val);
unsigned char inportb(int port);
unsigned short inportw(int port);
unsigned long inportl(int port);
void insl(int port, void *ptr, int cnt);

/* for syscall */
void enable(void);
void disable(void);
void debug_hlt(void);
void sys_wait(void);
#endif

