#include <syscall.h>

#define NEW_TASK_REGS_SPACE 13

int _sys_create_td();
int sys_create(int priority, void (*function)());
TaskDescriptor *get_td(int id);