#include <shared.h>
#include <lib_ts7200.h>
#include <stdio.h>

unsigned int read_timer() {
	unsigned int *timer_value;
	timer_value = (unsigned int *)( TIMER2_BASE + VAL_OFFSET );

	unsigned int value = *timer_value;
	return value;
}

unsigned int get_time_elaspsed(unsigned int start_time) {
	return (start_time - read_timer()) / CLOCK_PER_MILLISEC_508K;
}

void charstr_copy(char *msg, char *buf, int length) {
	for (int i=0; i < length; i++) {
		buf[i] = msg[i];
	}
}