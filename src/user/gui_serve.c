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

static int _clock_server_tid = -1;
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
    _clock_server_tid = WhoIs(CLOCK_SERVER_NAME);
    _uart2_tx_server_tid = WhoIs(UART2_TX_SERVER_NAME);
    _gui_server_tid = MyTid();
}

void loading_task() 
{   
    // char str[10000];
    // for (int i = 0; i < 10000;i++) str[i] = '1';
    // str[9999] = '\0';
    // Pixels pixels = {.source = PX_CLIENT, .chars = str};
    // Send(_gui_server_tid, (const char *) &pixels, sizeof(pixels), (char *)&pixels, sizeof(pixels));
}

void px_worker_task() 
{
    Pixels pixels = {.source = PX_WORKER};
    while(Send(_gui_server_tid, (const char *) &pixels, sizeof(pixels), (char *)&pixels, sizeof(pixels)) > -1) {
        Putc(_uart2_tx_server_tid, COM2, pixels.chars[0]);
    }
}

void gui_server() 
{
    _init_gui_server();
    int px_worker_tid = Create(PX_WORKER_TASK_PRIORITY, px_worker_task);
    Create(LOADING_TASK_PRIORITY, loading_task);
    RingBuffer byte_buffer;
    Pixels pixels;
    int client_tid, idx, is_workder_idled = 1;
    while (Receive(&client_tid, (char *) &(pixels), sizeof(pixels))) {
        switch (pixels.source)
        {
        case PX_WORKER:
            if (byte_buffer.start != byte_buffer.end) {
                pixels.chars = &byte_buffer.buffer[byte_buffer.start];
                byte_buffer.start++;
                byte_buffer.start &= UART_BUFFER_MASK;
                is_workder_idled = 0;
                Reply(client_tid, (const char *) &(pixels), sizeof(pixels));
            } else {
                is_workder_idled = 1;
            }
            break;
        case PX_CLIENT:
            idx = 0;
            while (pixels.chars[idx] != '\0') {
                byte_buffer.buffer[byte_buffer.end++] = pixels.chars[idx++];
                byte_buffer.end &= UART_BUFFER_MASK;
            }
            Reply(client_tid, (const char *) &(pixels), sizeof(pixels));

            if (is_workder_idled) {
                pixels.chars = &byte_buffer.buffer[byte_buffer.start];
                byte_buffer.start++;
                byte_buffer.start &= UART_BUFFER_MASK;
                is_workder_idled = 0;
                Reply(px_worker_tid, (const char *) &(pixels), sizeof(pixels));
            }
            break;        
        default:
            break;
        }
    }
}