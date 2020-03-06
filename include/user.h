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
#define CLOCK_SERVER_NAME "clock_server"
#define UART_BUFFER_SIZE 16384
    #define UART_BUFFER_MASK 0x3FFF
#define COMMAND_MAX_LEN 8
#define UART1_RX_SERVER_NAME "uart1_rx_server"
#define UART1_TX_SERVER_NAME "uart1_tx_server"
#define UART2_RX_SERVER_NAME "uart2_rx_server"
#define UART2_TX_SERVER_NAME "uart2_tx_server"
#define TRACK_SERVER_NAME "track_server"
#define GUI_SERVER_NAME "gui_server"
#define UPDATE_EVERY_X_IDLES 20
#define BIG_ENOUGH_BUFFER_SIZE 1024

/*
 * Enum definition
 */
typedef enum {
    NS_REGISTERAS = 0,
    NS_WHOIS
} NAME_SERVER_OPERATION;

typedef enum {
    UM_FROM_USER,
    UM_FROM_SERVER,
    UM_FROM_DEVICE
} UART_MESSAGE_SOURCE;

typedef enum {
    CM_TICK = 0,
    CM_TIME,
    CM_DELAY,
    CM_DELAY_UNTIL
} CLOCK_MESSAGE_TYPE;

/*
 * Struct definition
 */
typedef struct
{
    char content[BIG_ENOUGH_BUFFER_SIZE];
    int index;
} General_Buffer;

typedef struct {
	char buffer[UART_BUFFER_SIZE];
	int start;
	int end;
} RingBuffer;

typedef struct {
    UART_MESSAGE_SOURCE source;
    unsigned char data;
} UartMessage;

typedef struct {
    CLOCK_MESSAGE_TYPE type;
    int ticks;
} ClockMessage;

typedef struct {
	NAME_SERVER_OPERATION operation;
    const char *name;
    int tid;
} NameServerMessage;

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

void u_debug(char *fmt, ...);
void u_info(char *fmt, ...);
void u_error(char *fmt, ...);
void u_sprintf(General_Buffer *buffer, char *fmt, ...);
int PutStr(char *str, int size);
int update_sensor(char *str, int size);
int update_switch(char *str, int size);
int update_idle(int percent_idle);
int update_clock(int hundredth_milsec);
void gui_server();
void track_server();

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