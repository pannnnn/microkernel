#include <kernel.h>
#include <syscall.h>

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

        int stack_pointer = leave_kernel(td->stack_pointer, args);

        td->stack_pointer = stack_pointer;

        switch (args.code) {
            case SYS_CODE.CREATE:
                break;
            case SYS_CODE.EXIT:
                break;
            default:
                break;
        }


    }
}