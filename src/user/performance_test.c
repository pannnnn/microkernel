#include <user.h>
#include <shared.h>
#include <lib_periph_init.h>
#include <stdio.h>

static char *cache_status_string[2] = {"cache", "nocache"};
static char *execution_order_string[2] = {"S", "R"};

void performance_task() {
    bwprintf( COM2, "\n\rstart performance test\n\r");
    int message_size[3] = {4, 64, 256};
    for (int execution_order = 0; execution_order < 2; execution_order++){
        for (int j = 0; j < 3; j++) {
            pf_send_receive_test(0, execution_order, message_size[j]);
        }
    }
}

void pf_send_receive_test(CACHE_STATUS cache_status, PF_EXECUTION_ORDER execution_order, int message_size) {
        void *sender_task = NULL;
        void *receiver_task = NULL;
        int sender_priority = 0;
        int receiver_priority = 0;
        switch (execution_order)
        {
        case SENDER_FIRST:
            sender_priority = 2;
            receiver_priority = 1;
            break;
        case RECEIVER_FIRST:
            sender_priority = 1;
            receiver_priority = 2;
            break;
        default:
            break;
        }

        switch (message_size)
        {
        case 4:
            sender_task = sender_task_4;
            receiver_task = receiver_task_4;
            break;
        case 64:
            sender_task = sender_task_64;
            receiver_task = receiver_task_64;
            break;
        case 256:
            sender_task = sender_task_256;
            receiver_task = receiver_task_256;
            break;
        default:
            break;
        }
        
        unsigned int start_time = read_timer();
        Create(sender_priority, sender_task);
        Create(receiver_priority, receiver_task);
        bwprintf( COM2, "\n\r%s %s %d %d\n\r", cache_status_string[cache_status], execution_order_string[execution_order], message_size, get_time_elaspsed(start_time)/10);
}

void sender_task_4() {
    char test_string[4];
    for (int i = 0; i < 10000; i++) {
        Send(2, (const char *) test_string, 4, test_string, 4);
    }
}

void sender_task_64() {
    char test_string[64];
    for (int i = 0; i < 10000; i++) {
        Send(2, (const char *) test_string, 64, test_string, 64);
    }
}


void sender_task_256() {
    char test_string[256];
    for (int i = 0; i < 10000; i++) {
        Send(2, (const char *) test_string, 256, test_string, 256);
    }
}

void receiver_task_4() {
    int tid;
    char test_string[4];
    for (int i = 0; i < 10000; i++) {
        Receive(&tid, test_string, 4);
        Reply(1, test_string, 4);
    }
}

void receiver_task_64() {
    int tid;
    char test_string[64];
    for (int i = 0; i < 10000; i++) {
        Receive(&tid, test_string, 64);
        Reply(1, test_string, 64);
    }
}

void receiver_task_256() {
    int tid;
    char test_string[256];
    for (int i = 0; i < 10000; i++) {
        Receive(&tid, test_string, 256);
        Reply(1, test_string, 256);
    }
}