/*
 * January 2020
 *
 * task.c - manage task-related functions
 */

#include <syscall.h>

extern int swi(int code);

int Create(int priority, void (*function)()){
	// allocate & initialize a task descriptor
	// use the given function pointer as the pointer to the
		// entry point of executable code (essentially a function with)
		// no arguments and no return value
	// when complete, task descriptor contains all info necessary to run
	// the task's main function
	Args args;
	args.code = SYS_CODE.CREATE;
    args.a0 = priority;
    args.a1 = function;
	swi(args);
	// return value:
		// if successful: the task id set for this task
		// if invalid priority: -1
		// if no more task descriptors available: -2 
}

int myTid() {
	// returns the task id of the calling task
}

int myParentTid() {
	// returns the task id of the task that created the calling task
}

void Yield() {
	// causes the calling task to pause execution
	// task is moved to the end of the priority queue
	// kernel mode is entered & tasks are rescheuled
}

void Exit() {
	// causes the calling task to cease all exection forever
	// removed from priority queues
	// removed from send queues
	// removed from receive queues
	// removed from event queues
	Args args;
	args.code = SYS_CODE.EXIT;
    swi(args);
}