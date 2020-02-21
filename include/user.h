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
#define UART_BUFFER_SIZE 4096
    #define UART_BUFFER_MASK 0xFFF
#define COMMAND_MAX_LEN 8
#define UART2_FIFO_SIZE 16
#define UART1_RX_SERVER_NAME "uart1_rx_server"
#define UART1_TX_SERVER_NAME "uart1_tx_server"
#define UART2_RX_SERVER_NAME "uart2_rx_server"
#define UART2_TX_SERVER_NAME "uart2_tx_server"
#define COMMAND_SERVER_NAME "command_server"

/*
 * Enum definition
 */
typedef enum 
{
    FROM_USER,
    FROM_SERVER,
    FROM_DEVICE
} UM_SOURCE;
typedef enum
{
    TICK = 0,
    TIME,
    DELAY,
    DELAY_UNTIL
} CR_TYPE;

typedef enum
{
    REGISTERAS = 0,
    WHOIS
} NS_OPERATION;

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
	unsigned char buffer[UART_BUFFER_SIZE];
	int start;
	int end;
} RingBuffer;

typedef struct {
    UM_SOURCE source;
    unsigned char data;
} UartMessage;

typedef struct {
    CR_TYPE type;
    int ticks;
} ClockMessage;

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


void uart1_rx_notifier();
void uart1_rx_server();
void uart1_tx_notifier();
void uart1_tx_server();
void uart2_rx_notifier();
void uart2_rx_server();
void uart2_tx_notifier();
void uart2_tx_server();
int Getc(int tid, int channel);
int Putc(int tid, int channel, char ch);

void train_server();
void command_server();

void init_hash_table(unsigned int (*hash_table)[2], int hash_size);
HashEntry *get(unsigned int (*hash_table)[2], int hash_size, const char *key);
void put(unsigned int (*hash_table)[2], int hash_size, const char *key, unsigned int value);
int remove(unsigned int (*hash_table)[2], int hash_size, char *key);
void dump_hash_map(unsigned int (*hash_table)[2]);

void function_wrapper(void (*function)());
void user_task_0();
void client_task();
void idle_task();

#endif