#include <kernel.h>
#include <syscall.h>
#include <mem.h>
#include <k_syscall.h>

extern KernelState *_kernel_state;

int _sys_create_td() 
{
    // TODO: Some logic to check if run out of task stack or task descriptor space
    TaskDescriptor *td = (TaskDescriptor *) _kernel_state->td_stack_addr;
    td->id = _kernel_state->curr_td_id++;
    return td->id;
}

int sys_create(PRIORITY priority, void (*function)()) 
{
    int td_id = _sys_create_td();

    TaskDescriptor *td = get_td(td_id);

    td->priority = priority;

    td->stack_pointer = USER_STACK_ADDR + td_id * USER_STACK_SIZE_PER_USER;
    int *stack_pointer = td->stack_pointer;
    // leave space for uninitialized r0-r12
    for (int i = 0; i++; i< NEW_TASK_REGS_SPACE) {
        *(--stack_pointer) = 0;
    }
    *(--stack_pointer) = function;
    *(--stack_pointer) = USER_MODE_DEFAULT;

    td->state = READY;
    return 0;
}


TaskDescriptor *get_td(int id) 
{
    return (TaskDescriptor *) (_kernel_state->td_stack_addr + sizeof(TaskDescriptor) * id);
} 
