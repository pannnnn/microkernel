#ifndef LMCVITTI_Y247PAN_USER
#define LMCVITTI_Y247PAN_USER

/*
 * Include section
 */

/*
 * Macro definition
 */
#define HASHSIZE 101
#define STRING_MAX_LENGTH 256
#define NAME_SERVER_NAME "name_server"
#define RPS_SERVER_NAME "rps_server"
#define CLOCK_SERVER_NAME "clock_server"

/*
 * Enum definition
 */
typedef enum
{
    REGISTERAS = 0,
    WHOIS
} NS_OPERATION;

typedef enum
{
    TICK,
    TIME,
    DELAY,
    DELAY_UNTIL
} CR_Type;

typedef enum {
    CACHE_ON = 0,
    CACHE_OFF = 1
} CACHE_STATUS;

typedef enum
{
    SENDER_FIRST = 0,
    RECEIVER_FIRST
} PF_EXECUTION_ORDER;

/*
 * Struct definition
 */
typedef struct {
    CR_Type type;
    int ticks;
} ClockRequest;

typedef struct 
{
	NS_OPERATION operation;
    const char *name;
    int tid;
} NSMessage;

typedef struct _HashEntry
{
	struct _HashEntry *prev;
	struct _HashEntry *next;
    const char *key;
    unsigned int value;
} HashEntry;

// task creation
int Create(int priority, void (*function)());
int MyTid();
int MyParentTid();
void Yield();
void Exit();

// message passing
int Send(int tid, const char *msg, int msglen, char *reply, int rplen);
int Receive(int *tid, char *msg, int msglen);
int Reply(int tid, const char *reply, int rplen);
int AwaitEvent(int eventid);
char *Malloc(int size);
void Free(char *ptr);

void name_server();
int RegisterAs(const char *name);
int WhoIs(const char *name);

void clock_server();
void clock_notifier();
int Time(int tid);
int Delay(int tid, int ticks);
int DelayUntil(int tid, int ticks);

void init_hash_table(unsigned int (*hash_table)[2], int hash_size);
HashEntry *get(unsigned int (*hash_table)[2], int hash_size, const char *key);
void put(unsigned int (*hash_table)[2], int hash_size, const char *key, unsigned int value);
int remove(unsigned int (*hash_table)[2], int hash_size, char *key);
void dump_hash_map(unsigned int (*hash_table)[2]);

void function_wrapper(void (*function)());
void user_task_0();
void client_task();
void idle_task();

void sender_task_4();
void sender_task_64();
void sender_task_256();
void receiver_task_4();
void receiver_task_64();
void receiver_task_256();
void pf_send_receive_test(CACHE_STATUS cache_status, PF_EXECUTION_ORDER execution_order, int message_size);
void performance_task();

#endif