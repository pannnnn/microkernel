#include <lib_ts7200.h>
#include <lib_periph_bwio.h>
#include <stdio.h>

// initializes the uarts so they can be communicated with
void init_uart() {
	bwsetspeed( COM1, 2400 );
	bwsetfifo( COM1, OFF );
	bwsetstopbits( COM1, 2 );

	bwsetspeed( COM2, 115200 );
	bwsetfifo( COM2, ON );
	bwsetstopbits( COM2, 1 );
}

void init_timer() {
    volatile int *timer2_control;
	timer2_control = (int *)( TIMER3_BASE + CRTL_OFFSET );

	*timer2_control = ENABLE_MASK | CLKSEL_MASK;
}

void init_terminal() {
    // clear terminal 
    // putstr("\033[2J");
    // putstr("\033[2r");
}

void init_interrupt() {
    // volatile int *timer2_clear, *timer2_load, *timer2_control;
    // timer2_clear = (int *) ( TIMER2_BASE + CLR_OFFSET );
    // timer2_load = (int *)( TIMER2_BASE + LDR_OFFSET );
	// timer2_control = (int *)( TIMER2_BASE + CRTL_OFFSET );
    // *timer2_clear = 0;
    // *timer2_load = VIC_TIMER_INTR_INTERVAL * CLOCK_PER_MILLISEC_2K;
    // *timer2_control = ENABLE_MASK | MODE_MASK;

    // volatile int *vic1_int_select, *vic1_int_enable_clear, *vic1_int_enable;
    // vic1_int_select = (int *) ( VIC1 + VICxIntSelect );
    // vic1_int_enable_clear = (int *) ( VIC1 + VICxIntEnClear );
    // vic1_int_enable = (int *) ( VIC1 + VICxIntEnable );
    // *vic1_int_select = VIC_IRQ_MODE;
    // *vic1_int_enable_clear = 0;
    // *vic1_int_enable = (1 << TC2UI);

    volatile int *uart1_control;
    uart1_control = (int *) (UART1_BASE + UART_CTLR_OFFSET);
    *uart1_control = MSIEN_MASK | RIEN_MASK | UARTEN_MASK;

    volatile int *vic2_int_select, *vic2_int_enable_clear, *vic2_int_enable;
    vic2_int_select = (int *) ( VIC2 + VICxIntSelect );
    vic2_int_enable_clear = (int *) ( VIC2 + VICxIntEnClear );
    vic2_int_enable = (int *) ( VIC2 + VICxIntEnable );
    *vic2_int_select = VIC_IRQ_MODE;
    *vic2_int_enable_clear = 0;
    *vic2_int_enable = (1 << UART1_INTERRUPT);
}

void disable_interrupt() {
    // volatile int * vic1_int_enable_clear, *vic1_int_enable;
    // vic1_int_enable_clear = (int *) ( VIC1 + VICxIntEnClear );
    // vic1_int_enable = (int *) ( VIC1 + VICxIntEnable );
    // *vic1_int_enable = 0;
    // *vic1_int_enable_clear = 0;

    // volatile int *timer2_control, *timer2_clear;
	// timer2_control = (int *)( TIMER2_BASE + CRTL_OFFSET );
    // timer2_clear = (int *) ( TIMER2_BASE + CLR_OFFSET );
    // *timer2_control = 0;
    // *timer2_clear = 0;

    volatile int * vic2_int_enable_clear, *vic2_int_enable;
    vic2_int_enable_clear = (int *) ( VIC2 + VICxIntEnClear );
    vic2_int_enable = (int *) ( VIC2 + VICxIntEnable );
    *vic2_int_enable = 0;
    *vic2_int_enable_clear = 0;

    volatile int *uart1_control;
    uart1_control = (int *) (UART1_BASE + UART_CTLR_OFFSET);
    *uart1_control = 0;
}

void cache_on() {
    asm("stmfd sp!, {r0}\n\t"
        "mrc p15, 0, r0, c1, c0, 0\n\t"
        "orr r0, r0, #4096\n\t"
        "orr r0, r0, #4\n\t"
        "mcr p15, 0, r0, c1, c0, 0\n\t"
        "mcr p15, 0, r0, c7, c7, 0\n\t"
        "ldmfd sp!, {r0}\n\t");
}

void cache_off() {
    asm("stmfd sp!, {r0}\n\t"
        "mrc p15, 0, r0, c1, c0, 0\n\t"
        "bic r0, r0, #4096\n\t"
        "bic r0, r0, #4\n\t"
        "mcr p15, 0, r0, c1, c0, 0\n\t"
        "ldmfd sp!, {r0}\n\t");
}