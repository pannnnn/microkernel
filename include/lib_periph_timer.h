#ifndef LMCVITTI_Y247PAN_TIMER
#define LMCVITTI_Y247PAN_TIMER

#define TIM1 	11
#define TIM2 	12
#define TIM3 	13

#define LO 	0
#define HI 	1

#define FREE 	0
#define PER 	1

#define TIMER1_BASE 	0x80810000
#define TIMER2_BASE 	0x80810020
#define TIMER3_BASE 	0x80810080

#define LDR_OFFSET 	0x00000000	// 16/32 bits, RW
#define VAL_OFFSET	0x00000004	// 16/32 bits, RO
#define CTRL_OFFSET 	0x00000008	// 3 (important) bits, RW
	#define ENABLE_MASK	0x00000080
	#define MODE_MASK	0x00000040
	#define CLKSEL_MASK	0x00000008
#define CLR_OFFSET	0x0000000c	// no data, WO

#define MILISEC				508

void enableTimer(int timer);

void disableTimer(int timer);

int getTimerVal(int timer);

int startTimer(int timer, int mode, int speed, int value);

#endif