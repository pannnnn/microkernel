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

typedef struct
{
    char content[BIG_ENOUGH_BUFFER_SIZE];
    int index;
} Movement_Buffer;

typedef struct
{
    char* content;
    int index;
} General_Buffer;

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

int update_idle(char *str, int size) {
    Pixels pixels = { .type=PX_IDLE_UPDATE, .size = size, .chars = str };
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
    // Never get freed since no clue of when program exit
    _log_level = DEBUG;
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
        charstr_copy(ANSI_WHITE, &fmt_buffer->content[0], ANSI_PREFIX_CHARS_COUNT);
        charstr_copy("[DEBUG]", &fmt_buffer->content[ANSI_PREFIX_CHARS_COUNT], LOG_PREFIX_CHARS_COUNT);
        break;
    case INFO:
        charstr_copy(ANSI_CYAN, &fmt_buffer->content[0], ANSI_PREFIX_CHARS_COUNT);
        charstr_copy("[ INFO]", &fmt_buffer->content[ANSI_PREFIX_CHARS_COUNT], LOG_PREFIX_CHARS_COUNT);
        break;
    case ERROR:
        charstr_copy(ANSI_RED, &fmt_buffer->content[0], ANSI_PREFIX_CHARS_COUNT);
        charstr_copy("[ERROR]", &fmt_buffer->content[ANSI_PREFIX_CHARS_COUNT], LOG_PREFIX_CHARS_COUNT);
        break;
    default:
        break;
    }
    charstr_copy(ANSI_RESET, &fmt_buffer->content[fmt_buffer->index], ANSI_SUFFIX_CHARS_COUNT);
    fmt_buffer->index += ANSI_SUFFIX_CHARS_COUNT;
}

void u_debug(char *fmt, ...)
{
    if (_log_level > DEBUG) return;
    
    General_Buffer format_buffer = {.content = Malloc(BIG_ENOUGH_BUFFER_SIZE), .index = ANSI_PREFIX_CHARS_COUNT + LOG_PREFIX_CHARS_COUNT};

    va_list va;
	va_start(va, fmt);
    _format(&format_buffer, fmt, va);
	va_end(va);

    _add_log_info(DEBUG, &format_buffer);
    PutStr(format_buffer.content, format_buffer.index);

    Free(format_buffer.content);
}

void u_info(char *fmt, ...)
{
    if (_log_level > INFO) return;

    General_Buffer format_buffer = {.content = Malloc(BIG_ENOUGH_BUFFER_SIZE), .index = ANSI_PREFIX_CHARS_COUNT + LOG_PREFIX_CHARS_COUNT};

    va_list va;
	va_start(va, fmt);
    _format(&format_buffer, fmt, va);
	va_end(va);

    _add_log_info(INFO, &format_buffer);

    PutStr(format_buffer.content, format_buffer.index);

    Free(format_buffer.content);
}

void u_error(char *fmt, ...)
{
    if (_log_level > ERROR) return;

    General_Buffer format_buffer = {.content = Malloc(BIG_ENOUGH_BUFFER_SIZE), .index = ANSI_PREFIX_CHARS_COUNT + LOG_PREFIX_CHARS_COUNT};

    va_list va;
	va_start(va, fmt);
    _format(&format_buffer, fmt, va);
	va_end(va);

    _add_log_info(ERROR, &format_buffer);
    PutStr(format_buffer.content, format_buffer.index);

    Free(format_buffer.content);
}

