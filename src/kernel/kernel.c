#include <kernel.h>
#include <shared.h>
#include <lib_periph_bwio.h>

extern int leave_kernel(int sp, Args *args);
extern int swi_exit(int sp, void** tf);

int schedule() 
{
    return 0;
}

void k_main() 
{
    while(1) {
        int taskId = schedule();

        TaskDescriptor *td = get_td(taskId);

        Args *args;
        Args emptyArg = {};
        args = &emptyArg;

        unsigned int stack_pointer = leave_kernel(td->stack_pointer, args);

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

        ((unsigned int*)td->stack_pointer)[2] = result;
    }
}