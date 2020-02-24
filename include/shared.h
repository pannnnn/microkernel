#ifndef LMCVITTI_Y247PAN_SHARED
#define LMCVITTI_Y247PAN_SHARED

/*
 * Macro definition
 */
// in bytes/chars (each char is 1 byte)
#define MESSAGE_SIZE    4
#define NULL 0
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define CHECK_BIT(x, y) (x & (1 << y))
#define UNUSED(x) (void)(x)
#define INTERRUPT_COUNT 5
#define CTS_INTERRUPT_COUNT 2

#define INIT_TASK_PRIORITY 0
#define IDLE_TASK_PRIORITY 100
#define NAME_SERVER_PRIORITY 1
#define CLOCK_NOTIFIER_PRIORITY 2
#define CLOCK_SERVER_PRIORITY 3
#define UART1_RX_NOTIFIER_PRIORITY 10
#define UART1_RX_SERVER_PRIORITY 11
#define UART1_TX_NOTIFIER_PRIORITY 12
#define UART1_TX_SERVER_PRIORITY 13
#define UART2_RX_NOTIFIER_PRIORITY 14
#define UART2_RX_SERVER_PRIORITY 15
#define UART2_TX_NOTIFIER_PRIORITY 16
#define UART2_TX_SERVER_PRIORITY 17
#define COMMAND_SERVER_PRIORITY 20
#define SENSOR_EXECUTOR_PRIORITY 21
#define TERMINAL_EXECUTOR_PRIORITY 21
#define COMMAND_EXECUTOR_PRIORITY 21
#define CLIENT_TASK_PRIORITY 30

/*
 * Enum definition
 */
typedef enum
{
	CREATE = 0,
	TID,
	PID,
	YIELD,
    EXIT,
    DELETE,
    SEND,
    RECEIVE,
    REPLY,
    MALLOC,
    FREE,
    INTERRUPT,
    AWAIT_EVENT,
    GETC,
    PUTC
} SYS_CODE;

typedef enum
{
    TIMER_EVENT = 0,
    UART1_RX_EVENT,
    UART1_TX_EVENT,
    UART2_RX_EVENT,
    UART2_TX_EVENT,
    CTS_NEG,
    CTS_AST
} EVENT_TYPE;

/*
 * Struct definition
 */
typedef struct
{
    SYS_CODE code;
    unsigned int arg0;
    unsigned int arg1;
    unsigned int arg2;
    unsigned int arg3;
    unsigned int arg4;
} Args;

/*
 *  Shared global integer
 */
extern int percent_idle;
extern int event_notifier_registrar[INTERRUPT_COUNT];

/*
 * Function definition
 */
void charstr_copy(char *msg, char *buf, int length);
unsigned int read_timer();
unsigned int get_time_elaspsed(unsigned int start_time);

#endif
