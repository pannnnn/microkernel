#include <lib_periph_bwio.h>
#include <lib_periph_timer.h>

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
	startTimer(TIM3, FREE, HI, (int) 0xFFFFFFFF);
}