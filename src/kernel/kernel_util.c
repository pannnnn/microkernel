#include <kernel.h>

// declared as global variable in main.c
extern KernelState _kernel_state;

// gets the location of the task decriptor for the given task id
TaskDescriptor *get_td(int id) 
{
    return (TaskDescriptor *) (_kernel_state.kernel_stack_td_addr - sizeof(TaskDescriptor) * (id + 1));
} 
