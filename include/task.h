/*
 * January 2020
 *
 * task.h - definition for task-related functions
 */

#define MAX_DESCRIPTORS 10

// task states
#define UNDEFINED			-1
#define ACTIVE				0
#define READY  				1
#define ZOMBIE  			2
#define SEND_BLOCKED  		3
#define RECEIVE_BLOCKED  	4
#define REPLY_BLOCKED		5
#define EVENT_BLOCKED		6

typedef struct task_descriptor {
	// each struct has 7 ints and
	// therefore takes 28 bytes
	int parent_id;
	int priority;
	int next_ready;
	int next_send;
	int state;
	int stack_pointer;
	int execution_pointer;
} task_descriptor;

int task_descriptor_array[MAX_DESCRIPTORS];
int current_task_id;

int Create(int priority, void (*function)());
int myTid();
int myParentTid();
void Yield();
void Exit();
