#include <lib_periph_timer.h>

int _switchTimer(int timer) {
	switch(timer) {
	case TIM1:
		return TIMER1_BASE;
	case TIM2:
		return TIMER2_BASE;
	case TIM3:
		return TIMER3_BASE;
	default:
		// illegal loc; error won't be pretty, but should be fast
		return  0xF000000;
	}
}

void _setCtrls(int timer_base, int mode, int speed) {
	int *ctrl, buf;
	ctrl = (int*)(timer_base + CTRL_OFFSET);

	buf = *ctrl;
	buf = mode ? buf | MODE_MASK : buf & ~MODE_MASK;
	buf = speed ? buf | CLKSEL_MASK : buf & ~CLKSEL_MASK;
	*ctrl = buf;	
}

void _setStartVal(int timer_base, int value) {
	int *timer_value;
	timer_value = (int*)(timer_base + LDR_OFFSET);
	*timer_value = value;
}

void enableTimer(int timer) {
	int *ctrl, buf;
	ctrl = (int*)(_switchTimer(timer) + CTRL_OFFSET);

	buf = *ctrl;
	buf = buf | ENABLE_MASK;
	*ctrl = buf;
}

void disableTimer(int timer) {
	int *ctrl, buf;

	ctrl = (int*)(_switchTimer(timer) + CTRL_OFFSET);

	buf = *ctrl;
	buf = buf & ~ENABLE_MASK;
	*ctrl = buf;
}

int getTimerVal(int timer) {
	/*
	 * Note: this function may need to be updated for TIM1 and TIM2
	 * 	so they don't try to read the whole 32 bits into the int
	 * 	and end up with garbage from the reserved sections
	 * 	I'm only using TIM3 for A0 so I'm not worrying about it too much
	 * 	at the moment
	 */
	volatile int *val_loc = (int*)(_switchTimer(timer) + VAL_OFFSET);
	return (int) *val_loc;
}

int startTimer(int timer, int mode, int speed, int value) {
	/* 
	 * timer: timer-identifying constant: TIM1, TIM2, or TIM3
	 * mode: the mode to execute the timer in
	 * 	TIM_PER: periodic
	 * 	TIM_FREE: free-running
	 * speed: the clock selection speed
	 * 	HI: 508kHz
	 * 	LO: 2kHz
	 * value: the value the timer is initialized at
	 */

	int timer_base = _switchTimer(timer);

	_setCtrls(timer_base, mode, speed);
	_setStartVal(timer_base, value);
	enableTimer(timer);
	return getTimerVal(timer);
}
