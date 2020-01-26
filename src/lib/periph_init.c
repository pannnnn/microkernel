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