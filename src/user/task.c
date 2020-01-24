#include <user.h>
#include <lib_periph_bwio.h>

void function_wrapper(void (*function)()) {
    function();
    Exit();
}

void user_task_0() {
    bwprintf( COM2, "\n\rTask Id <%d> Parent Task Id <%d>\n\r", 0, -1);
    int task_1_id = Create(0, user_task_test);
    bwprintf( COM2, "\n\rCreated: <%d>\n\r", task_1_id);
    int task_2_id = Create(0, user_task_test);
    bwprintf( COM2, "\n\rCreated: <%d>\n\r", task_2_id);
    int task_3_id = Create(2, user_task_test);
    bwprintf( COM2, "\n\rCreated: <%d>\n\r", task_3_id);
    int task_4_id = Create(2, user_task_test);
    bwprintf( COM2, "\n\rCreated: <%d>\n\r", task_4_id);
    bwprintf( COM2, "\n\rFirstUserTask: exiting\n\r");
}

void user_task_test() {
    int tid = MyTid();
    int pid = MyParentTid();
    bwprintf( COM2, "\n\rTask Id <%d> Parent Task Id <%d>\n\r", tid, pid);
    Yield();
    bwprintf( COM2, "\n\rTask Id <%d> Parent Task Id <%d>\n\r", tid, pid);
}