#include <user.h>
#include <shared.h>
#include <lib_periph_init.h>
#include <lib_periph_bwio.h>

// provides a wrapper for the main function of the task
void function_wrapper(void (*function)()) {
    function();
    Exit();
}

// the main function of first task that is created (with priority 1)
void user_task_0() {
    bwprintf( COM2, "\n\rTask Id <%d> Parent Task Id <%d>\n\r", 0, -1);
    int name_server_tid = Create(99, NameServer);
    bwprintf( COM2, "\n\rCreated Name Server: <%d>\n\r", name_server_tid);
    int game_server1_tid = Create(1, game_server1);
    bwprintf( COM2, "\n\rCreated Game Server 1: <%d>\n\r", game_server1_tid);
    int game_server2_tid = Create(1, game_server2);
    bwprintf( COM2, "\n\rCreated Game Server 2: <%d>\n\r", game_server2_tid);
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

void game_server1() {
    bwprintf( COM2, "\n\rGame Server 1 Id <%d> Parent Task Id <%d>\n\r", MyTid(), MyParentTid());
    RegisterAs("game_server_1");
    bwprintf( COM2, "\n\rGame Server 2 Id: <%d>\n\r", WhoIs("game_server_2"));
}


void game_server2() {
    bwprintf( COM2, "\n\rGame Server 2 Id <%d> Parent Task Id <%d>\n\r", MyTid(), MyParentTid());
    RegisterAs("game_server_2");
    bwprintf( COM2, "\n\rGame Server 1 Id: <%d>\n\r", WhoIs("game_server_1"));
}


void performance_task() {
    for (int cache = 0; cache < 2; ++cache) {
        if (cache) cache_on();
        else cache_off();
        PF_EXECUTION_ORDER execution_orders[2] = {SENDER_FIRST, RECEIVER_FIRST};
        int message_size[3] = {4, 64, 256};
        for (int i = 0; i < 2; i++){
            for (int j = 0; j < 3; j++) {
                unsigned int start = read_timer();
                int priority = 0;
                int tid = Create(priority, pf_send_receive_test);
                PerformanceTest pf = {
                    .execution_order = execution_orders[i],
                    .message_size = message_size[j]
                };
                int result = Send(tid, (const char *) &pf, sizeof(pf), (char *) &pf, sizeof(pf));
                unsigned int end = read_timer();
            }
        }
    }
}

void pf_send_receive_test() {
    
}