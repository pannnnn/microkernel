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
void function_wrapper(void (*function)()) 
{
    function();
    Exit();
}

void uart1_client() {
    int uart1_rx_server_tid = WhoIs(UART1_RX_SERVER_NAME);
    int uart1_tx_server_tid = WhoIs(UART1_TX_SERVER_NAME);
    for (int i = 0; i < 5; i++) {
        log("put a byte %d", Putc(uart1_tx_server_tid, COM1, 'a' + i));
    }
    // for (int i = 0; i < 26; i++) {
    //     log("get a byte %d", Getc(uart1_rx_server_tid, COM1));
    // }
}

void user_task_0() {
    putstr("\033[0m\n\r");
    log("Task Id <%d> Parent Task Id <%d>", 0, -1);
    int name_server_tid = Create(NAME_SERVER_PRIORITY, name_server);
    log("Created Name Server: <%d>", name_server_tid);
    int clock_server_tid = Create(CLOCK_SERVER_PRIORITY, clock_server);
    log("Created Clock Server: <%d>", clock_server_tid);
    int uart1_rx_server_tid = Create(UART1_RX_SERVER_PRIORITY, uart1_rx_server);
    log("Created UART1 RX Server: <%d>", uart1_rx_server_tid);
    int uart1_tx_server_tid = Create(UART1_TX_SERVER_PRIORITY, uart1_tx_server);
    log("Created UART1 TX Server: <%d>", uart1_tx_server_tid);
    int uart1_client_tid = Create(CLIENT_TASK_PRIORITY, uart1_client);
    log("Created UART1 Client: <%d>", uart1_client_tid);
    // int priority_base = 4, number_of_tasks = 4;
    // for (int i = 0; i < number_of_tasks; i++) {
    //     int client_task_tid = Create(priority_base + i, client_task);
    //     log("Created Client Task 1 with id: <%d>", client_task_tid);
    // }
    // int client_tid = -1;
    // int data[4][2] = {{10, 20}, {23, 9}, {33, 6}, {71, 3}};
    // ClientTestData client_test_data;
    // for (int i = 0; i < number_of_tasks; i++) {
    //     Receive(&client_tid, (char *) &client_test_data, sizeof(client_test_data));
    //     client_test_data.delay_interval = data[i][0];
    //     client_test_data.number_of_delays = data[i][1];
    //     Reply(client_tid, (const char *) &client_test_data, sizeof(client_test_data));
    // }
    log("FirstUserTask: exiting");
}

void client_task() {
    ClientTestData client_test_data;
    Send(MyParentTid(), (const char *) &client_test_data, sizeof(client_test_data), (char *)&client_test_data, sizeof(client_test_data));
    int clock_server_tid = WhoIs(CLOCK_SERVER_NAME);
    int tid = MyTid();
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
    // putstr("\033[s\033[HIdle Percentage:\033[u"); 
    while (1) {
        unsigned int percent_integer = percent_idle / 10;
        // unsigned int percent_fractional = percent_idle % 10;
        // usage_notification("%d.%d", percent_integer, percent_fractional);
        // log("Halting...");
        *halt;
    }
}

void malloc_test() {
    for (int i = 0; i < 16384; i++) {
        // log("Malloc 52: %x", Malloc(52));
        // Malloc(52);
        Free(Malloc(52));
    }
    log("Malloc 52: %x", Malloc(52));
    for (int i = 0; i < 4096; i++) {
        // log("Malloc 244: %x", Malloc(244));
        // Malloc(244);
        Free(Malloc(244));
    }
    log("Malloc 244: %x", Malloc(244));
    for (int i = 0; i < 1024; i++) {
        // log("Malloc 1024: %x", Malloc(1012));
        // Malloc(1012);
        Free(Malloc(1012));
    }
    log("Malloc 1024: %x", Malloc(1012));
}