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
    #define S_HEAP_REGION 0x1E00000
        #define S_HEAP_BLOCK_COUNT 16384
        #define S_HEAP_BLOCK_SIZE 64
    #define M_HEAP_REGION 0x1D00000
        #define M_HEAP_BLOCK_COUNT 4096
        #define M_HEAP_BLOCK_SIZE 256
    #define L_HEAP_REGION 0x1C00000
        #define L_HEAP_BLOCK_COUNT 1024
        #define L_HEAP_BLOCK_SIZE 1024
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
    SEND_WAIT,
    RECEIVE_WAIT,
    REPLY_WAIT
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
typedef struct {
    union {
        int *sent_message;
        int *receive_message;
    };
    union {
        int sent_message_length;
        int receive_message_length;
    };
    int *replied_message;
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
    // queue implementation
    Queue inbox;

	unsigned int stack_pointer;
} TaskDescriptor;

typedef struct _BlockMeta {
    struct _BlockMeta *prev;
    struct _BlockMeta *next;
    int id;
    HEAP_TYPE heap_type;
} BlockMeta;

typedef struct {
    char *s_block_used;
    char *s_block_unused;
    char *m_block_used;
    char *m_block_unused;
    char *l_block_used;
    char *l_block_unused;
} Block;

typedef struct 
{
    MACHINE_STATE machine_state;

    // task & scheduling mgmt
    int schedule_counter;
    int scheduled_tid;

    // task queues
    Queue ready_queue;

    // malloced block
    Block block;

    int td_user_stack_availability[KERNEL_STACK_TD_LIMIT];
} KernelState;

typedef struct {
    int heap_block_size;
    int heap_block_count;
    char *heap_block_used;
    char *heap_block_unused;
    unsigned int heap_region_addr;
} HeapInfo;

/*
 * Function definition
 */
void bootstrap();
void k_main();

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
void sys_send(int tid, int *msg, int msglen, int *reply, int rplen);
void sys_receive(int *tid, int *msg, int msglen);
int sys_reply(int tid, int *reply, int rplen);

TaskDescriptor *get_td(int id);
void set_result(TaskDescriptor *td, unsigned int return_val);

void mem_init_task_descriptors();
void mem_init_heap_region(HEAP_TYPE heap_type);
void mem_free(char *ptr);
char *mem_malloc(int size);

#endif