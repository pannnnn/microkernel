#ifndef LMCVITTI_Y247PAN_BWIO
#define LMCVITTI_Y247PAN_BWIO

#include <stdio.h>
#include <display.h>

/*
 * lib_periph_bwio.h
 */
#define ON	1
#define	OFF	0

int bwsetfifo( int channel, int state );

int bwsetspeed( int channel, int speed );

int bwsetstopbits( int channel, int bits );

int bwputc( int channel, char c );

int bwgetc( int channel );

int bwputx( int channel, char c );

int bwputstr( int channel, char *str );

int bwputr( int channel, unsigned int reg );

void bwputw( int channel, int n, char fc, char *bf );

void bwprintf( int channel, char *format, ... );


#define inline_printf(f, ...)       bwprintf(COM2, f, ## __VA_ARGS__)
#define printf(f, ...)              bwprintf(COM2, "%s:%d\t" f, __FILE__, __LINE__, ## __VA_ARGS__)
#define putc(c)                     bwputc(COM2, c)
#define putstr(str)                 bwputstr(COM2, str)
#define getc()                      bwgetc(COM2)

#define log(f, ...)                 { printf(ANSI_GREEN f ANSI_RESET "\r\n", ## __VA_ARGS__); }
#define error(f, ...)               { printf(ANSI_RED f ANSI_RESET "\r\n", ## __VA_ARGS__); }

#if DEBUG
    #define debug(f, ...)           { printf(f "\r\n", ## __VA_ARGS__); }
#else
    #define debug(f, ...)           { }
#endif

#endif