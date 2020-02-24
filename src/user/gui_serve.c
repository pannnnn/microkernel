#include <shared.h>
#include <user.h>
#include <lib_ts7200.h>
#include <stdio.h>
#include <ds.h>
#include <gui.h>

typedef enum
{
    PX_WORKER,
    PX_CLIENT,
    PX_NOWHERE
} PX_SOURCE;

typedef struct
{
    PX_SOURCE source;
    char *chars;
} Pixels;

static int _uart2_tx_server_tid = -1;
static int _gui_server_tid = -1;

int PutStr(char *str) 
{
    Pixels pixels = {.source = PX_CLIENT, .chars = str };
    Send(_gui_server_tid, (const char *) &pixels, sizeof(pixels), (char *)&pixels, sizeof(pixels));
    return 0;
}


void _init_gui_server() 
{
	RegisterAs(GUI_SERVER_NAME);
    _uart2_tx_server_tid = WhoIs(UART2_TX_SERVER_NAME);
    _gui_server_tid = MyTid();
}

void loading_task() 
{   
    int init_loading_size = 2955;
    char *str = Malloc(init_loading_size);
    str[0] = ESC;

    charstr_copy(CLEAR_SCREEN, (char *) (str + 1), 3);

    str[4] = ESC;

    charstr_copy(MOVE_CURSOR_HOME, (char *) (str + 5), 2);

    charstr_copy(SECTION_SEPARATOR, (char *) (str + 7), 101);

    for (int i = 0; i < 23; i++) {
        charstr_copy(SWITCH_ROW, (char *) (str + 108 + i * 101), 101);
	}
    charstr_copy(SECTION_SEPARATOR, (char *) (str + 2330), 101);

    str[2431] = ESC;

    charstr_copy(SCROLL_SECTION, (char *) (str + 2432), 7);

    str[2439] = ESC;

    charstr_copy("[2;90H", (char *) (str + 2440), 6);

    str[2446] = ESC;

    charstr_copy(CLEAR_TO_END, (char *) (str + 2447), 2);

    charstr_copy("switches |", (char *) (str + 2449), 10);

    for (int i = 1; i < 23; i++) {
		int row = i + 2;
		int col = SWITCH_COL;

		char *move_str = MOVE_CURSOR;
		move_str[1] = (char) row/10 + '0';
		move_str[2] = (char) row%10 + '0';
		move_str[4] = (char) col/10 + '0';
		move_str[5] = (char) col%10 + '0';

        str[2438 + 21 * i] = ESC;

        charstr_copy(move_str, (char *) (str + 2438 + 21 * i + 1), 7);

        str[2438 + 21 * i + 8] = ESC;

        charstr_copy(CLEAR_TO_END, (char *) (str + 2438 + 21 * i + 9), 2);

		int sw;
		char *sw_str = "xxx |    |";
		sw = (i <= 18) ? i : i+134;
		sw_str[0] = (char) sw/100 + '0';
		sw %= 100;
		sw_str[1] = (char) sw/10 + '0';
		sw_str[2] = (char) sw%10 + '0';

        charstr_copy(sw_str, (char *) (str + 2438 + 21 * i + 11), 10);
    }

    str[2921] = ESC;

    charstr_copy("[2;88H", (char *) (str + 2922), 6);

    for (int i=0; i<10; i++) str[2928+i] = DEL;

    charstr_copy("  sensors ", (char *) (str + 2938), 10);

    str[2948] = ESC;

    charstr_copy(CURSOR_CMD_HOME, (char *) (str + 2949), 6);

    str[2955] = '\0';

    Pixels pixels = {.source = PX_CLIENT, .chars = str};
    Send(_gui_server_tid, (const char *) &pixels, sizeof(pixels), (char *)&pixels, sizeof(pixels));

    Free(str);
}

void px_worker_task() {
    Pixels pixels = {.source = PX_WORKER};
    while(Send(_gui_server_tid, (const char *) &pixels, sizeof(pixels), (char *)&pixels, sizeof(pixels)) > -1) {
        if (pixels.source == PX_NOWHERE) {
            pixels.source = PX_WORKER;
            continue;
        }
        Putc(_uart2_tx_server_tid, COM2, pixels.chars[0]);
    }
}

void gui_server() 
{
    _init_gui_server();
    Create(PX_WORKER_TASK_PRIORITY, px_worker_task);
    // Create(LOADING_TASK_PRIORITY, loading_task);
    RingBuffer byte_buffer;
    Pixels pixels;
    int client_tid, idx;
    while (Receive(&client_tid, (char *) &(pixels), sizeof(pixels))) {
        switch (pixels.source)
        {
        case PX_WORKER:
            if (byte_buffer.start != byte_buffer.end) {
                pixels.chars = &byte_buffer.buffer[byte_buffer.start];
                byte_buffer.start++;
                byte_buffer.start &= UART_BUFFER_MASK;
            } else {
                pixels.source = PX_NOWHERE;
            }
            Reply(client_tid, (const char *) &(pixels), sizeof(pixels));
            break;
        case PX_CLIENT:
            idx = 0;
            while (pixels.chars[idx] != '\0') {
                byte_buffer.buffer[byte_buffer.end++] = pixels.chars[idx++];
                byte_buffer.end &= UART_BUFFER_MASK;
            }
            Reply(client_tid, (const char *) &(pixels), sizeof(pixels));
            break;        
        default:
            break;
        }
    }
}