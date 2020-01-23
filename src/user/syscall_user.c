/*
 * January 2020
 *
 * task.c - manage task-related functions
 */

#include <user.h>
#include <shared.h>

extern int enter_swi(Args *args);

int Create(int priority, void (*function)()){
	// // allocate & initialize a task descriptor
	// // use the given function pointer as the pointer to the
	// 	// entry point of executable code (essentially a function with)
	// 	// no arguments and no return value
	// // when complete, task descriptor contains all info necessary to run
	// // the task's main function
	Args args;
	args.code = CREATE;
    args.arg0 = priority;
    args.arg1 = (unsigned int) function;
	return enter_swi(&args);
	// // return value:
	// 	// if successful: the task id set for this task
	// 	// if invalid priority: -1
	// 	// if no more task descriptors available: -2 
	// // find this task's id:
	// int i = 1;
	// int* new_td;
	// for (; i<10; i++) {
	// 	new_td = task_descriptor_array[i];
	// 	if (new_td->state == UNDEFINED) {
	// 		new_task_id = i;
	// 		break;
	// 	}
	// }
	// if (i == 10) return -2;

	// // new task descriptor identified: update info
	// new_td->parent_id = current_task_id;
	// // TODO: check priority is valid
	// new_td->priority = priority;
	// new_td->next_ready = 0x00000000;
	// new_td->next_send = 0x00000000;
	// new_td->state = READY;
	// // TODO: work out math for initializing stack pointer
	// new_td->stack_pointer = 0x00000000;
	// // TODO: I'm not completely sure what the function syntax in
	// 	// the declaration means; we want execution_pointer to be
	// 	// a pointer to the start of the function here
	// new_td->execution_pointer = function;

	// // TODO: add task to ready queue
	// // alternatively: ready queue could be auto-generated
	// 	// by iterating over all tasks and checking their states
	// 	// probably better to maintain a queue though 
	// return i;
}

int MyTid() {
	Args args;
	args.code = TID;
    enter_swi(&args);
	// returns the task id of the calling task
	// return current_task_id;
	return 0;
}

int MyParentTid() {
	Args args;
	args.code = PID;
    enter_swi(&args);
	// returns the task id of the task that created the calling task
	// int* current_task_descriptor;
	// current_task_descriptor = task_descriptor_array[current_task_id];
	// return current_task_descriptor->parent_id;
	return 0;
}

void Yield() {
	Args args;
	args.code = YIELD;
    enter_swi(&args);
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
	args.code = EXIT;
    enter_swi(&args);
}
