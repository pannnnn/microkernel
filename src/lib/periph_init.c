#include <lib_ts7200.h>
#include <lib_periph_bwio.h>

// initializes the uarts so they can be communicated with
void init_uart() {
	bwsetspeed( COM1, 2400 );
	bwsetfifo( COM1, OFF );
	bwsetstopbits( COM1, 2 );

	bwsetspeed( COM2, 115200 );
	bwsetfifo( COM2, OFF );
	bwsetstopbits( COM2, 1 );
}

void init_timer() {
	int *timer_control;
	timer_control = (int *)( TIMER3_BASE + CRTL_OFFSET );

	*timer_control = ENABLE_MASK | CLKSEL_MASK;
}

void cache_on() {
    asm("mrc p15, 0, r0, c1, c0, 0\n\t"
        "orr r0, r0, #4096\n\t"
        "orr r0, r0, #4\n\t"
        "mcr p15, 0, r0, c1, c0, 0\n\t"
        "mcr p15, 0, r0, c7, c7, 0\n\t");
}

void cache_off() {
    asm("mrc p15, 0, r0, c1, c0, 0\n\t"
        "bic r0, r0, #4096\n\t"
        "bic r0, r0, #4\n\t"
        "mcr p15, 0, r0, c1, c0, 0\n\t"
        "mcr p15, 0, r0, c7, c7, 0\n\t");
}