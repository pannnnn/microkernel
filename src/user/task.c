#include <user.h>
#include <lib_periph_bwio.h>

// provides a wrapper for the main function of the task
void function_wrapper(void (*function)()) {
    function();
    Exit();
}

// the main function of first task that is created (with priority 1)
void user_task_0() {
    bwprintf( COM2, "\n\rTask Id <%d> Parent Task Id <%d>\n\r", 0, -1);
    int name_server_tid = Create(0, NameServer);
    bwprintf( COM2, "\n\rCreated Name Server: <%d>\n\r", name_server_tid);
    bwprintf( COM2, "\n\rFirstUserTask: exiting\n\r");
}

// the main function of the child tasks
// each child task gets its id and its parent's id
//      (two software interrupts into kernel mode) 
// and then prints them. Then it yields for one final
// software interrup into kernel mode, and prints its id
// and its parent's id once more before exiting
void user_task_test() {
    int tid = MyTid();
    int pid = MyParentTid();
    bwprintf( COM2, "\n\rTask Id <%d> Parent Task Id <%d>\n\r", tid, pid);
    Yield();
    bwprintf( COM2, "\n\rTask Id <%d> Parent Task Id <%d>\n\r", tid, pid);
}