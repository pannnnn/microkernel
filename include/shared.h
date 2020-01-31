#ifndef LMCVITTI_Y247PAN_SHARED
#define LMCVITTI_Y247PAN_SHARED

/*
 * Macro definition
 */
#define NULL 0
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

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
    FREE
} SYS_CODE;

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

void charstr_copy(char *msg, char *buf, int length);
unsigned int read_timer();
unsigned int get_time_elaspsed(unsigned int start_time);

#endif