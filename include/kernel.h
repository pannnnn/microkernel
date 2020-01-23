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
    int machine_state;
    int curr_td_id;
    
    unsigned int kernel_stack_addr;
    unsigned int kernel_stack_td_addr;
    unsigned int user_stack_addr;
    unsigned int user_stack_size_per_user;
    unsigned int stack_pointer;
} KernelState;

typedef struct 
{
	int id;
	int pid;
	int priority;
	int next_ready;
	int next_send;
	void *next_td;
	TASK_STATE state;

	unsigned int stack_pointer;
} TaskDescriptor;

/*
 * Function definition
 */
void bootstrap();
void k_main();

int _sys_create_td();
int sys_create(int priority, void (*function)());
int sys_tid();
int sys_pid();
void sys_yield();
void sys_exit();
TaskDescriptor *get_td(int id);