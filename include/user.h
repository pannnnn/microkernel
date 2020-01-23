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

int myTid();

int myParentTid();

void Yield();

void Exit();

void function_wrapper(void (*function)());

void user_task_1();