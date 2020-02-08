#include <kernel.h>
#include <shared.h>
#include <ds.h>
#include <stdio.h>
#include <lib_periph_init.h>

// defined in swi.S
extern int leave_kernel(int sp, Args **args);
extern int swi_exit(int sp, void** tf);

// defined as global variable in main.c
extern KernelState _kernel_state;


int _exists_live_task() {
    for (int i = 0; i < KERNEL_STACK_TD_LIMIT; i++) {
        if (_kernel_state.td_user_stack_availability[i] == 1) {
            return 1;
        }
    }
    for (int i = 0; i < INTERRUPT_COUNT; i++) {
        if (_kernel_state.await_queues[i].size != 0) {
            return 1;
        }
    }
    return 0;
}

// get the next task to be run
// reschedule the tasks in the priority queue
// returns the id of the task to be run
int schedule() 
{
    // nothing to be run; 
    if (_kernel_state.ready_queue.size == 0) return -1;
    
    // get the next scheduled task
    int scheduled_tid = pq_pop(&_kernel_state.ready_queue);
    
    // rescheduling
    _kernel_state.schedule_counter +=2;
    TaskDescriptor *td = get_td(scheduled_tid);
    td->scheduled_count = _kernel_state.schedule_counter;
    _kernel_state.scheduled_tid = scheduled_tid;
    
    // return the id of the task to be scheduled
    return scheduled_tid;
}

void pre_measure_performance(int tid) {
    if (tid == _kernel_state.performance_metric.idle_task_tid) {
        _kernel_state.performance_metric.idle_task_count_down_ticks = read_timer();
    }
}

void post_measure_performance(int tid) {
    if (tid == _kernel_state.performance_metric.idle_task_tid) {
        _kernel_state.performance_metric.idle_task_ticks += _kernel_state.performance_metric.idle_task_count_down_ticks - read_timer();
        // int integer = _kernel_state.performance_metric.idle_thousandth / 10;
        // int fractional = _kernel_state.performance_metric.idle_thousandth % 10;
        // log("I:<%d> F:<%d>", integer, fractional);
        // log("T:<%d>", _kernel_state.performance_metric.idle_thousandth);
        // log("I:<%d> F:<%d>", _kernel_state.performance_metric.kernel_ticks, _kernel_state.performance_metric.idle_task_ticks);
    }
    _kernel_state.performance_metric.kernel_ticks = _kernel_state.performance_metric.kernel_init_count_down_ticks - read_timer();
    _kernel_state.performance_metric.idle_thousandth =  1000 * _kernel_state.performance_metric.idle_task_ticks / _kernel_state.performance_metric.kernel_ticks;
    // int integer = _kernel_state.performance_metric.idle_thousandth / 10;
    // int fractional = _kernel_state.performance_metric.idle_thousandth % 10;
    // log("I:<%d> F:<%d>", integer, fractional);
    // usage_notification("CPU idle percentage <%d>.<%d>", integer, fractional);
}

void k_main() 
{
    while(1) {
        // get the new task from the scheduler
        int tid = schedule();

        // dump_queue(&_kernel_state.ready_queue);
        // debug("schedule tid <%d>", tid);
        if (tid == -1)  {
            if (_exists_live_task() == 0) return;
            continue;
        }

        // get the task descriptor from the task id
        TaskDescriptor *td = get_td(tid);

        Args *args;
        Args interruptArg = {.code = INTERRUPT};
        args = &interruptArg;

        // switch out of kernel mode and run the scheduled task

        pre_measure_performance(tid);

        unsigned int stack_pointer = leave_kernel(td->stack_pointer, &args);

        post_measure_performance(tid);

        // store the new location of the task's stack pointer into
        // the task's task descriptor
        td->stack_pointer = stack_pointer;

        // determine what the kernel needs to do based on the system code
        // that comes from the user task that was just switched away from
        unsigned int result;
        switch (args->code) {
            case CREATE:
                // create a new task; arg0 will hold the priority
                // arg1 will hold the pointer to the new task's main function
                // will return the id of the created task
                result = sys_create(args->arg0, (void *) args->arg1);
                set_result(td, result);
                // put the task back on the ready queue
                pq_insert(&_kernel_state.ready_queue, tid);
                break;
            case TID:
                // return the task id of the task that was just interrupted
                result = sys_tid();
                set_result(td, result);
                // put the task back on the ready queue
                pq_insert(&_kernel_state.ready_queue, tid);
                break;
            case PID:
                // return the task id of the parent of the task
                // that was just interrupted
                result = sys_pid();
                set_result(td, result);
                // put the task back on the ready queue
                pq_insert(&_kernel_state.ready_queue, tid);
                break;
            case YIELD:
                // does nothing
                sys_yield();
                // put the task back on the ready queue
                pq_insert(&_kernel_state.ready_queue, tid);
                break;
            case EXIT:
                // removes the exiting task from all queues
                sys_exit();
                break;
            case SEND:
                // try to send message
                sys_send(args->arg0, (char*)args->arg1, args->arg2, (char*)args->arg3, args->arg4);
                break;
            case RECEIVE:
                // attempt to receive a sent message
                sys_receive((int*)args->arg0, (char*)args->arg1, args->arg2);
                break;
            case REPLY:
                result = (unsigned int) sys_reply(args->arg0, (char*)args->arg1, args->arg2);
                set_result(td, result);
                // put the task back on the ready queue
                pq_insert(&_kernel_state.ready_queue, tid);
                break;
            case MALLOC:
                result = (unsigned int) sys_malloc(args->arg0);
                set_result(td, result);
                // put the task back on the ready queue
                pq_insert(&_kernel_state.ready_queue, tid);
                break;
            case FREE:
                sys_free((char *) args->arg0);
                // put the task back on the ready queue
                pq_insert(&_kernel_state.ready_queue, tid);
                break;
            case AWAIT_EVENT:
                // debug("Await event");
                sys_await_event((int) args->arg0);
                break;
            case INTERRUPT:
                interrupt_handler();
                pq_insert(&_kernel_state.ready_queue, tid);
                break;
            default:
                break;
        }
    }
}