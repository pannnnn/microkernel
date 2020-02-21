#include <shared.h>
#include <user.h>
#include <ds.h>
#include <stdio.h>

static int _uart1_rx_server_tid = -1;
static int _uart1_tx_server_tid = -1;
static int _uart2_rx_server_tid = -1;
static int _uart2_tx_server_tid = -1;
static volatile int *uart1_data = (int *)( UART1_BASE + UART_DATA_OFFSET );
static volatile int *uart1_flags = (int *) ( UART1_BASE + UART_FLAG_OFFSET );
static volatile int *uart2_data = (int *)( UART2_BASE + UART_DATA_OFFSET );
static volatile int *uart2_flags = (int *) ( UART2_BASE + UART_FLAG_OFFSET );

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
    if (channel != COM1 && channel != COM2) return -1;
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
    while ((uart1_rx_message.data = AwaitEvent(UART1_RX_EVENT)) > -1) {
        debug("uart1 rx notifier");
        uart1_rx_message.source = FROM_DEVICE;
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
                debug("get byte from uart1 rx notifier and send to waiting clients");
                uart1_rx_message.data = byte_buffer.buffer[byte_buffer.start++];
                byte_buffer.start &= UART_BUFFER_MASK;
                while((replied_tid = deque(&client_queue)) != -1) {
                    Reply(replied_tid, (const char *) &uart1_rx_message, sizeof(uart1_rx_message));
                }
            } else {
                debug("get byte from uart1 rx notifier and add to buffer");
            }
            uart1_rx_message.source = FROM_SERVER;
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
    while (AwaitEvent(UART1_TX_EVENT) > -1) {
        debug("uart1 notifier: await cts ast");
        // To speed up
        if ( !(*uart1_flags & CTS_MASK) ) {
            AwaitEvent(CTS_AST);
        } else {
            highlight("uart1 notifier: cts already asserted");
        }
        debug("uart1 notifier: cts asserted");
        uart1_tx_message.source = FROM_DEVICE;
        int result = Send(_uart1_tx_server_tid, (const char *) &uart1_tx_message, sizeof(uart1_tx_message), (char *)&uart1_tx_message, sizeof(uart1_tx_message));
        if (result < 0) {
            error("something went wrong here");
        } else {
            debug("uart1 notifier: tries to put char %c", uart1_tx_message.data);
            *uart1_data = uart1_tx_message.data;
            debug("uart1 notifier: await cts neg");
            // To speed up
            if ( *uart1_flags & CTS_MASK ) {
                AwaitEvent(CTS_NEG);
            } else {
                highlight("uart1 notifier: cts already negated");
            }
            debug("uart1 notifier: cts negated");
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
    int client_tid, put_ready = 0;
    while (Receive(&client_tid, (char *) &uart1_tx_message, sizeof(uart1_tx_message))) {
        switch ( uart1_tx_message.source )
        {
        case FROM_DEVICE:
            if (byte_buffer.start == byte_buffer.end) {
                debug("uart1 server: register notifier %d", client_tid);
                put_ready = 1;
            } else {
                uart1_tx_message.data = byte_buffer.buffer[byte_buffer.start++];
                byte_buffer.start &= UART_BUFFER_MASK;
                // to differentiate if the notifier has been waiting or not
                uart1_tx_message.source = FROM_SERVER;
                debug("uart1 server: put char %c to notifier from buffer %d", uart1_tx_message.data, uart1_tx_notifier_tid);
                Reply(uart1_tx_notifier_tid, (const char *) &uart1_tx_message, sizeof(uart1_tx_message));
            }
            break;
        case FROM_USER:
            byte_buffer.buffer[byte_buffer.end++] = uart1_tx_message.data;
            byte_buffer.end &= UART_BUFFER_MASK;
            if (put_ready) {
                uart1_tx_message.data = byte_buffer.buffer[byte_buffer.start++];
                byte_buffer.start &= UART_BUFFER_MASK;
                debug("uart1 server: put char %c to notifier from user", uart1_tx_message.data);
                Reply(uart1_tx_notifier_tid, (const char *) &uart1_tx_message, sizeof(uart1_tx_message));
                put_ready = 0;
                uart1_tx_message.source = FROM_DEVICE;
            } else {
                // to differentiate if the client has been waiting or not
                debug("uart1 server: tx not ready reply back");
                uart1_tx_message.source = FROM_SERVER;
            }
            Reply(client_tid, (const char *) &uart1_tx_message, sizeof(uart1_tx_message));
            break;
        default:
            break;
        }
    }
}


/*
 * UART2 RX
 */
void _init_uart2_rx_server() 
{
	RegisterAs(UART2_RX_SERVER_NAME);
    _uart2_rx_server_tid = WhoIs(UART2_RX_SERVER_NAME);
}

void uart2_rx_notifier() 
{
    event_notifier_registrar[UART2_RX_EVENT] = MyTid();
    UartMessage uart2_rx_message;
    while (AwaitEvent(UART2_RX_EVENT) > -1) {
        debug("uart2 rx notifier");
        uart2_rx_message.source = FROM_DEVICE;
        int result = Send(_uart2_rx_server_tid, (const char *) &uart2_rx_message, sizeof(uart2_rx_message), (char *)&uart2_rx_message, sizeof(uart2_rx_message));
        if (result < 0) {
            error("something went wrong here");
        }
    }
}

void uart2_rx_server() 
{
    _init_uart2_rx_server();
    RingBuffer byte_buffer ={.start = 0, .end = 0};
    Queue client_queue = {.size = 0, .index = 0};
    UartMessage uart2_rx_message;
    int uart2_rx_notifier_tid = Create(UART2_RX_NOTIFIER_PRIORITY, uart2_rx_notifier);
    if (uart2_rx_notifier_tid < 0) {
        error("failed to create uart2 rx notifier");
    }
    int client_tid, replied_tid;
    while (Receive(&client_tid, (char *) &uart2_rx_message, sizeof(uart2_rx_message))) {
        switch ( uart2_rx_message.source )
        {
        case FROM_DEVICE:
            while ( !(*uart2_flags & RXFE_MASK) ) {
                byte_buffer.buffer[byte_buffer.end++] = *uart2_data & DATA_MASK;
                byte_buffer.end &= UART_BUFFER_MASK;
            }
            if (  client_queue.index != client_queue.size ) {
                debug("get byte from uart2 rx notifier and send to waiting clients");
                uart2_rx_message.data = byte_buffer.buffer[byte_buffer.start++];
                byte_buffer.start &= UART_BUFFER_MASK;
                while((replied_tid = deque(&client_queue)) != -1) {
                    Reply(replied_tid, (const char *) &uart2_rx_message, sizeof(uart2_rx_message));
                }
            } else {
                debug("get byte from uart2 rx notifier and add to buffer");
            }
            uart2_rx_message.source = FROM_SERVER;
            Reply(uart2_rx_notifier_tid, (const char *) &uart2_rx_message, sizeof(uart2_rx_message));
            break;
        case FROM_USER:
            debug("get request from client for uart2 rx server");
            if ( byte_buffer.start != byte_buffer.end ) {
                uart2_rx_message.data = byte_buffer.buffer[byte_buffer.start++];
                byte_buffer.start &= UART_BUFFER_MASK;
                // to differentiate if the client has been waiting or not
                uart2_rx_message.source = FROM_SERVER;
                Reply(client_tid, (const char *) &uart2_rx_message, sizeof(uart2_rx_message));
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
 * UART2 TX
 */
void _init_uart2_tx_server() 
{
	RegisterAs(UART2_TX_SERVER_NAME);
    _uart2_tx_server_tid = WhoIs(UART2_TX_SERVER_NAME);
}

void uart2_tx_notifier() 
{
    event_notifier_registrar[UART2_TX_EVENT] = MyTid();
    UartMessage uart2_tx_message;

    while (AwaitEvent(UART2_TX_EVENT) > -1) {
        debug("uart2 tx notifier");
        uart2_tx_message.source = FROM_DEVICE;
        int result = Send(_uart2_tx_server_tid, (const char *) &uart2_tx_message, sizeof(uart2_tx_message), (char *)&uart2_tx_message, sizeof(uart2_tx_message));
        if (result < 0) {
            error("something went wrong here");
        }
    }
}

void uart2_tx_server() 
{
    _init_uart2_tx_server();
    RingBuffer byte_buffer ={.start = 0, .end = 0};
    UartMessage uart2_tx_message;
    int uart2_tx_notifier_tid = Create(UART2_TX_NOTIFIER_PRIORITY, uart2_tx_notifier);
    if (uart2_tx_notifier_tid < 0) {
        error("failed to create uart2 tx notifier");
    }
    int client_tid, put_ready = 0;
    while (Receive(&client_tid, (char *) &uart2_tx_message, sizeof(uart2_tx_message))) {
        switch ( uart2_tx_message.source )
        {
        case FROM_DEVICE:
            if (byte_buffer.start == byte_buffer.end) {
                debug("uart2 server: register notifier %d", client_tid);
                put_ready = 1;
            } else {
                for (int i = byte_buffer.start, count = 0; 
                    byte_buffer.start != byte_buffer.end && !(*uart2_flags & TXFF_MASK); 
                    i++, count++) {
                    // highlight("buffer %c", byte_buffer.buffer[byte_buffer.start]);
                    *uart2_data = byte_buffer.buffer[byte_buffer.start++];
                    byte_buffer.start &= UART_BUFFER_MASK;
                }
                // to differentiate if the notifier has been waiting or not
                uart2_tx_message.source = FROM_SERVER;
                debug("uart2 server: put %d chars to notifier from buffer %d", count, uart2_tx_notifier_tid);
                Reply(uart2_tx_notifier_tid, (const char *) &uart2_tx_message, sizeof(uart2_tx_message));
            }
            break;
        case FROM_USER:
            byte_buffer.buffer[byte_buffer.end++] = uart2_tx_message.data;
            byte_buffer.end &= UART_BUFFER_MASK;
            if (put_ready) {
                *uart2_data = byte_buffer.buffer[byte_buffer.start++];
                byte_buffer.start &= UART_BUFFER_MASK;
                debug("uart2 server: put char %c to notifier from user", uart2_tx_message.data);
                Reply(uart2_tx_notifier_tid, (const char *) &uart2_tx_message, sizeof(uart2_tx_message));
                put_ready = 0;
                uart2_tx_message.source = FROM_DEVICE;
            } else {
                // to differentiate if the client has been waiting or not
                debug("uart2 server: tx not ready reply back");
                uart2_tx_message.source = FROM_SERVER;
            }
            Reply(client_tid, (const char *) &uart2_tx_message, sizeof(uart2_tx_message));
            break;
        default:
            break;
        }
    }
}