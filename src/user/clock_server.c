#include <shared.h>
#include <user.h>
#include <ds.h>
#include <lib_periph_bwio.h>

static int _clock_server_tid = -1;
static int _tid_to_ticks[QUEUE_SIZE] = {0};

void _init_clock_server() 
{
	RegisterAs(CLOCK_SERVER_NAME);
    _clock_server_tid = WhoIs(CLOCK_SERVER_NAME);
    for (int i = 0;i < QUEUE_SIZE; i++) {
        _tid_to_ticks[i] = 0;
    }
}

int _await_queue_comparator1(int tid) 
{
    return _tid_to_ticks[tid];
}

int _await_queue_comparator2(int dummy_value) 
{
    return 0;
}

int Time(int tid) 
{
    if (tid != _clock_server_tid) return -1;
    ClockMessage clock_request;
    clock_request.type = TIME;
    Send(_clock_server_tid, (const char *) &clock_request, sizeof(clock_request), (char *)&clock_request, sizeof(clock_request));
    return clock_request.ticks;
}

int Delay(int tid, int ticks) 
{
    if (tid != _clock_server_tid) return -1;
    ClockMessage clock_request;
    clock_request.type = DELAY;
    clock_request.ticks = ticks;
    Send(_clock_server_tid, (const char *) &clock_request, sizeof(clock_request), (char *)&clock_request, sizeof(clock_request));
    return clock_request.ticks;
}

int DelayUntil(int tid, int ticks) 
{
    if (tid != _clock_server_tid) return -1;
    ClockMessage clock_request;
    clock_request.type = DELAY_UNTIL;
    clock_request.ticks = ticks;
    Send(_clock_server_tid, (const char *) &clock_request, sizeof(clock_request), (char *)&clock_request, sizeof(clock_request));
    return clock_request.ticks;
}

void clock_notifier() 
{
    event_notifier_registrar[TIMER_EVENT] = MyTid();
    ClockMessage clock_request;
    clock_request.type = TICK;
    while (AwaitEvent(TIMER_EVENT) > -1) {
        // debug("awake every one tick (one sec)");
        int result = Send(_clock_server_tid, (const char *) &clock_request, sizeof(clock_request), (char *)&clock_request, sizeof(clock_request));
        if (result < 0) {
            error("something went wrong here");
        }
    }
}

void clock_server() 
{
    _init_clock_server();
    Queue blocked_tids = {.size = 0, .index = 0};
    blocked_tids.get_arg1 = _await_queue_comparator1;
    blocked_tids.get_arg2 = _await_queue_comparator2;
    ClockMessage clock_request;
    int clock_notifier_tid = Create(CLOCK_NOTIFIER_PRIORITY, clock_notifier);
    if (clock_notifier_tid < 0) {
        error("failed to create clock notifier");
    }
    int client_tid, ticks = 0;
    while (Receive(&client_tid, (char *) &clock_request, sizeof(clock_request))) {
        switch ( clock_request.type ) {
        case TICK:
            // check result error
            // debug("Ticks <%d>", ticks);
            ticks++;
            Reply(client_tid, (const char *) &clock_request, sizeof(clock_request));
            int blocked_tid = -1;
            clock_request.ticks = ticks;
            while ((blocked_tid = pq_get_min(&blocked_tids)) != -1) {
                if (_tid_to_ticks[blocked_tid] > ticks) {
                    break;
                }
                pq_pop(&blocked_tids);
                _tid_to_ticks[blocked_tid] = 0;
                Reply(blocked_tid, (const char *) &clock_request, sizeof(clock_request));
            }
            break;
        case TIME:
            clock_request.ticks = ticks;
            Reply(client_tid, (const char *) &clock_request, sizeof(clock_request));
            break;
        case DELAY:
            if (clock_request.ticks < 0) {
                clock_request.ticks = -2;
                Reply(client_tid, (const char *) &clock_request, sizeof(clock_request));
            } else {
                _tid_to_ticks[client_tid] = ticks + clock_request.ticks;
                pq_insert(&blocked_tids, client_tid);
            }
            break;
        case DELAY_UNTIL:
            if (clock_request.ticks < ticks) {
                clock_request.ticks = -2;
                Reply(client_tid, (const char *) &clock_request, sizeof(clock_request));
            } else {
                _tid_to_ticks[client_tid] = clock_request.ticks;
                pq_insert(&blocked_tids, client_tid);
            }
            break;
        default:
            break;
        }
    }
}