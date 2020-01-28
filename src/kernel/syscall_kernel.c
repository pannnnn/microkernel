#include <kernel.h>
#include <queue.h>
#include <lib_periph_bwio.h>

// declared as global variable in main.c
extern KernelState _kernel_state;

// defined in task.c
extern void function_wrapper(void (*function)());

// initializes the new task descriptor
int _sys_create_td(int priority) 
{
    if (priority < MIN_PRIORITY || priority > MAX_PRIORITY) return -1;
    if (_kernel_state.id_counter >= KERNEL_STACK_TD_CAP) return -2;
    
    TaskDescriptor *td = get_td(_kernel_state.id_counter);
    td->id = _kernel_state.id_counter++;
    // ensure the created task has a higher priority than its parent
    td->scheduled_count = _kernel_state.schedule_counter - 1;
    td->pid = _kernel_state.scheduled_tid;
    td->priority = priority;
    td->state = READY;
    td->stack_pointer = 
        (_kernel_state.user_stack_addr - td->id * _kernel_state.user_stack_size_per_user);
    return td->id;
}

// creates a new task by creating the task descriptor and
// adding the task id to the ready queue
int sys_create(int priority, void (*function)()) 
{
    int td_id = _sys_create_td(priority);

    if (td_id < 0) return td_id;

    TaskDescriptor *td = get_td(td_id);

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

    pq_insert(&_kernel_state.ready_queue, td_id);
    return td_id;
}

// returns the id of the currently "active" task
int sys_tid()
{
    return _kernel_state.scheduled_tid;
}

// returns the id of the parent of the currently "active" task
int sys_pid()
{
    int tid = _kernel_state.scheduled_tid;
    TaskDescriptor *td = get_td(tid);
    return td->pid;
}

void sys_yield() {}

// may need to do some clean up and relaim memory address
void sys_exit() {}