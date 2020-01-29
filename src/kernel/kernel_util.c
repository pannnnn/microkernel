#include <kernel.h>

// declared as global variable in main.c
extern KernelState _kernel_state;

// gets the location of the task decriptor for the given task id
TaskDescriptor *get_td(int id) 
{
    return (TaskDescriptor *) (_kernel_state.kernel_stack_td_addr - sizeof(TaskDescriptor) * (id + 1));
} 

void task_return(TaskDescriptor *td, int return_val) {
    // load the return value into the memory location that will be
    // loaded into r0 when context-switching back to this task
    // when it is next scheduled; the task can then access the 
    // result as the return value of the function that caused this
    // interrupt
	((unsigned int*)td->stack_pointer)[2] = return_val;
}