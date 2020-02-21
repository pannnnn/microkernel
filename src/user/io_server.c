#include <shared.h>
#include <user.h>
#include <stdio.h>

static int _command_server_tid = -1;

void _init_command_server() 
{
	RegisterAs(COMMAND_SERVER_NAME);
    _command_server_tid = WhoIs(COMMAND_SERVER_NAME);
}

void train_server() 
{

}

void command_server() {
    int uart2_rx_server_tid = WhoIs(UART2_TX_SERVER_NAME);
    unsigned char c;
    while ( (c = Getc(uart2_rx_server_tid, COM2)) > -1) {
        if (c != TERMINAL_ENTER_KEY_CODE) {

        }
    }
}
