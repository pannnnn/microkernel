#include <kernel.h>
#include <lib_periph_bwio.h>

extern KernelState _kernel_state;
extern void function_wrapper(void (*function)());

int _sys_create_td() 
{
    // TODO: Some logic to check if run out of task stack or task descriptor space
    TaskDescriptor *td = get_td(_kernel_state.id_counter);
    td->id = _kernel_state.id_counter++;
    td->scheduled_count = 0;
    td->pid = _kernel_state.scheduled_tid;
    return td->id;
}

int sys_create(int priority, void (*function)()) 
{
    int td_id = _sys_create_td();

    TaskDescriptor *td = get_td(td_id);

    td->priority = priority;

    td->stack_pointer = 
        (_kernel_state.user_stack_addr + td_id * _kernel_state.user_stack_size_per_user);
    unsigned int *stack_pointer_addr = (unsigned int *) td->stack_pointer;
    // user space function return address can be null set in initialization
    *(--stack_pointer_addr) = 0;
    // leave space for uninitialized r12-r1
    for (int i = 0; i < NEW_TASK_UNUSED_REGS_SPACE; i++) {
        *(--stack_pointer_addr) = 0;
    }
    // r0 serves as function pointer
    *(--stack_pointer_addr) = (unsigned int) function;
    *(--stack_pointer_addr) = (unsigned int) function_wrapper;
    *(--stack_pointer_addr) = USER_MODE_DEFAULT;
    td->stack_pointer = (unsigned int) stack_pointer_addr;

    td->state = READY;

    pq_insert(td_id);
    return td_id;
}

int sys_tid()
{
    return _kernel_state.scheduled_tid;
}

int sys_pid()
{
    int tid = _kernel_state.scheduled_tid;
    TaskDescriptor *td = get_td(tid);
    return td->pid;
}

void sys_yield() {}

void sys_exit()
{
    pq_remove(_kernel_state.scheduled_tid);
}