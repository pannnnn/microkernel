#include <kernel.h>
#include <lib_periph_bwio.h>

extern KernelState _kernel_state;

TaskDescriptor *get_td(int id) 
{
    return (TaskDescriptor *) (KERNEL_STACK_ADDR - KERNEL_STACK_TD_SIZE * (id + 1));
} 

void set_result(TaskDescriptor *td, unsigned int return_val) {
    // load the return value into the memory location that will be
    // loaded into r0 when context-switching back to this task
    // when it is next scheduled; the task can then access the 
    // result as the return value of the function that caused this
    // interrupt
	((unsigned int*)td->stack_pointer)[2] = return_val;
}