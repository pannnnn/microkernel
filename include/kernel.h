#ifndef LMCVITTI_Y247PAN_KERNEL
#define LMCVITTI_Y247PAN_KERNEL

/*
 * Include section
 */
#include <queue.h>

/*
 * Macro definition
 */
#define KERNEL_STACK_ADDR 0x2000000
    #define KERNEL_STACK_TD_COUNT 8192
    #define KERNEL_STACK_TD_LIMIT 128
    #define KERNEL_STACK_TD_SIZE 128
#define USER_STACK_ADDR 0x1F00000
    #define USER_STACK_HEAP_META_SIZE 12
    #define USER_STACK_S_HEAP_REGION 0x1E00000
        #define USER_STACK_S_HEAP_BLOCK_COUNT 65536
        #define USER_STACK_S_HEAP_BLOCK_SIZE 16
    #define USER_STACK_M_HEAP_REGION 0x1D00000
        #define USER_STACK_M_HEAP_BLOCK_COUNT 16384
        #define USER_STACK_M_HEAP_BLOCK_SIZE 64
    #define USER_STACK_L_HEAP_REGION 0x1C00000
        #define USER_STACK_L_HEAP_BLOCK_COUNT 4096
        #define USER_STACK_L_HEAP_BLOCK_SIZE 256
    #define USER_STACK_STACK_REGION 0x1B00000
        #define USER_STACK_STACK_SIZE_PER_USER 0x10000

#define SWI_HANDLER_ADDR   0x28

#define USER_MODE_DEFAULT 0x10

#define NEW_TASK_UNUSED_REGS_SPACE 12

#define MIN_PRIORITY 0
#define MAX_PRIORITY 100

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
    BLOCKED,
    EXITED, 
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
	int id;
	int pid;
    // this is an increasing time related id
    int scheduled_count;
	int priority;
	int next_ready;    // not used yet
	int next_send;     // not used yet
	void *next_td;     // not used yet
	TASK_STATE state;

    // message passing
    int *msg;           // message to send
    int msglen;         // lenght of msg
    int *rpl;           // reply buffer
    int rpllen;         // len of repl buf
    int *sender_id_ptr;     // for receiver; ptr to sender id
    Queue sending;      // ol implementation

	unsigned int stack_pointer;
} TaskDescriptor;

typedef struct 
{
    MACHINE_STATE machine_state;

    // task & scheduling mgmt
    int id_counter;
    int schedule_counter;
    int scheduled_tid;

    // task queues
    Queue ready_queue;      // pq implementation
    Queue send_queue;       // ul implementation
    Queue receive_queue;    // ul implementation
    Queue reply_queue;      // ul implementation

    int td_queue_size;
    int td_queue[KERNEL_STACK_TD_LIMIT + 1];
    int td_user_stack_availability[KERNEL_STACK_TD_LIMIT];

    char *s_block_used;
    char *s_block_unused;
    char *m_block_used;
    char *m_block_unused;
    char *l_block_used;
    char *l_block_unused;
} KernelState;

/*
 * Function definition
 */
void bootstrap();
void k_main();

// task creation & mgmt
int sys_create(int priority, void (*function)());
int sys_tid();
int sys_pid();
void sys_yield();
void sys_exit();

// message passing
int sys_send(int tid, int *msg, int msglen, int *reply, int rplen);
int sys_receive(int *tid, int *msg, int msglen);
int sys_reply(int tid, int *reply, int rplen);

int _sys_create_td(int priority);
TaskDescriptor *get_td(int id);
void task_return(TaskDescriptor *td, int return_val);

#endif