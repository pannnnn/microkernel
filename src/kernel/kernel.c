#include <kernel.h>
#include <shared.h>
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
    if (_kernel_state.queue_size == 0) return -1;
    
    // get the next scheduled task
    int scheduled_tid = pq_pop();

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

        // put the task onto the appropriate queue 
        if (args->code != EXIT) pq_insert(tid);

        // determine what the kernel needs to do based on the system code
        // that comes from the user task that was just switched away from
        int result = -1;
        switch (args->code) {
            case CREATE:
                // create a new task; arg0 will hold the priority
                // arg1 will hold the pointer to the new task's main function
                // will return the id of the created task
                result = sys_create(args->arg0, (void *) args->arg1);
                break;
            case TID:
                // return the task id of the task that was just interrupted
                result = sys_tid();
                break;
            case PID:
                // return the task id of the parent of the task
                // that was just interrupted
                result = sys_pid();
                break;
            case YIELD:
                // does nothing
                sys_yield();
                break;
            case EXIT:
                // removes the exiting task from all queues
                sys_exit();
                break;
            default:
                break;
        }

        // load the return value into the memory location that will be
        // loaded into r0 when context-switching back to this task
        // when it is next scheduled; the task can then access the 
        // result as the return value of the function that caused this
        // interrupt
        ((unsigned int*)td->stack_pointer)[2] = result;
    }
}