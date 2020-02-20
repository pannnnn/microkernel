#include <shared.h>
#include <user.h>
#include <ds.h>
#include <stdio.h>

static int _uart1_rx_server_tid = -1;
static int _uart1_tx_server_tid = -1;
static int _uart2_rx_server_tid = -1;
static int _uart2_tx_server_tid = -1;
static volatile int *uart1_data = (int *)( UART1_BASE + UART_DATA_OFFSET );

int Getc(int tid, int channel) 
{
    if (channel != COM1 && channel != COM2) return -1;    
    if (channel == COM1 && tid != _uart1_rx_server_tid) return -1;
    if (channel == COM2 && tid != _uart2_rx_server_tid) return -1;
    int uart_server_tid = channel == COM1 ? _uart1_rx_server_tid : _uart2_rx_server_tid;
    UartMessage uart_rx_message;
    uart_rx_message.source = FROM_USER;
    Send(uart_server_tid, (const char *) &uart_rx_message, sizeof(uart_rx_message), (char *)&uart_rx_message, sizeof(uart_rx_message));
    return (int) uart_rx_message.data;
}

int Putc(int tid, int channel, char ch) 
{
    if (channel != COM1 || channel != COM2) return -1;
    if (channel == COM1 && tid != _uart1_tx_server_tid) return -1;
    if (channel == COM2 && tid != _uart2_tx_server_tid) return -1;
    int uart_server_tid = channel == COM1 ? _uart1_tx_server_tid : _uart2_tx_server_tid;
    UartMessage uart_tx_message;
    uart_tx_message.source = FROM_USER;
    uart_tx_message.data = ch;
    Send(uart_server_tid, (const char *) &uart_tx_message, sizeof(uart_tx_message), (char *)&uart_tx_message, sizeof(uart_tx_message));
    return 0;
}

/*
 * UART1 RX
 */
void _init_uart1_rx_server() 
{
	RegisterAs(UART1_RX_SERVER_NAME);
    _uart1_rx_server_tid = WhoIs(UART1_RX_SERVER_NAME);
}

void uart1_rx_notifier() 
{
    event_notifier_registrar[UART1_RX_EVENT] = MyTid();
    UartMessage uart1_rx_message;
    uart1_rx_message.source = FROM_DEVICE;
    while ((uart1_rx_message.data = AwaitEvent(UART1_RX_EVENT)) > -1) {
        debug("uart1 rx notifier");
        int result = Send(_uart1_rx_server_tid, (const char *) &uart1_rx_message, sizeof(uart1_rx_message), (char *)&uart1_rx_message, sizeof(uart1_rx_message));
        if (result < 0) {
            error("something went wrong here");
        }
    }
}

void uart1_rx_server() 
{
    _init_uart1_rx_server();
    RingBuffer byte_buffer ={.start = 0, .end = 0};
    Queue client_queue = {.size = 0, .index = 0};
    UartMessage uart1_rx_message;
    int uart1_rx_notifier_tid = Create(UART1_RX_NOTIFIER_PRIORITY, uart1_rx_notifier);
    if (uart1_rx_notifier_tid < 0) {
        error("failed to create uart1 rx notifier");
    }
    int client_tid, replied_tid;
    while (Receive(&client_tid, (char *) &uart1_rx_message, sizeof(uart1_rx_message))) {
        switch ( uart1_rx_message.source )
        {
        case FROM_DEVICE:
            byte_buffer.buffer[byte_buffer.end++] = uart1_rx_message.data;
            byte_buffer.end &= UART_BUFFER_MASK;
            if (  client_queue.index != client_queue.size ) {
                debug("get byte from uart1 rx server and send to waiting clients");
                uart1_rx_message.data = byte_buffer.buffer[byte_buffer.start++];
                byte_buffer.start &= UART_BUFFER_MASK;
                while((replied_tid = deque(&client_queue)) != -1) {
                    Reply(replied_tid, (const char *) &uart1_rx_message, sizeof(uart1_rx_message));
                }
            } else {
                debug("get byte from uart1 rx server and add to buffer");
            }
            Reply(uart1_rx_notifier_tid, (const char *) &uart1_rx_message, sizeof(uart1_rx_message));
            break;
        case FROM_USER:
            debug("get request from client for uart1 rx server");
            if (byte_buffer.start != byte_buffer.end) {
                uart1_rx_message.data = byte_buffer.buffer[byte_buffer.start++];
                byte_buffer.start &= UART_BUFFER_MASK;
                // to differentiate if the client has been waiting or not
                uart1_rx_message.source = FROM_SERVER;
                Reply(client_tid, (const char *) &uart1_rx_message, sizeof(uart1_rx_message));
            } else {
                enqueue(&client_queue, client_tid);
            }
            break;
        default:
            break;
        }
    }
}


