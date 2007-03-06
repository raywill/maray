#ifndef __I386_TIMER
#define __I386_TIMER


#ifndef TIMER_FREQ
	#define TIMER_FREQ ((long)1193180)
	#define HZ 100			/* shedual per 10ms */
	#define SHED_RREQ (TIMER_FREQ/HZ+1)/* 20 ticks more per second */
#endif

void init_timer();
void enable_timer();
void disable_timer();

#endif
