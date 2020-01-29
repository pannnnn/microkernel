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
    #define KERNEL_STACK_TD_ADDR 0x2000000
    #define KERNEL_STACK_TD_CAP 256
#define USER_STACK_ADDR 0x1F00000
    #define USER_STACK_SIZE_PER_USER 0x10000

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

    // stack ptrs
    unsigned int kernel_stack_addr;
    unsigned int kernel_stack_td_addr;
    unsigned int user_stack_addr;
    unsigned int user_stack_size_per_user;
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