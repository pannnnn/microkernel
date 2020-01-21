typedef enum
{
    NORMAL = 0,
    IDLE
} MACHINE_STATE;

typedef struct 
{
    int machine_state;
    int td_stack_addr;
    int curr_td_id;
    int stack_pointer;
} KernelState;

typedef enum
{
	CREATE = 0,
    EXIT
} SYS_CODE;

typedef struct
{
    SYS_CODE code;
} Args;


void k_main();