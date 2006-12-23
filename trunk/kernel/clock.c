/*
  Name:	clock.c
  Copyright: GPL
  Author: Raywill
  Date: 22-12-06 14:49
  Description: Init and Update System Clock
*/

#include <maray/clock.h>
#include <i386/timer.h>
#include <libc.h>

/* What is BCD? Ref to man directory */
/* Generally speaking,use one byte to store two digits .Simple! */
#define BCD_TO_DEC(b) ( b = (b>>4)*10 + (b&15))/* Learn from Linux,really good :)*/
#define CMOS_READ(addr) rtc_cmos_read(addr)
/*
#define CMOS_READ(addr) ({	\
	outportb(0x70,addr);		\
	inportb(0x71);					\
})
*/
/* I don'tknow if '{}' are nessary.Just add it ,won't do any harm . */

extern sys_time real_tm;
extern sys_time start_tm;

unsigned 
char rtc_cmos_read(unsigned char addr)
{
	unsigned char val;
	outportb(0x70, addr);
	val = inportb(0x71);
	return val;
}

void 
init_system_clock(sys_time *tm)
{
/* 0x70 is the register address index port of RTC(Real Time)
0x71 is the data port
	*/
	do{
		tm->sec = CMOS_READ(0);
		tm->min = CMOS_READ(2);
		tm->hour = CMOS_READ(4);
		tm->week = CMOS_READ(6);
		tm->day = CMOS_READ(7);
		tm->month = CMOS_READ(8);
		tm->year = CMOS_READ(9);/*take care,year has only 2 bits*/
	}while(tm->sec!=CMOS_READ(0) );
	BCD_TO_DEC(tm->sec);
	BCD_TO_DEC(tm->min );
	BCD_TO_DEC(tm->hour);
	BCD_TO_DEC(tm->day);
	BCD_TO_DEC(tm->month);
	BCD_TO_DEC(tm->year);
	BCD_TO_DEC(tm->week);
	tm->week--;/* I prefer Sun=0,Mon=1,....,Sat=6 */
}

char *
timetostr(sys_time *ptm,char *tstr)
{
	/*What if I have a own version of sprintf()! */
	char *sp=tstr;
	char buf[3];	
	/*Should look like 6-1-8 */
	/* I would like to regard it as UTC time system ,ranges from 1950-2049*/
	if(ptm->year < 10){
		strcpy(sp,"200");
	}else{ 
		if(ptm->year < 50) 
			strcpy(sp,"20");
		else
			strcpy(sp,"19");
	}
	
	itoa((unsigned)ptm->year,buf,10);
	strcat(sp,buf);
	strcat(sp,"-");
	itoa((unsigned)ptm->month,buf,10);
	if ((unsigned)ptm->month < 10)
		strcat(sp, "0");
	strcat(sp,buf);
	strcat(sp,"-");
	itoa((unsigned)ptm->day,buf,10);
	if ((unsigned)ptm->day < 10)
		strcat(sp, "0");
	strcat(sp,buf);
	
	strcat(sp,"  ");/*seperator*/
	
	/* now the time */				
	itoa((unsigned)ptm->hour,buf,10);
	if ((unsigned)ptm->hour < 10)
		strcat(sp, "0");
	strcat(sp,buf);	
	strcat(sp,":");
	itoa((unsigned)ptm->min,buf,10);
	if ((unsigned)ptm->min < 10)
		strcat(sp, "0");
	strcat(sp,buf);
	strcat(sp,":");	
	itoa((unsigned)ptm->sec,buf,10);
	if ((unsigned)ptm->sec < 10)
		strcat(sp, "0");
	strcat(sp,buf);
	
	
	return tstr;
}


sys_time *
update_sys_time(int timefly)
{
	static int last_timefly = 0;
        int d_timefly;
        int d_sec;

        if ((d_timefly = timefly - last_timefly) < HZ)
                return &real_tm;

        d_sec = d_timefly / HZ;
        real_tm.sec += d_sec;
        if (real_tm.sec >= 60) {
                real_tm.min ++;
                real_tm.sec = real_tm.sec % 60;
        }
        if (real_tm.min >= 60) {
                real_tm.hour++;
                real_tm.min = real_tm.min - 60;
        }
        if (real_tm.hour == 24) {
                real_tm.hour = 0;
                real_tm.day ++;
        }

        last_timefly = timefly / HZ * HZ;

	return &real_tm;
}
