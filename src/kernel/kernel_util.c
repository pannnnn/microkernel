#include <kernel.h>

extern KernelState _kernel_state;

TaskDescriptor *get_td(int id) 
{
    return (TaskDescriptor *) (_kernel_state.kernel_stack_td_addr + sizeof(TaskDescriptor) * id);
} 
