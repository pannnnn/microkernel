#include <user.h>
#include <shared.h>
#include <lib_periph_init.h>
#include <rps.h>
#include <stdio.h>

typedef struct 
{
    int delay_interval;
    int number_of_delays;
} ClientTestData;

// provides a wrapper for the main function of the task
void function_wrapper(void (*function)()) {
    function();
    Exit();
}

void user_task_0() {
    log("\n\rTask Id <%d> Parent Task Id <%d>\n\r", 0, -1);
    int name_server_tid = Create(1, name_server);
    log("\n\rCreated Name Server: <%d>\n\r", name_server_tid);
    int clock_server_tid = Create(2, clock_server);
    log("\n\rCreated Clock Server: <%d>\n\r", clock_server_tid);
    int priority_base = 3, number_of_tasks = 4;
    int data[4][2] = {{10, 20}, {23, 9}, {33, 6}, {71, 3}};
    for (int i = 0; i < number_of_tasks; i++) {
        int client_task_tid = Create(priority_base + i, client_task);
        log("\n\rCreated Client Task 1 with id: <%d>\n\r", client_task_tid);
    }
    int client_tid = -1;
    ClientTestData client_test_data;
    for (int i = 0; i < number_of_tasks; i++) {
        Receive(&client_tid, (char *) &client_test_data, sizeof(client_test_data));
        client_test_data.delay_interval = data[i][0];
        client_test_data.number_of_delays = data[i][1];
        Reply(client_tid, (const char *) &client_test_data, sizeof(client_test_data));
    }
    log("\n\rFirstUserTask: exiting\n\r");
}

void client_task() {
    ClientTestData client_test_data;
    Send(MyParentTid(), (const char *) &client_test_data, sizeof(client_test_data), (char *)&client_test_data, sizeof(client_test_data));
    int clock_server_tid = WhoIs(CLOCK_SERVER_NAME);
    int tid = MyTid();
    for (int i = 0; i < client_test_data.number_of_delays; i++) {
        Delay(clock_server_tid, client_test_data.delay_interval);
        log("\n\rTask Id <%d> Delay Interval <%d> Number Of Delay <%d>\n\r", tid, client_test_data.delay_interval, i);
    }
}

void idle_task() {
    // int a = 1;
    while (1) {
        // if ((a % 100000000) == 0) {
        //     log("\n\rThis is a idle task {%d}\n\r", a);
        // }
        // a++;
    }
}