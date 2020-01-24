/*
 * Include section
 */

/*
 * Macro definition
 */
#define KERNEL_STACK_ADDR 0x2000000
    #define KERNEL_STACK_TD_ADDR 0x1F00000
#define USER_STACK_ADDR 0x1E00000
    #define USER_STACK_SIZE_PER_USER 0x10000

#define SWI_HANDLER_ADDR   0x28

#define USER_MODE_DEFAULT 0x10

#define NEW_TASK_UNUSED_REGS_SPACE 12

#define QUEUE_SIZE 1024

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
    BLOCKED
} TASK_STATE;


/*
 * Struct definition
 */
typedef struct 
{
	int id;
	int pid;
    // this is a increasing time related id
    int scheduled_count;
	int priority;
	int next_ready;
	int next_send;
	void *next_td;
	TASK_STATE state;

	unsigned int stack_pointer;
} TaskDescriptor;

typedef struct 
{
    MACHINE_STATE machine_state;

    int id_counter;
    int schedule_counter;
    int scheduled_tid;
    int queue_size;
    int queue[QUEUE_SIZE + 1];

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

int sys_create(int priority, void (*function)());
int sys_tid();
int sys_pid();
void sys_yield();
void sys_exit();

int _sys_create_td();
TaskDescriptor *get_td(int id);

void percolate_up(int index);
void percolate_down(int index);
void pq_insert(int tid);
int pq_pop();
void pq_remove(int tid) ;
int min_child(int index);
void dump_queue();