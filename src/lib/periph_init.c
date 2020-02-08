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
	timer_control = (int *)( TIMER2_BASE + CRTL_OFFSET );

	*timer_control = ENABLE_MASK | CLKSEL_MASK;
}


void init_interrupt() {
    volatile int *timer_clear, *timer_load, *timer_control;
    timer_clear = (int *) ( TIMER3_BASE + CLR_OFFSET );
    timer_load = (int *)( TIMER3_BASE + LDR_OFFSET );
	timer_control = (int *)( TIMER3_BASE + CRTL_OFFSET );
    *timer_clear = 0;
    *timer_load = VIC_TIMER_INTR_INTERVAL * CLOCK_PER_MILLISEC_508K;
    *timer_control = ENABLE_MASK | MODE_MASK | CLKSEL_MASK;

    volatile int *vic_mode_selection, *vic_control_clear, *vic_control;
    vic_mode_selection = (int *) ( VIC2 + VICxIntSelect );
    vic_control_clear = (int *) ( VIC2 + VICxIntEnClear );
    vic_control = (int *) ( VIC2 + VICxIntEnable );
    *vic_mode_selection = VIC_IRQ_MODE;
    *vic_control_clear = 0;
    *vic_control = 1 << TC3UI;
}

void disable_interrupt() {
    volatile int *timer_control, *timer_clear;
	timer_control = (int *)( TIMER3_BASE + CRTL_OFFSET );
    timer_clear = (int *) ( TIMER3_BASE + CLR_OFFSET );
    *timer_control = 0;
    *timer_clear = 0;

    volatile int *vic_control = (int *)( VIC2 + VICxIntEnable );
    *vic_control = 0;
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