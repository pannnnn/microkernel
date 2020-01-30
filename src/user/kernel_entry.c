#include <user.h>
#include <shared.h>

// defined in swi.S
extern int enter_swi(Args *args);

// create a new user task; enters kernel mode
int Create(int priority, void (*function)()){
	Args args;
	args.code = CREATE;
    args.arg0 = priority;
    args.arg1 = (unsigned int) function;
	return enter_swi(&args);
}

// gets this task's id; enters kernel mode
int MyTid() {
	Args args;
	args.code = TID;
    return enter_swi(&args);
}

// gets this task's parent's id; enters kernel mode
int MyParentTid() {
	Args args;
	args.code = PID;
	return enter_swi(&args);
}

// yields execution; enters kernel mode
void Yield() {
	Args args;
	args.code = YIELD;
    enter_swi(&args);
}

// ends the task; enters kernel mode
void Exit() {
	Args args;
	args.code = EXIT;
    enter_swi(&args);
}

int Send(int tid, const char *msg, int msglen, char *reply, int rplen) {
	Args args;
	args.code = SEND;
	args.arg0 = tid;
	args.arg1 = (unsigned int) msg;
	args.arg2 = msglen;
	args.arg3 = (unsigned int) reply;
	args.arg4 = rplen;
	return enter_swi(&args);
}

int Receive(int *tid, char *msg, int msglen) {
	Args args;
	args.code = RECEIVE;
	args.arg0 = (unsigned int) tid;
	args.arg1 = (unsigned int) msg,
	args.arg2 = msglen;
	return enter_swi(&args);
}

int Reply(int tid, const char *reply, int rplen) {
	Args args;
	args.code = REPLY;
	args.arg0 = tid;
	args.arg1 = (unsigned int) reply;
	args.arg2 = rplen;
	return enter_swi(&args);
}

char *Malloc(int size) {
	Args args;
	args.code = MALLOC;
    args.arg0 = size;
    return (char *) enter_swi(&args);
}

void Free(char *ptr) {
	Args args;
	args.code = FREE;
    args.arg0 = (unsigned int) ptr;
    enter_swi(&args);
}