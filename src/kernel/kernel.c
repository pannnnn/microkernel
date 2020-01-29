#include <kernel.h>
#include <shared.h>
#include <queue.h>
#include <lib_periph_bwio.h>

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

void k_main() 
{
    while(1) {
        // get the new task from the scheduler
        int tid = schedule();

        if (tid == -1) return;

        // get the task descriptor from the task id
        TaskDescriptor *td = get_td(tid);

        Args *args;
        Args emptyArg = {};
        args = &emptyArg;

        // switch out of kernel mode and run the scheduled task
        unsigned int stack_pointer = leave_kernel(td->stack_pointer, &args);

        // store the new location of the task's stack pointer into
        // the task's task descriptor
        td->stack_pointer = stack_pointer;

        // determine what the kernel needs to do based on the system code
        // that comes from the user task that was just switched away from
        int result = -1;
        switch (args->code) {
            case CREATE:
                // create a new task; arg0 will hold the priority
                // arg1 will hold the pointer to the new task's main function
                // will return the id of the created task
                result = sys_create(args->arg0, (void *) args->arg1);
                task_return(td, result);
                // put the task back on the ready queue
                pq_insert(&_kernel_state.ready_queue, tid);
                break;
            case TID:
                // return the task id of the task that was just interrupted
                result = sys_tid();
                task_return(td, result);
                // put the task back on the ready queue
                pq_insert(&_kernel_state.ready_queue, tid);
                break;
            case PID:
                // return the task id of the parent of the task
                // that was just interrupted
                result = sys_pid();
                task_return(td, result);
                // put the task back on the ready queue
                pq_insert(&_kernel_state.ready_queue, tid);
                break;
            case YIELD:
                // does nothing
                sys_yield();
                break;
            case EXIT:
                // removes the exiting task from all queues
                sys_exit();
                break;
            case SEND:
                // try to send message
                sys_send(args->arg0, (int*)args->arg1, args->arg2, (int*)args->arg3, args->arg4);
                break;
            case RECEIVE:
                // attempt to receive a sent message
                sys_receive((int*)args->arg0, (int*)args->arg1, args->arg2);
                break;
            case REPLY:
                result = sys_reply(args->arg0, (int*)args->arg1, args->arg2);
                task_return(td, result);
                // put the task back on the ready queue
                pq_insert(&_kernel_state.ready_queue, tid);
                break;
            default:
                break;
        }
    }
}