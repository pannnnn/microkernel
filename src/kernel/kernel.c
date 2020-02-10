#include <kernel.h>
#include <shared.h>
#include <ds.h>
#include <stdio.h>
#include <lib_periph_init.h>
#include <user.h>

// defined in swi.S
extern int leave_kernel(int sp, Args **args);
extern int swi_exit(int sp, void** tf);

// defined as global variable in main.c
extern KernelState _kernel_state;


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

void task_performance(int tid) {
    unsigned int curr_time = read_timer();
    unsigned int runtime = _kernel_state.performance.task_start_time - curr_time;
    if (runtime == 0) log("runtime 0 ticks from task <%d>", tid);
    _kernel_state.performance.task_start_time = curr_time;
    
    // increment idle counter if just-finished task was idle task
    if (tid == _kernel_state.performance.idle_task_tid) _kernel_state.performance.idle_ticks += runtime;
    _kernel_state.performance.total_ticks += runtime;

    unsigned int percent_idle = (_kernel_state.performance.idle_ticks) / (_kernel_state.performance.total_ticks / 100);

    // print percent idle time if just-finished task was idle task
    if (tid == _kernel_state.performance.idle_task_tid) usage_notification("percent idle: %d", percent_idle);
}

void k_main() 
{
    while(1) {
        // get the new task from the scheduler
        int tid = schedule();

        //if (tid == -1) {
        if (tid == -1 || tid == _kernel_state.performance.idle_task_tid) {
            if (_kernel_state.num_active_tasks <= NUM_ALWAYS_LIVE_TASKS) {
               log("Exiting");
               return;
            }
            if (tid == -1) {debug("no ready tasks"); continue;}
        }

        // get the task descriptor from the task id
        TaskDescriptor *td = get_td(tid);

        Args *args;
        Args interruptArg = {.code = INTERRUPT};
        args = &interruptArg;

        // measure time of kernel activity & print idle percentage
        task_performance(-1);

        // switch out of kernel mode and run the scheduled task
        unsigned int stack_pointer = leave_kernel(td->stack_pointer, &args);

        // measure time of task activity & print idle percentage
        task_performance(tid);

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