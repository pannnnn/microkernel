#include <k.h>

extern int enter_kernel();

extern KernelState *_kernel_state;

static void register_swi_handler()
{
    unsigned int *swiHandlerAddr = (unsigned int*) SWI_HANDLER_ADDR;
    *swiHandlerAddr = (unsigned int) enter_kernel;
}

static void init_kernel_state()
{
    KernelState kernel_state;
    kernel_state.machine_state = NORMAL;
    kernel_state.td_stack_addr = KERNEL_STACK_TD_ADDR;
    kernel_state.curr_td_id = 0;
    _kernel_state = &kernel_state;
}

static void create_first_user_task()
{
    PRIORITY priority = PRIORITY.MEDIUM;
    sys_create(priority, first_user_task);
}

void bootstrap()
{
    register_swi_handler();
    init_kernel_state();
    create_first_user_task();
}