/*
 * UART1 TX
 */
void _init_uart1_tx_server() 
{
	RegisterAs(UART1_TX_SERVER_NAME);
    _uart1_tx_server_tid = WhoIs(UART1_TX_SERVER_NAME);
}

void uart1_tx_notifier() 
{
    event_notifier_registrar[UART1_TX_EVENT] = MyTid();
    UartMessage uart1_tx_message;
    uart1_tx_message.source = FROM_DEVICE;
    while ((uart1_tx_message.data = AwaitEvent(UART1_TX_EVENT)) > -1) {
        debug("uart1 tx notifier");
        AwaitEvent(CTS_AST);
        int result = Send(_uart1_tx_server_tid, (const char *) &uart1_tx_message, sizeof(uart1_tx_message), (char *)&uart1_tx_message, sizeof(uart1_tx_message));
        if (result < 0) {
            error("something went wrong here");
        } else {
            *uart1_data = uart1_tx_message.data;
            AwaitEvent(CTS_NEG);
        }
    }
}

void uart1_tx_server() 
{
    _init_uart1_tx_server();
    RingBuffer byte_buffer ={.start = 0, .end = 0};
    UartMessage uart1_tx_message;
    int uart1_tx_notifier_tid = Create(UART1_TX_NOTIFIER_PRIORITY, uart1_tx_notifier);
    if (uart1_tx_notifier_tid < 0) {
        error("failed to create uart1 tx notifier");
    }
    int client_tid, notifier_tid = -1;
    while (Receive(&client_tid, (char *) &uart1_tx_message, sizeof(uart1_tx_message))) {
        switch ( uart1_tx_message.source )
        {
        case FROM_DEVICE:
            if (byte_buffer.start == byte_buffer.end) {
                notifier_tid = client_tid;
            } else {
                uart1_tx_message.data = byte_buffer.buffer[byte_buffer.start++];
                byte_buffer.start &= UART_BUFFER_MASK;
                // to differentiate if the notifier has been waiting or not
                uart1_tx_message.source = FROM_SERVER;
                Reply(notifier_tid, (const char *) &uart1_tx_message, sizeof(uart1_tx_message));
            }
            break;
        case FROM_USER:
            byte_buffer.buffer[byte_buffer.end++] = uart1_tx_message.data;
            byte_buffer.end &= UART_BUFFER_MASK;
            if (notifier_tid != -1) {
                uart1_tx_message.data = byte_buffer.buffer[byte_buffer.start++];
                byte_buffer.start &= UART_BUFFER_MASK;
                Reply(notifier_tid, (const char *) &uart1_tx_message, sizeof(uart1_tx_message));
                notifier_tid = -1;
                uart1_tx_message.source = FROM_DEVICE;
            } else {
                // to differentiate if the client has been waiting or not
                uart1_tx_message.source = FROM_SERVER;
            }
            Reply(client_tid, (const char *) &uart1_tx_message, sizeof(uart1_tx_message));
            break;
        default:
            break;
        }
    }
}
