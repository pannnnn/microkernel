#include <kernel.h>
#include <user.h>
#include <shared.h>
#include <lib_mem.h>
#include <lib_periph_init.h>

// defined in swi.S
extern int enter_kernel();

// defined as a global variable in main.c
extern KernelState _kernel_state;

// set enter_kernel as the software interrupt handler
static void register_swi_handler()
{
    unsigned int *swiHandlerAddr = (unsigned int*) SWI_HANDLER_ADDR;
    *swiHandlerAddr = (unsigned int) enter_kernel;
}

// initialize _kernel_state with starting values
static void init_kernel_state()
{
    _kernel_state.scheduled_tid = -1;

    _kernel_state.ready_queue.size = 0;
    _kernel_state.send_queue.size = 0;
    _kernel_state.receive_queue.size = 0;
    _kernel_state.reply_queue.size = 0;

    _kernel_state.schedule_counter = 0;

    mem_init_task_descriptors();
    mem_init_heap_region(SMALL);
    mem_init_heap_region(MEDIUM);
    mem_init_heap_region(LARGE);

    _kernel_state.machine_state = NORMAL;
}

// create the first user task with priority 1
static void create_first_user_task()
{
    int priority = 1;
    sys_create(priority, user_task_0);
}

// initialize the peripherals
static void init_peripheral() {
    init_uart();
}

// initialize the system
void bootstrap()
{
    init_peripheral();
    register_swi_handler();
    init_kernel_state();
    create_first_user_task();
}

