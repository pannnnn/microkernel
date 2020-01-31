#include <user.h>
#include <shared.h>
#include <lib_periph_init.h>
#include <rps.h>
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
    // int game_server1_tid = Create(1, game_server1);
    // bwprintf( COM2, "\n\rCreated Game Server 1: <%d>\n\r", game_server1_tid);
    // int game_server2_tid = Create(1, game_server2);
    // bwprintf( COM2, "\n\rCreated Game Server 2: <%d>\n\r", game_server2_tid);
    int rps_server_tid = Create(2, rps_server_main);
    bwprintf( COM2, "\n\rCreated RPS Server: <%d>\n\r", rps_server_tid);
    for (int i=0; i<MAX_NUM_PLAYERS; i++) {
        int player_tid = Create(1, rps_player_main);
        bwprintf( COM2, "\n\rCreated RPS Player: <%d>\n\n", player_tid);
    }
    // int game_server2_tid = Create(3, game_server);
    // bwprintf( COM2, "\n\rCreated Game Server 2: <%d>\n\r", game_server2_tid);
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
