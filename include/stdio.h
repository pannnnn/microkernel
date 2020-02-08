#include <lib_ts7200.h>
#include <lib_periph_bwio.h>

#define printf(f, ...)              bwprintf(COM2, f, ## __VA_ARGS__)
#define putc(c)                     bwputc(COM2, c)
#define putstr(str)                 bwputstr(COM2, str)
#define getc()                      bwgetc(COM2)


#define log(f, ...)                 { printf(f, ## __VA_ARGS__); }
#define error(f, ...)               { putstr("\033[31m"); printf(f, ## __VA_ARGS__); putstr("\033[0m"); }
#if DEBUG
    #define debug(f, ...)           { putstr("\033[33m"); printf(f, ## __VA_ARGS__); putstr("\033[0m"); }
#else
    #define debug(f, ...)           { }
#endif