#ifndef LMCVITTI_Y247PAN_SHARED
#define LMCVITTI_Y247PAN_SHARED

/*
 * Enum definition
 */
typedef enum
{
	CREATE = 0,
	TID,
	PID,
	YIELD,
    EXIT
} SYS_CODE;

typedef enum
{
    READY = 0,
    BLOCKED,
    EXITING
} TASK_STATE;

/*
 * Struct definition
 */
typedef struct
{
    SYS_CODE code;
    TASK_STATE state;
    unsigned int arg0;
    unsigned int arg1;
    unsigned int arg2;
    unsigned int arg3;
    unsigned int arg4;
} Args;

#endif