void _sprintf( General_Buffer *buffer, char *fmt, ...)
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
	
	char *str1 = "-----------------------------------------------------------------------------------------------------------------------------------------------------------------\r\n|********************************************************************************************************************    | Switch State                         |\r\n|                          *        *                                                                                *   |                                      |\r\n|**************************        *                                                                                  *  |   1   2   3   4   5   6   7   8   9  |\r\n|         A16        *            * *********************************************************************************  * |                                      |\r\n|********************            **                                   *           *                                   * *|                                      |\r\n|         A15                   *                                       *   *   *                                       *|                                      |\r\n|                               *                                         * * *                                         *|  10  11  12  13  14  15  16  17  18  |\r\n|                               *                                           *                                           *|                                      |\r\n|                               *                                           *                                           *|                                      |\r\n|                               *                                           *                                           *|                                      |\r\n|                               *                                           *                                           *| 153 154 155 156                      |\r\n|                               *                                           *                                           *|                                      |\r\n|                               *                                           *                                           *|                                      |\r\n|                               *                                           *                                           *|                                      |\r\n|                               *                                           *                                           *|--------------------------------------|\r\n";
	PutStr(str1, 2609);
	char *str2 = "|                               *                                           *                                           *| Sensor State                         |\r\n|                               *                                           *                                           *|                                      |\r\n|                               *                                           *                                           *|                                      |\r\n|                               *                                         * * *                                         *|                                      |\r\n|                               *                                       *   *   *                                       *|                                      |\r\n|                                **                                   *           *                                   * *|                                      |\r\n|**************                   * *********************************************************************************  * |                                      |\r\n|              *                   *                                                                                  *  |                                      |\r\n|********************               *                                                                                *   |                                      |\r\n|                    *               ********************************************************************************    |                                      |\r\n|**************************                          *                                              *                    |                                      |\r\n|                          *                           *                                          *                      |                                      |\r\n|***************************************************************************************************************         |                                      |\r\n|---------------------------------------------------------------------------------------------------------------------------------------------------------------|\r\n";
	PutStr(str2, 2283);
	char *str3 = "| >                                                                                                                      | Kernel Statistics                    |\r\n|                                                                                                                        |                                      |\r\n|                                                                                                                        | Clock:                               |\r\n|                                                                                                                        |                                      |\r\n|                                                                                                                        | Idle time:                           |\r\n|                                                                                                                        |                                      |\r\n|                                                                                                                        |                                      |\r\n|                                                                                                                        |                                      |\r\n|                                                                                                                        |                                      |\r\n|                                                                                                                        |                                      |\r\n|                                                                                                                        |                                      |\r\n|                                                                                                                        |                                      |\r\n|                                                                                                                        |                                      |\r\n|                                                                                                                        |                                      |\r\n|                                                                                                                        |                                      |\r\n|                                                                                                                        |                                      |\r\n|                                                                                                                        |                                      |\r\n|                                                                                                                        |                                      |\r\n|                                                                                                                        |                                      |\r\n|                                                                                                                        |                                      |\r\n|                                                                                                                        |                                      |\r\n-----------------------------------------------------------------------------------------------------------------------------------------------------------------\r\n";
	PutStr(str3, 3587);

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
    int client_tid, row, col, prev_row, switch_number;
	char switch_direction;
    General_Buffer movement_buffer = {.content = Malloc(BIG_ENOUGH_BUFFER_SIZE), .index = 0};
    while (Receive(&client_tid, (char *) &(pixels), sizeof(pixels))) {
        Reply(client_tid, (const char *) &(pixels), sizeof(pixels));
		switch (pixels.type)
		{
		case PX_REGULAR:
			for (int i = 0; i < pixels.size; i++) {
				Putc(_uart2_tx_server_tid, COM2, pixels.chars[i]);
			}
			break;
		case PX_SENSOR_UPDATE:
			row = sensor_gui.map[sensor_gui.index][0];
			col = sensor_gui.map[sensor_gui.index][1];
			if (sensor_gui.index == 0) prev_row = sensor_gui.map[SENSOR_TRACKED - 1][0];
			else prev_row = sensor_gui.map[sensor_gui.index - 1][0];
			if (++sensor_gui.index == SENSOR_TRACKED) sensor_gui.index = 0;
			_sprintf(&movement_buffer, SAVE_CURSOR "\033[%d;%dH" BACKSPACE "\033[%d;%dH> %s" RESTORE_CURSOR, 
			prev_row, col + 1, row, col, pixels.chars);
			for (int i = 0; i < movement_buffer.index; i++) {
				Putc(_uart2_tx_server_tid, COM2, movement_buffer.content[i]);
			}
			movement_buffer.index = 0;
			break;
		case PX_SWITCH_UPDATE:
			switch_number = (int) pixels.chars[1];
			switch_direction = pixels.chars[0] == SWITCH_STRAIGHT ? SWITCH_STRAIGHT_SYMBOL : SWITCH_BRANCH_SYMBOL;
			switch_number = switch_number > 152 ? switch_number - 135 : switch_number - 1;
			row = switch_gui.map[switch_number][0];
			col = switch_gui.map[switch_number][1];
			_sprintf(&movement_buffer, SAVE_CURSOR "\033[%d;%dH%c" RESTORE_CURSOR, row, col, switch_direction);
			for (int i = 0; i < movement_buffer.index; i++) {
				Putc(_uart2_tx_server_tid, COM2, movement_buffer.content[i]);
			}
			movement_buffer.index = 0;
			break;
		case PX_IDLE_UPDATE:
			_sprintf(&movement_buffer, SAVE_CURSOR "\033[%d;%dH%d.%d" RESTORE_CURSOR, 35, 135, (int) pixels.chars[0], (int) pixels.chars[1]);
			for (int i = 0; i < movement_buffer.index; i++) {
				Putc(_uart2_tx_server_tid, COM2, movement_buffer.content[i]);
			}
			movement_buffer.index = 0;
			break;
		default:
			break;
		}
    }
}