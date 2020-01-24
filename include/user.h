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


// int task_descriptor_array[MAX_DESCRIPTORS];
// int current_task;

int Create(int priority, void (*function)());

int MyTid();

int MyParentTid();

void Yield();

void Exit();

void function_wrapper(void (*function)());

void user_task_0();
void user_task_1();
void user_task_2();
void user_task_3();
void user_task_4();
void user_task_5();