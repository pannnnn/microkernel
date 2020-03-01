#include <shared.h>
#include <user.h>
#include <lib_ts7200.h>
#include <stdio.h>
#include <ds.h>
#include <gui.h>

typedef struct
{
    char *chars;
} Pixels;

static int _clock_server_tid = -1;
static int _uart2_tx_server_tid = -1;
static int _gui_server_tid = -1;

int PutStr(char *str) 
{
    Pixels pixels = {.chars = str };
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
    // dont put more than 4096 chars since buffer of com2 server is less than that
    // char str[4000];
    // for (int i = 0; i < 4000;i++) str[i] = 'a' + i % 26;
    // str[3999] = '\0';
    // Pixels pixels = {.chars = str};
    // Send(_gui_server_tid, (const char *) &pixels, sizeof(pixels), (char *)&pixels, sizeof(pixels));
}

// proprietor server
void gui_server() 
{
    _init_gui_server();
    Create(LOADING_TASK_PRIORITY, loading_task);
    Pixels pixels;
    int client_tid, idx;
    while (Receive(&client_tid, (char *) &(pixels), sizeof(pixels))) {
        Reply(client_tid, (const char *) &(pixels), sizeof(pixels));
        idx = 0;
        while (pixels.chars[idx] != '\0') {
            Putc(_uart2_tx_server_tid, COM2, pixels.chars[idx++]);
        }
    }
}