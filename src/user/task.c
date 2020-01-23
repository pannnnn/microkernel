#include <u.h>

void function_wrapper(void (*function)()) {
    function();
    Exit();
}

void user_task_1() {
    return;
}