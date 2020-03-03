#ifndef LMCVITTI_Y247PAN_STDIO
#define LMCVITTI_Y247PAN_STDIO

#define COM1	0
#define COM2	1

#define ANSI_PREFIX_CHARS_COUNT 5
#define LOG_PREFIX_CHARS_COUNT 8
#define ANSI_SUFFIX_CHARS_COUNT 4
#define NEW_LINE_CHARS_COUNT 2
#define BIG_ENOUGH_BUFFER_SIZE 1024

typedef enum
{
    DEBUG = 0,
    INFO,
    ERROR
} LOG_LEVEL;

typedef char *va_list;

#define __va_argsiz(t)	\
		(((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define va_start(ap, pN) ((ap) = ((va_list) __builtin_next_arg(pN)))

#define va_end(ap)	((void)0)

#define va_arg(ap, t)	\
		 (((ap) = (ap) + __va_argsiz(t)), *((t*) (void*) ((ap) - __va_argsiz(t))))

#endif