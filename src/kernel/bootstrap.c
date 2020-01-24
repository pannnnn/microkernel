#include <kernel.h>
#include <user.h>
#include <lib_periph_init.h>

extern int enter_kernel();

extern KernelState _kernel_state;

static void register_swi_handler()
{
    unsigned int *swiHandlerAddr = (unsigned int*) SWI_HANDLER_ADDR;
    *swiHandlerAddr = (unsigned int) enter_kernel;
}

static void init_kernel_state()
{
    _kernel_state.id_counter = 0;
    _kernel_state.scheduled_tid = -1;
    _kernel_state.queue_size = 0;

    _kernel_state.kernel_stack_addr = (unsigned int) &_kernel_state;
    _kernel_state.kernel_stack_td_addr = KERNEL_STACK_TD_ADDR;
    _kernel_state.user_stack_addr = USER_STACK_ADDR;
    _kernel_state.user_stack_size_per_user = USER_STACK_SIZE_PER_USER;

    _kernel_state.machine_state = NORMAL;
}

static void create_first_user_task()
{
    int priority = 1;
    sys_create(priority, user_task_0);
}

static void init_peripheral() {
    init_uart();
}

void bootstrap()
{
    init_peripheral();
    register_swi_handler();
    init_kernel_state();
    create_first_user_task();
}

