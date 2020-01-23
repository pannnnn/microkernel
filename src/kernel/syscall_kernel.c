#include <k.h>

extern KernelState *_kernel_state;
extern void function_wrapper(void (*function)());


int _sys_create_td() 
{
    // TODO: Some logic to check if run out of task stack or task descriptor space
    TaskDescriptor *td = (TaskDescriptor *) _kernel_state->td_stack_addr;
    td->id = _kernel_state->curr_td_id++;
    return td->id;
}


TaskDescriptor *get_td(int id) 
{
    return (TaskDescriptor *) (_kernel_state->td_stack_addr + sizeof(TaskDescriptor) * id);
} 

int sys_create(PRIORITY priority, void (*function)()) 
{
    int td_id = _sys_create_td();

    TaskDescriptor *td = get_td(td_id);

    td->priority = priority;

    td->stack_pointer = USER_STACK_ADDR + td_id * USER_STACK_SIZE_PER_USER;
    int *stack_pointer = td->stack_pointer;
    // r0 serves as function pointer
    *(--stack_pointer) = function;
    // leave space for uninitialized r1-r12
    for (int i = 1; i++; i< NEW_TASK_REGS_SPACE) {
        *(--stack_pointer) = 0;
    }
    *(--stack_pointer) = function_wrapper;
    *(--stack_pointer) = USER_MODE_DEFAULT;

    td->state = READY;
    return 0;
}