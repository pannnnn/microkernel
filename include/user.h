#ifndef LMCVITTI_Y247PAN_USER
#define LMCVITTI_Y247PAN_USER

/*
 * Include section
 */

/*
 * Macro definition
 */
// #define MAX_DESCRIPTORS 10

/*
 * Enum definition
 */

/*
 * Struct definition
 */

// task creation
int Create(int priority, void (*function)());
int MyTid();
int MyParentTid();
void Yield();
void Exit();

// message passing
int Send(int tid, const char *msg, int msglen, char *reply, int rplen);
int Receive(int *tid, char *msg, int msglen);
int Reply(int tid, const char *reply, int rplen);

void function_wrapper(void (*function)());

void user_task_0();
void user_task_test();

#endif