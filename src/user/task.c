#include <user.h>
#include <lib_periph_bwio.h>

void function_wrapper(void (*function)()) {
    function();
    Exit();
}

void user_task_1() {
    bwprintf( COM2, "\n\rUser task 1 started\n\r");
    return;
}