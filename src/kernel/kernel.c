#include <kernel.h>
#include <shared.h>
#include <lib_periph_bwio.h>

extern int leave_kernel(int sp, Args **args);
extern int swi_exit(int sp, void** tf);

extern KernelState _kernel_state;

int schedule() 
{
    if (_kernel_state.queue_size == 0) return -1;
    int scheduled_tid = pq_pop();
    _kernel_state.schedule_counter +=2;
    TaskDescriptor *td = get_td(scheduled_tid);
    td->scheduled_count = _kernel_state.schedule_counter;
    pq_insert(scheduled_tid);
    _kernel_state.scheduled_tid = scheduled_tid;
    return scheduled_tid;
}

void k_main() 
{
    while(1) {
        int tid = schedule();

        if (tid == -1) return;

        TaskDescriptor *td = get_td(tid);

        Args *args;
        Args emptyArg = {};
        args = &emptyArg;

        unsigned int stack_pointer = leave_kernel(td->stack_pointer, &args);

        td->stack_pointer = stack_pointer;

        int result = -1;
        switch (args->code) {
            case CREATE:
                result = sys_create(args->arg0, (void *) args->arg1);
                break;
            case TID:
                result = sys_tid();
                break;
            case PID:
                result = sys_pid();
                break;
            case YIELD:
                sys_yield();
                break;
            case EXIT:
                sys_exit();
                break;
            default:
                break;
        }

        // load value into r0
        ((unsigned int*)td->stack_pointer)[2] = result;
    }
}