#include <k.h>

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

        int result = 0;
        switch (args->code) {
            case SYS_CODE.CREATE:
                PRIORITY priority = args->a0;
                void *function = (void*) args->a1;
                result = sys_create(priority, function);
                break;
            case SYS_CODE.EXIT:
                sys_exit();
                break;
            default:
                break;
        }
        
        ((int*)td->stack_pointer)[2] = result;
    }
}