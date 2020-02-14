#include <kernel.h>
#include <shared.h>
#include <lib_ts7200.h>
#include <stdio.h>

extern KernelState _kernel_state;

static volatile int *timer2_clear = (int *) ( TIMER2_BASE + CLR_OFFSET );
static volatile int *uart1_status = (int *) ( UART1_BASE + UART_INTR_OFFSET );
static volatile int *uart1_flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
static volatile int *uart1_data = (int *)( UART1_BASE + UART_DATA_OFFSET );
static volatile int *vic1_status = (int *) ( VIC1 + VICxIRQStatus );
static volatile int *vic2_status = (int *) ( VIC2 + VICxIRQStatus );

void interrupt_handler() {
    int tid = -1;
    if ( *vic1_status & TC2UI_MASK ) {
        while ((tid = deque(&_kernel_state.await_queues[TIMER_EVENT])) != -1) {
            TaskDescriptor *td = get_td(tid);
            td->state = READY;
            pq_insert(&_kernel_state.ready_queue, tid);
            set_result(td, (unsigned int) 1);
        }
        // clear the interrupt
        *timer2_clear = 1;
    } else if ( *vic2_status & UART1_INT_MASK) {
        if (*uart1_status & RIS)  {
            while ((tid = deque(&_kernel_state.await_queues[UART1_RX_EVENT])) != -1) {
                TaskDescriptor *td = get_td(tid);
                td->state = READY;
                pq_insert(&_kernel_state.ready_queue, tid);
                if ( !( *uart1_flags & RXFF_MASK ) ) {
                    set_result(td, (unsigned int) -1);
                } else {
                    set_result(td, (unsigned int) *uart1_data);
                }
            }
        } else if (*uart1_status & TIS)  {

        } else if (*uart1_status & RTIS) {

        }
    } else if ( *vic2_status & UART2_INT_MASK) {

    }
}

void sys_await_event(int eventid) {
    int tid = _kernel_state.scheduled_tid;
    TaskDescriptor *td = get_td(tid);
    if (eventid == TIMER_EVENT || eventid == UART1_RX_EVENT || eventid == UART1_TX_EVENT || eventid == UART2_RX_EVENT || eventid == UART2_TX_EVENT) {
        td->state = EVENT_WAIT;
        enqueue(&_kernel_state.await_queues[eventid], tid);
    } else {
        pq_insert(&_kernel_state.ready_queue, tid);
        set_result(td, (unsigned int) -1);
    }
}