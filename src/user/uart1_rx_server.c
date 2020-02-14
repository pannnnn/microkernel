#include <shared.h>
#include <user.h>
#include <ds.h>
#include <stdio.h>

static int _uart1_rx_server_tid = -1;

void _init_uart1_rx_server() 
{
	RegisterAs(UART1_RX_SERVER_NAME);
    _uart1_rx_server_tid = WhoIs(UART1_RX_SERVER_NAME);
}

void uart1_rx_notifier() 
{
    UartMessage uart_message;
    uart_message.type = UART1_RECEIVE_BYTE;
    while ((uart_message.data = AwaitEvent(UART1_RX_EVENT)) > -1) {
        debug("awake every one tick (one sec)");
        int result = Send(_uart1_rx_server_tid, (const char *) &uart_message, sizeof(uart_message), (char *)&uart_message, sizeof(uart_message));
        if (result < 0) {
            debug("something went wrong here");
        }
    }
}

void uart1_rx_server() 
{
    _init_clock_server();
    Queue blocked_tids = {.size = 0, .index = 0};
    UartMessage uart_message;
    int clock_notifier_tid = Create(2, uart1_rx_notifier);
    if (clock_notifier_tid < 0) {
        debug("failed to create uart1 rx notifier");
    }

    // TODO: get message from notifier or user
}
