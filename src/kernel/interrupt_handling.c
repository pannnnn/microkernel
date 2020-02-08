#include <kernel.h>
#include <shared.h>
#include <lib_ts7200.h>
#include <stdio.h>

extern KernelState _kernel_state;

static volatile int *timer_clear = (int *) ( TIMER2_BASE + CLR_OFFSET );
static volatile int *vic1_status = (int *) ( VIC1 + VICxIRQStatus );

int _check_bit(int bits, int position) {
    return (1 << position) & bits;
}

void interrupt_handler() {
    if (CHECK_BIT(*vic1_status, TC2UI) != 0) {
        int tid = -1;
        while ((tid = deque(&_kernel_state.await_queues[TIMER_EVENT])) != -1) {
            TaskDescriptor *td = get_td(tid);
            td->state = READY;
            pq_insert(&_kernel_state.ready_queue, tid);
            set_result(td, (unsigned int) 1);
        }
        // clear the interrupt
        *timer_clear = 1;
    }
}

void sys_await_event(int eventid) {
    int tid = _kernel_state.scheduled_tid;
    TaskDescriptor *td = get_td(tid);
    if (eventid == TIMER_EVENT) {
        td->state = EVENT_WAIT;
        enqueue(&_kernel_state.await_queues[eventid], tid);
    } else {
        pq_insert(&_kernel_state.ready_queue, tid);
        set_result(td, (unsigned int) -1);
    }
}