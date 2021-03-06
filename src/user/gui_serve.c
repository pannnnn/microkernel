#include <shared.h>
#include <user.h>
#include <lib_ts7200.h>
#include <stdio.h>
#include <ds.h>
#include <display.h>

typedef enum {
	PX_REGULAR = 0,
	PX_SENSOR_UPDATE,
	PX_SWITCH_UPDATE,
	PX_RAIL_UPDATE,
	PX_IDLE_UPDATE,
	PX_CLOCK_UPDATE,
	PX_LOG_UPDATE,
	PX_TIME_DIFF_UPDATE,
	PX_DISTANCE_DIFF_UPDATE
} PIXEL_TYPE;

typedef struct
{
	PIXEL_TYPE type;
    char *chars;
    int size;
} Pixels;

typedef struct
{
	int map[SENSOR_TRACKED][2];
    int index;
} Sensor_Gui;

typedef struct
{
	int map[SWITCH_TRACKED][2];
    int index;
} Switch_Gui;

static int _clock_server_tid = -1;
static int _uart2_tx_server_tid = -1;
static int _gui_server_tid = -1;
// this is used for user task log
static LOG_LEVEL _log_level = DEBUG;

int PutStr(char *str, int size) 
{
    Pixels pixels = { .type=PX_REGULAR, .size = size, .chars = str };
    Send(_gui_server_tid, (const char *) &pixels, sizeof(pixels), (char *)&pixels, sizeof(pixels));
    return 0;
}

int update_log(char *str, int size) {
    Pixels pixels = { .type=PX_LOG_UPDATE, .size = size, .chars = str };
    Send(_gui_server_tid, (const char *) &pixels, sizeof(pixels), (char *)&pixels, sizeof(pixels));
    return 0;
}

int update_sensor(char *str, int size) {
    Pixels pixels = { .type=PX_SENSOR_UPDATE, .size = size, .chars = str };
    Send(_gui_server_tid, (const char *) &pixels, sizeof(pixels), (char *)&pixels, sizeof(pixels));
    return 0;
}

int update_switch(char *str, int size) {
    Pixels pixels = { .type=PX_SWITCH_UPDATE, .size = size, .chars = str };
    Send(_gui_server_tid, (const char *) &pixels, sizeof(pixels), (char *)&pixels, sizeof(pixels));
    return 0;
}

int update_idle(int percent_idle) {
	char idles[2];
	idles[0] = percent_idle / 10;
	idles[1] = percent_idle % 10;
    Pixels pixels = { .type=PX_IDLE_UPDATE, .size = 2, .chars = idles };
    Send(_gui_server_tid, (const char *) &pixels, sizeof(pixels), (char *)&pixels, sizeof(pixels));
    return 0;
}

int update_clock(int hundredth_milsec) {
	int sec = 0, min = 0;
    char ticks[8];
	ticks[7] = '\0';
	sec = hundredth_milsec / 10;
	min = sec / 60;
	sec %= 60;
	ticks[6] = '0' + hundredth_milsec % 10;
	ticks[5] = ':';
	ticks[4] = '0' + sec % 10;
	ticks[3] = '0' + sec / 10;
	ticks[2] = ':';
	ticks[1] = '0' + min % 10;
	ticks[0] = '0' + min / 10;
    Pixels pixels = { .type=PX_CLOCK_UPDATE, .size = 8, .chars = ticks };
    Send(_gui_server_tid, (const char *) &pixels, sizeof(pixels), (char *)&pixels, sizeof(pixels));
    return 0;
}

int update_time_difference(int time_difference) {
	char difference[4];
	*((int *) difference) = time_difference;
    Pixels pixels = { .type=PX_TIME_DIFF_UPDATE, .size = 4, .chars = difference };
    Send(_gui_server_tid, (const char *) &pixels, sizeof(pixels), (char *)&pixels, sizeof(pixels));
    return 0;
}

int update_distance_difference(int distance_difference) {
	char difference[4];
	*((int *) difference) = distance_difference;
    Pixels pixels = { .type=PX_DISTANCE_DIFF_UPDATE, .size = 4, .chars = difference };
    Send(_gui_server_tid, (const char *) &pixels, sizeof(pixels), (char *)&pixels, sizeof(pixels));
    return 0;
}


