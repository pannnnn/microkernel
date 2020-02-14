#ifndef LMCVITTI_Y247PAN_KERNEL
#define LMCVITTI_Y247PAN_KERNEL

/*
 * Include section
 */
#include <ds.h>

/*
 * Macro definition
 */
#define KERNEL_STACK_ADDR 0x2000000
    #define KERNEL_STACK_TD_COUNT 1024
    #define KERNEL_STACK_TD_LIMIT 128
    #define KERNEL_STACK_TD_SIZE 1024
#define HEAP_ADDR 0x1F00000
    #define HEAP_META_SIZE 12
    #define S_HEAP_REGION 0x1F00000
        #define S_HEAP_BLOCK_COUNT 16384
        #define S_HEAP_BLOCK_SIZE 64
    #define M_HEAP_REGION 0x1E00000
        #define M_HEAP_BLOCK_COUNT 4096
        #define M_HEAP_BLOCK_SIZE 256
    #define L_HEAP_REGION 0x1D00000
        #define L_HEAP_BLOCK_COUNT 1024
        #define L_HEAP_BLOCK_SIZE 1024
#define USER_STACK_STACK_REGION 0x1C00000
    #define USER_STACK_STACK_SIZE_PER_USER 0x10000

#define SWI_HANDLER_ADDR   0x28
#define IRQ_HANDLER_ADDR   0x38
    #define INTERRUPT_COUNT 5

#define USER_MODE_DEFAULT 0x10

#define NEW_TASK_UNUSED_REGS_SPACE 12

#define MIN_PRIORITY 0
#define MAX_PRIORITY 100

// KEEP UPDATED
#define NUM_ALWAYS_LIVE_TASKS   4

/*
 * Enum definition
 */
typedef enum
{
    NORMAL = 0,
    IDLE
} MACHINE_STATE;

typedef enum
{
    READY = 0,
    SEND_WAIT,
    RECEIVE_WAIT,
    REPLY_WAIT,
    EVENT_WAIT
} TASK_STATE;

typedef enum
{
	SMALL = 0,
	MEDIUM,
	LARGE,
} HEAP_TYPE;

/*
 * Struct definition
 */
typedef struct
{
    int idle_task_tid;
    unsigned int idle_ticks;
    unsigned int total_ticks;
    unsigned int task_start_time;
} PerformanceMetric;

typedef struct {
    int *receiver_reserved_sid;
    union {
        char *sent_message;
        char *receive_message;
    };
    union {
        int sent_message_length;
        int receive_message_length;
    };
    char *replied_message;
    int replied_message_length;
} Message;

typedef struct
{
	int id;
	int pid;
    // this is an increasing time related id
    int scheduled_count;
	int priority;
	TASK_STATE state;
    
    // message passing
    Message message;
    // fifo queue implementation
    Queue inbox;    

	unsigned int stack_pointer;
} TaskDescriptor;

typedef struct {
    int heap_block_size;
    int heap_block_count;
    unsigned int heap_block_used;
    unsigned int heap_block_unused;
    unsigned int heap_region_addr;
} HeapInfo;

typedef struct _BlockMeta {
    struct _BlockMeta *prev;
    struct _BlockMeta *next;
    int id;
    HEAP_TYPE heap_type;
} BlockMeta;

typedef struct 
{
    MACHINE_STATE machine_state;

    // task & scheduling mgmt
    int schedule_counter;
    int scheduled_tid;
    int num_active_tasks;
    
    PerformanceMetric performance;

    // priority queue implementation
    Queue ready_queue;
    // fifo queue implementaiton, await event list (k3 timer only)
    Queue await_queues[INTERRUPT_COUNT];

    // heap management
    HeapInfo s_heap_info;
    HeapInfo m_heap_info;
    HeapInfo l_heap_info;

    int td_user_stack_availability[KERNEL_STACK_TD_LIMIT];
} KernelState;


/*
 * Function definition
 */
void bootstrap();
void k_main();
void clear_up();

// task creation
int sys_create(int priority, void (*function)());
int sys_tid();
int sys_pid();
void sys_yield();
void sys_exit();

// memory allocation
char *sys_malloc(int size);
void sys_free();

// message passing
void sys_send(int tid, char *msg, int msglen, char *reply, int rplen);
void sys_receive(int *tid, char *msg, int msglen);
int sys_reply(int tid, char *reply, int rplen);

// interrupt
void sys_await_event(int eventid);
void interrupt_handler();

// UART
void sys_getc(int tid, int channel);
void sys_putc(int tid, int channel, char ch);

TaskDescriptor *get_td(int id);
void set_result(TaskDescriptor *td, unsigned int return_val);

void mem_init_all_heap_info();
void mem_init_heap_region(HEAP_TYPE heap_type);
void mem_free(char *ptr);
char *mem_malloc(int size);
void dump_heap(HEAP_TYPE heap_type);

#endif