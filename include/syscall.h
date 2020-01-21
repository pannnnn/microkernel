/*
 * January 2020
 *
 * task.h - definition for task-related functions
 */

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

#define MAX_DESCRIPTORS 10

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

int task_descriptor_array[MAX_DESCRIPTORS];
int current_task;

int Create(int priority, void (*function)());

int myTid();

int myParentTid();

void Yield();

void Exit();