int _a2d( char ch ) 
{
	if( ch >= '0' && ch <= '9' ) return ch - '0';
	if( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
	if( ch >= 'A' && ch <= 'F' ) return ch - 'A' + 10;
	return -1;
}

char _a2i( char ch, char **src, int base, int *nump ) 
{
	int num, digit;
	char *p;

	p = *src; num = 0;
	while( ( digit = _a2d( ch ) ) >= 0 ) {
		if ( digit > base ) break;
		num = num*base + digit;
		ch = *p++;
	}
	*src = p; *nump = num;
	return ch;
}

void _ui2a( unsigned int num, unsigned int base, char *bf ) 
{
	int n = 0;
	int dgt;
	unsigned int d = 1;
	
	while( (num / d) >= base ) d *= base;
	while( d != 0 ) {
		dgt = num / d;
		num %= d;
		d /= base;
		if( n || dgt > 0 || d == 0 ) {
			*bf++ = dgt + ( dgt < 10 ? '0' : 'a' - 10 );
			++n;
		}
	}
	*bf = 0;
}

void _i2a( int num, char *bf ) 
{
	if( num < 0 ) {
		num = -num;
		*bf++ = '-';
	}
	_ui2a( num, 10, bf );
}

void _getw( General_Buffer *fmt_buf, int n, char fc, char *bf ) 
{
	char ch;
	char *p = bf;

	while( *p++ && n > 0 ) n--;
	while( n-- > 0 ) fmt_buf->content[fmt_buf->index++] = fc;
	while( ( ch = *bf++ ) ) fmt_buf->content[fmt_buf->index++] = ch;
}

void _format ( General_Buffer *fmt_buf, char *fmt, va_list va ) 
{
    char bf[12];
	char ch, lz;
	int w;

	while ( ( ch = *(fmt++) ) ) {
		if ( ch != '%' )
            fmt_buf->content[fmt_buf->index++] = ch;
		else {
			lz = 0; w = 0;
			ch = *(fmt++);
			switch ( ch ) {
			case '0':
				lz = 1; ch = *(fmt++);
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				ch = _a2i( ch, &fmt, 10, &w );
				break;
			}
			switch( ch ) {
			case 0: return;
			case 'c':
                fmt_buf->content[fmt_buf->index++] = va_arg( va, char );
				break;
			case 's':
				_getw( fmt_buf ,w, 0, va_arg( va, char* ) );
				break;
			case 'u':
				_ui2a( va_arg( va, unsigned int ), 10, bf );
				_getw( fmt_buf, w, lz, bf );
				break;
			case 'd':
				_i2a( va_arg( va, int ), bf );
				_getw( fmt_buf ,w, lz, bf );
				break;
			case 'x':
				_ui2a( va_arg( va, unsigned int ), 16, bf );
				_getw( fmt_buf, w, lz, bf );
				break;
			case '%':
                fmt_buf->content[fmt_buf->index++] = ch;
				break;
			}
		}
	}
}

void _init_gui_server() 
{
	RegisterAs(GUI_SERVER_NAME);
    _clock_server_tid = WhoIs(CLOCK_SERVER_NAME);
    _uart2_tx_server_tid = WhoIs(UART2_TX_SERVER_NAME);
    _gui_server_tid = MyTid();
    _log_level = INFO;
}

void _init_map_data(Sensor_Gui *sensor_gui, Switch_Gui *switch_gui) {
	// hard code value :(
	sensor_gui->index = 0;
	for (int i = 0; i < SENSOR_TRACKED; i++) {
		sensor_gui->map[i][0] = 19+i;
		sensor_gui->map[i][1] = 125;
	}
	switch_gui->index = 0;
	for (int i = 0; i < SWITCH_TRACKED; i++) {
		switch_gui->map[i][0] = 6 + 4 * (i / 9);
		switch_gui->map[i][1] = 126 + (i % 9) * 4;
	}
}

void _add_log_info(LOG_LEVEL level, General_Buffer *fmt_buffer)
{
    switch (level)
    {
    case DEBUG:
        chars_copy(ANSI_WHITE, &fmt_buffer->content[0], ANSI_PREFIX_CHARS_COUNT);
        chars_copy("[DEBUG] ", &fmt_buffer->content[ANSI_PREFIX_CHARS_COUNT], LOG_PREFIX_CHARS_COUNT);
        break;
    case INFO:
        chars_copy(ANSI_CYAN, &fmt_buffer->content[0], ANSI_PREFIX_CHARS_COUNT);
        chars_copy("[ INFO] ", &fmt_buffer->content[ANSI_PREFIX_CHARS_COUNT], LOG_PREFIX_CHARS_COUNT);
        break;
    case ERROR:
        chars_copy(ANSI_RED, &fmt_buffer->content[0], ANSI_PREFIX_CHARS_COUNT);
        chars_copy("[ERROR] ", &fmt_buffer->content[ANSI_PREFIX_CHARS_COUNT], LOG_PREFIX_CHARS_COUNT);
        break;
    default:
        break;
    }
    chars_copy(ANSI_RESET, &fmt_buffer->content[fmt_buffer->index], ANSI_SUFFIX_CHARS_COUNT);
    fmt_buffer->index += ANSI_SUFFIX_CHARS_COUNT;
	fmt_buffer->content[fmt_buffer->index++] = '\0';
}

void u_debug(char *fmt, ...)
{
    if (_log_level > DEBUG) return;
    
    General_Buffer format_buffer = {.index = ANSI_PREFIX_CHARS_COUNT + LOG_PREFIX_CHARS_COUNT};

    va_list va;
	va_start(va, fmt);
    _format(&format_buffer, fmt, va);
	va_end(va);

    _add_log_info(DEBUG, &format_buffer);
    update_log(format_buffer.content, format_buffer.index);
}

void u_info(char *fmt, ...)
{
    if (_log_level > INFO) return;

    General_Buffer format_buffer = {.index = ANSI_PREFIX_CHARS_COUNT + LOG_PREFIX_CHARS_COUNT};

    va_list va;
	va_start(va, fmt);
    _format(&format_buffer, fmt, va);
	va_end(va);

    _add_log_info(INFO, &format_buffer);

    update_log(format_buffer.content, format_buffer.index);
}

void u_error(char *fmt, ...)
{
    if (_log_level > ERROR) return;

    General_Buffer format_buffer = {.index = ANSI_PREFIX_CHARS_COUNT + LOG_PREFIX_CHARS_COUNT};

    va_list va;
	va_start(va, fmt);
    _format(&format_buffer, fmt, va);
	va_end(va);

    _add_log_info(ERROR, &format_buffer);

    update_log(format_buffer.content, format_buffer.index);
}

void u_sprintf( General_Buffer *buffer, char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
    _format(buffer, fmt, va);
	va_end(va);
}

void loading_task() 
{   
    // dont put more than 4096 chars since buffer of com2 server is less than that
	// hard code piece :(
	PutStr(CLEAR_SCREEN, sizeof(CLEAR_SCREEN) - 1);
	PutStr(CURSOR_HOME, sizeof(CURSOR_HOME) - 1);
	
	char *str = "-----------------------------------------------------------------------------------------------------------------------------------------------------------------\r\n|********************************************************************************************************************    | Switch State                         |\r\n|                          *        *                                                                                *   |                                      |\r\n|**************************        *                                                                                  *  |   1   2   3   4   5   6   7   8   9  |\r\n|         A16        *            * *********************************************************************************  * |                                      |\r\n|********************            **                                   *           *                                   * *|                                      |\r\n|         A15                   *                                       *   *   *                                       *|                                      |\r\n|                               *                                         * * *                                         *|  10  11  12  13  14  15  16  17  18  |\r\n|                               *                                           *                                           *|                                      |\r\n|                               *                                           *                                           *|                                      |\r\n|                               *                                           *                                           *|                                      |\r\n|                               *                                           *                                           *| 153 154 155 156                      |\r\n|                               *                                           *                                           *|                                      |\r\n|                               *                                           *                                           *|                                      |\r\n|                               *                                           *                                           *|                                      |\r\n|                               *                                           *                                           *|--------------------------------------|\r\n|                               *                                           *                                           *| Sensor State                         |\r\n|                               *                                           *                                           *|                                      |\r\n|                               *                                           *                                           *|                                      |\r\n|                               *                                         * * *                                         *|                                      |\r\n|                               *                                       *   *   *                                       *|                                      |\r\n|                                **                                   *           *                                   * *|                                      |\r\n|**************                   * *********************************************************************************  * |                                      |\r\n|              *                   *                                                                                  *  |                                      |\r\n|********************               *                                                                                *   |                                      |\r\n|                    *               ********************************************************************************    |                                      |\r\n|**************************                          *                                              *                    |                                      |\r\n|                          *                           *                                          *                      |                                      |\r\n|***************************************************************************************************************         |                                      |\r\n|---------------------------------------------------------------------------------------------------------------------------------------------------------------|\r\n| >                                                                                                                                                             |\r\n|---------------------------------------------------------------------------------------------------------------------------------------------------------------|\r\n| Kernel Statistics                   | Current Speed:                          Distance To Destination:                   Next Sensor:                         |\r\n|                                     |                                                                                                                         |\r\n| Clock:                              | Start Velocity:                         Time Difference:                                                                |\r\n|                                     |                                                                                                                         |\r\n| Idle Time:                          | Calibrated Velocity:                    Distance Difference:                                                            |\r\n|                                     |                                                                                                                         |\r\n|                                     |                                                                                                                         |\r\n|                                     |                                                                                                                         |\r\n-----------------------------------------------------------------------------------------------------------------------------------------------------------------\r\n";
	PutStr(str, 6682);

	PutStr(CURSOR_COMMAND_LINE, sizeof(CURSOR_COMMAND_LINE) - 1);
}

// proprietor server
void gui_server() 
{
    _init_gui_server();
	Create(LOADING_TASK_PRIORITY, loading_task);
	Sensor_Gui sensor_gui;
	Switch_Gui switch_gui;
	_init_map_data(&sensor_gui, &switch_gui);
    Pixels pixels;
    int client_tid, row, col, prev_row, switch_number, difference;
	char switch_direction;
    General_Buffer format_buffer = {.index = 0};
	General_Buffer char_buffer = {.index = 0};
    while (Receive(&client_tid, (char *) &(pixels), sizeof(pixels))) {
		chars_copy(pixels.chars, char_buffer.content, pixels.size);
		char_buffer.index = pixels.size;

        Reply(client_tid, (const char *) &(pixels), sizeof(pixels));
		switch (pixels.type)
		{
		case PX_REGULAR:
			for (int i = 0; i < char_buffer.index; i++) {
				Putc(_uart2_tx_server_tid, COM2, char_buffer.content[i]);
			}
			break;
		case PX_SENSOR_UPDATE:
			row = sensor_gui.map[sensor_gui.index][0];
			col = sensor_gui.map[sensor_gui.index][1];
			if (sensor_gui.index == 0) prev_row = sensor_gui.map[SENSOR_TRACKED - 1][0];
			else prev_row = sensor_gui.map[sensor_gui.index - 1][0];
			if (++sensor_gui.index == SENSOR_TRACKED) sensor_gui.index = 0;
			u_sprintf(&format_buffer, SAVE_CURSOR "\033[%d;%dH" BACKSPACE "\033[%d;%dH>%s " RESTORE_CURSOR, 
			prev_row, col + 1, row, col, char_buffer.content, row);
			for (int i = 0; i < format_buffer.index; i++) {
				Putc(_uart2_tx_server_tid, COM2, format_buffer.content[i]);
			}
			format_buffer.index = 0;
			break;
		case PX_SWITCH_UPDATE:
			switch_number = (int) char_buffer.content[1];
			switch_direction = char_buffer.content[0] == SWITCH_STRAIGHT ? SWITCH_STRAIGHT_SYMBOL : SWITCH_BRANCH_SYMBOL;
			switch_number = switch_number > 152 ? switch_number - 135 : switch_number - 1;
			row = switch_gui.map[switch_number][0];
			col = switch_gui.map[switch_number][1];
			u_sprintf(&format_buffer, SAVE_CURSOR "\033[%d;%dH%c" RESTORE_CURSOR, row, col, switch_direction);
			for (int i = 0; i < format_buffer.index; i++) {
				Putc(_uart2_tx_server_tid, COM2, format_buffer.content[i]);
			}
			format_buffer.index = 0;
			break;
		case PX_IDLE_UPDATE:
			u_sprintf(&format_buffer, SAVE_CURSOR "\033[%d;%dH%d.%d" RESTORE_CURSOR, 37, 14, (int) char_buffer.content[0], (int) char_buffer.content[1]);
			for (int i = 0; i < format_buffer.index; i++) {
				Putc(_uart2_tx_server_tid, COM2, format_buffer.content[i]);
			}
			format_buffer.index = 0;
			break;
		case PX_CLOCK_UPDATE:
			// kernel will never run above 255 mins, easy fix if we have to;
			u_sprintf(&format_buffer, SAVE_CURSOR "\033[%d;%dH%s" RESTORE_CURSOR, 35, 10, char_buffer.content);
			for (int i = 0; i < format_buffer.index; i++) {
				Putc(_uart2_tx_server_tid, COM2, format_buffer.content[i]);
			}
			format_buffer.index = 0;
			break;
		case PX_LOG_UPDATE:
			u_sprintf(&format_buffer, SAVE_CURSOR "\033[100;4H" "%s" SCROLL_UP RESTORE_CURSOR, char_buffer.content);
			for (int i = 0; i < format_buffer.index; i++) {
				Putc(_uart2_tx_server_tid, COM2, format_buffer.content[i]);
			}
			format_buffer.index = 0;
			break;
		case PX_TIME_DIFF_UPDATE:
			difference = *((int *) char_buffer.content);
			u_sprintf(&format_buffer, SAVE_CURSOR "\033[35;98H" "%d        " RESTORE_CURSOR, difference);
			for (int i = 0; i < format_buffer.index; i++) {
				Putc(_uart2_tx_server_tid, COM2, format_buffer.content[i]);
			}
			format_buffer.index = 0;
			break;
		case PX_DISTANCE_DIFF_UPDATE:
			difference = *((int *) char_buffer.content);
			u_sprintf(&format_buffer, SAVE_CURSOR "\033[37;102H" "%d        " RESTORE_CURSOR, difference);
			for (int i = 0; i < format_buffer.index; i++) {
				Putc(_uart2_tx_server_tid, COM2, format_buffer.content[i]);
			}
			format_buffer.index = 0;
			break;
		default:
			break;
		}
    }
}