#include <kernel.h>
#include <user.h>
#include <shared.h>
#include <lib_periph_init.h>

// defined in swi.S
extern int enter_kernel();
extern int enter_interrupt();

// defined as a global variable in main.c
extern KernelState _kernel_state;

// global variable
int percent_idle;

// set enter_kernel as the software interrupt handler
static void register_swi_handler()
{
    unsigned int *swiHandlerAddr = (unsigned int*) SWI_HANDLER_ADDR;
    *swiHandlerAddr = (unsigned int) enter_kernel;
}

static void register_irq_handler() 
{
    unsigned int *irqHandlerAddr = (unsigned int*) IRQ_HANDLER_ADDR;
    *irqHandlerAddr = (unsigned int) enter_interrupt;
}

int _ready_queue_comparator1(int tid) {
    return get_td(tid)->priority;
}

int _ready_queue_comparator2(int tid) {
    return get_td(tid)->scheduled_count;
}

void _init_kernel_queues() 
{
    _kernel_state.ready_queue.size = 0;
    _kernel_state.ready_queue.index = 0;
    _kernel_state.ready_queue.get_arg1 = _ready_queue_comparator1;
    _kernel_state.ready_queue.get_arg2 = _ready_queue_comparator2;
    for (int i = 0; i < KERNEL_STACK_TD_LIMIT; i++) {
        _kernel_state.td_user_stack_availability[i] = 0;
    }
}

// initialize _kernel_state with starting values
static void init_kernel_state()
{
    _kernel_state.scheduled_tid = -1;
    _kernel_state.schedule_counter = 0;
    _kernel_state.num_active_tasks = 0;

    _init_kernel_queues();
    
    mem_init_all_heap_info();
    mem_init_heap_region(SMALL);
    mem_init_heap_region(MEDIUM);
    mem_init_heap_region(LARGE);

    _kernel_state.performance.task_start_time = read_timer();
    _kernel_state.performance.total_ticks = 0;
    _kernel_state.performance.idle_ticks = 0;
    _kernel_state.machine_state = NORMAL;
}

// create the first user task with priority 1
static void create_first_user_task()
{
    sys_create(INIT_TASK_PRIORITY, user_task_0);
    int idle_task_tid = sys_create(IDLE_TASK_PRIORITY, idle_task);
    _kernel_state.performance.idle_task_tid = idle_task_tid;
    percent_idle = 0;
}

// initialize the peripherals
static void init_peripheral() {
    init_timer();
    init_terminal();
    init_uart();
    init_interrupt();
    cache_on();
}

// initialize the system
void bootstrap()
{
    register_swi_handler();
    register_irq_handler();
    init_peripheral();
    init_kernel_state();
    create_first_user_task();
}

void clear_up() {
    clear_terminal();
    disable_interrupt();
}