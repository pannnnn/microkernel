#include <kernel.h>
#include <shared.h>
#include <lib_ts7200.h>
#include <stdio.h>

extern KernelState _kernel_state;

static volatile int *timer2_clear = (int *) ( TIMER2_BASE + CLR_OFFSET );
static volatile int *uart1_control = (int *) (UART1_BASE + UART_CTLR_OFFSET);
static volatile int *uart1_flags = (int *) ( UART1_BASE + UART_FLAG_OFFSET );
static volatile int *uart1_status = (int *) ( UART1_BASE + UART_INTR_OFFSET );
static volatile int *uart1_data = (int *)( UART1_BASE + UART_DATA_OFFSET );
static volatile int *vic1_status = (int *) ( VIC1 + VICxIRQStatus );
static volatile int *vic2_status = (int *) ( VIC2 + VICxIRQStatus );

void interrupt_handler() {
    int tid = -1;
    if ( *vic1_status & TC2UI_MASK ) {
        // clear the interrupt
        *timer2_clear = 0;
        if (!event_notifier_awaited[TIMER_EVENT]) return;
        tid = event_notifier_registrar[TIMER_EVENT];
        TaskDescriptor *td = get_td(tid);
        td->state = READY;
        pq_insert(&_kernel_state.ready_queue, tid);
        set_result(td, (unsigned int) 1);
        // unawait event
        event_notifier_awaited[TIMER_EVENT] = 0;
    } else if ( *vic2_status & UART1_INT_MASK) {
        if ( *uart1_status & MIS ) {
            *uart1_control &= ~MSIEN_MASK;
            // clear the interrupt
            *uart1_status = 0;
            debug("INTR CTS: cts status change");
            if (!event_notifier_awaited[CTS_NEG] && !event_notifier_awaited[CTS_AST]) {
                error("INTR CTS: not task waits for MIS intr");
                return;
            };
            tid = event_notifier_registrar[UART1_TX_EVENT];
            TaskDescriptor *td = get_td(tid);
            td->state = READY;
            pq_insert(&_kernel_state.ready_queue, tid);
            set_result(td, (unsigned int) 0);
            if (event_notifier_awaited[CTS_NEG]) event_notifier_awaited[CTS_NEG] = 0;
            if (event_notifier_awaited[CTS_AST]) event_notifier_awaited[CTS_AST] = 0;
        } else if ( *uart1_status & RIS )  {
            if (!event_notifier_awaited[UART1_RX_EVENT]) return;
            debug("INTR RIS: ris status change");
            tid = event_notifier_registrar[UART1_RX_EVENT];
            TaskDescriptor *td = get_td(tid);
            td->state = READY;
            pq_insert(&_kernel_state.ready_queue, tid);
            set_result(td, (unsigned int) *uart1_data);
            event_notifier_awaited[UART1_RX_EVENT] = 0;
        } else if ( *uart1_status & TIS )  {
            if (!event_notifier_awaited[UART1_TX_EVENT]) return;
            debug("INTR TIS: tis status change");
            // Disable transmit interrupt in UART
            *uart1_control &= ~TIEN_MASK;
            tid = event_notifier_registrar[UART1_TX_EVENT];
            TaskDescriptor *td = get_td(tid);
            td->state = READY;
            pq_insert(&_kernel_state.ready_queue, tid);
            set_result(td, (unsigned int) 0);
            event_notifier_awaited[UART1_TX_EVENT] = 0;
        } else {
            error("something went wrong here")
        }
    } else if ( *vic2_status & UART2_INT_MASK) {

    }
}

void sys_await_event(int eventid) {
    int tid = _kernel_state.scheduled_tid;
    TaskDescriptor *td = get_td(tid);
    if ( eventid >= TIMER_EVENT && eventid <= UART2_TX_EVENT ) {
        event_notifier_awaited[eventid] = 1;
        td->state = EVENT_WAIT;
        if ( eventid == UART1_TX_EVENT || eventid == UART2_TX_EVENT ) {
            // Enable transmit interrupt in UART
            *uart1_control |= TIEN_MASK;
        }
    } else if ( eventid == CTS_AST ) {
        if ( *uart1_flags & CTS_MASK ) {
            highlight("AWAIT: cts already asserted");
            td->state = READY;
            pq_insert(&_kernel_state.ready_queue, tid);
            set_result(td, (unsigned int) 0);
        } else {
            debug("AWAIT: wait for cts asserted");
            event_notifier_awaited[eventid] = 1;
            *uart1_control |= MSIEN_MASK;
        }
    } else if ( eventid == CTS_NEG ) {
        if ( !(*uart1_flags & CTS_MASK) ) {
            highlight("AWAIT: cts already negated");
            td->state = READY;
            pq_insert(&_kernel_state.ready_queue, tid);
            set_result(td, (unsigned int) 0);
        } else {
            debug("AWAIT: wait for cts negated");
            event_notifier_awaited[eventid] = 1;
            *uart1_control |= MSIEN_MASK;
        }
    } else {
        pq_insert(&_kernel_state.ready_queue, tid);
        set_result(td, (unsigned int) -1);
    }
}