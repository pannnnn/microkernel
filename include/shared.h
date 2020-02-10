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
    AWAIT_EVENT
} SYS_CODE;

typedef enum
{
    TIMER_EVENT = 0,
    UART_EVENT
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

int percent_idle;

/*
 * Function definition
 */
void charstr_copy(char *msg, char *buf, int length);
unsigned int read_timer();
unsigned int get_time_elaspsed(unsigned int start_time);

#endif
