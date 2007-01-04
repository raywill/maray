#ifndef __STDIO
#define __STDIO

#include <stdarg.h>

#ifndef NULL
#define NULL ((void *) 0)
#endif

int kprintf(const char *fmt, ...);
int kvprintf(const char *fmt, va_list ap);

void kvprintfmt(void (*putch)(int, void*), void *putdat, const char *fmt, va_list ap);
void kprintfmt(void (*putch)(int, void*), void *putdat, const char *fmt, ...);

#endif
