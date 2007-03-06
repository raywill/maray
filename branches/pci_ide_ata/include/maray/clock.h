#ifndef __MARAY_CLOCK
#define __MARAY_CLOCK

typedef struct{
	/* Attention! month counts from ZERO,NOT one */
	unsigned char sec,min,hour,day,month;
	unsigned short year;
	unsigned char week;
} sys_time;

unsigned char rtc_cmos_read(unsigned char addr);
void init_system_clock(sys_time *tm);
sys_time *update_sys_time(unsigned int timefly);
char *timetostr(sys_time *tm,char *tstr);

#endif

