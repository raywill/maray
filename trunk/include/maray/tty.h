#ifndef __MARAY_TTY
#define __MARAY_TTY

#include <i386/task.h>


void init_tty(void);
void putc(int c);
void print( const char* str );
void kprint( const char* str );
void clrscr(void);
void gotoxy(int x,int y);
void getxy(int * x,int *y);

void set_console(struct task_struct *new_con);
void save_console(struct task_struct *old_con);

#endif
