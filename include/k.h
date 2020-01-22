/*
 * Include section
 */
#include <u.h>

/*
 * Macro definition
 */
#define KERNEL_STACK_ADDR 0x2000000;
    #define KERNEL_STACK_TD_ADDR 0x1F00000;
#define USER_STACK_ADDR 0x1E00000;
    #define USER_STACK_SIZE_PER_USER 0x10000;

#define SWI_HANDLER_ADDR   0x28

#define USER_MODE_DEFAULT 0x10

#define NEW_TASK_REGS_SPACE 13

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

typedef enum
{
    VERY_HIGH = 0,
    HIGH,
	MEDIUM,
	LOW,
	VERY_LOW
} PRIORITY;

typedef enum
{
	CREATE = 0,
    EXIT
} SYS_CODE;


/*
 * Struct definition
 */
typedef struct 
{
    int machine_state;
    int td_stack_addr;
    int curr_td_id;
    int stack_pointer;
} KernelState;

typedef struct 
{
	int id;
	int parent_id;
	PRIORITY priority;
	int next_ready;
	int next_send;
	void *next_td;
	TASK_STATE state;

	int stack_pointer;
} TaskDescriptor;

typedef struct
{
    SYS_CODE code;
    int arg1;
    int arg2;
    int arg3;
} Args;


/*
 * Function definition
 */
void bootstrap();
void k_main();

int _sys_create_td();
int sys_create(PRIORITY priority, void (*function)());
TaskDescriptor *get_td(int id);