/*
 * January 2020
 *
 * task.h - definition for task-related functions
 */

#define MAX_DESCRIPTORS 10

typedef struct task_descriptor {
	int task_id;
	int parent_id;
	int priority;
	int next_ready;
	int next_send;
	int state;
	int stack_pointer;
	int active;
} task_descriptor;

int task_descriptor_array[MAX_DESCRIPTORS];
int current_task;

int Create(int priority, void (*function)());

int myTid();

int myParentTid();

void Yield();

void Exit();