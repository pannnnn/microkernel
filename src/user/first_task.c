#include <user.h>
#include <shared.h>
#include <lib_ts7200.h>
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
    putstr("\033[0m\n\r");
    log("Task Id <%d> Parent Task Id <%d>", 0, -1);
    int name_server_tid = Create(1, name_server);
    log("Created Name Server: <%d>", name_server_tid);
    int clock_server_tid = Create(2, clock_server);
    log("Created Clock Server: <%d>", clock_server_tid);
    int priority_base = 3, number_of_tasks = 4;
    int data[4][2] = {{10, 20}, {23, 9}, {33, 6}, {71, 3}};
    for (int i = 0; i < number_of_tasks; i++) {
        int client_task_tid = Create(priority_base + i, client_task);
        log("Created Client Task 1 with id: <%d>", client_task_tid);
    }
    int client_tid = -1;
    ClientTestData client_test_data;
    for (int i = 0; i < number_of_tasks; i++) {
        Receive(&client_tid, (char *) &client_test_data, sizeof(client_test_data));
        client_test_data.delay_interval = data[i][0];
        client_test_data.number_of_delays = data[i][1];
        Reply(client_tid, (const char *) &client_test_data, sizeof(client_test_data));
    }
    log("FirstUserTask: exiting");
}

void client_task() {
    ClientTestData client_test_data;
    Send(MyParentTid(), (const char *) &client_test_data, sizeof(client_test_data), (char *)&client_test_data, sizeof(client_test_data));
    int clock_server_tid = WhoIs(CLOCK_SERVER_NAME);
    int tid = MyTid();
    // log("Current ticks <%d>", Time(clock_server_tid));
    for (int i = 0; i < client_test_data.number_of_delays; i++) {
        int ticks = Delay(clock_server_tid, client_test_data.delay_interval);
        log("Task Id <%d>, Delay Interval <%d>, Delay Number <%d>, Ticks <%d>", tid, client_test_data.delay_interval, i+1, ticks);
    }
}

void idle_task() {
    volatile int *sys_sw_lock, *device_cfg, *halt;
    sys_sw_lock = (int *) ( SysSWLock );
    device_cfg = (int *) ( DeviceCfg );
    halt = (int *) ( Halt );
    *sys_sw_lock = *sys_sw_lock | 0xAA;
    *device_cfg = *device_cfg | 1;
    while (1) {
        // log("Halting...");
        *halt;
    }
}