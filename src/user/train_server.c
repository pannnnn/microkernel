#include <shared.h>
#include <user.h>
#include <lib_ts7200.h>
#include <stdio.h>

static int _clock_server_tid = -1;
static int _uart1_rx_server_tid = -1;
static int _uart1_tx_server_tid = -1;
static int _uart2_rx_server_tid = -1;
static int _uart2_tx_server_tid = -1;
static int _acknowledged = 0;

typedef struct
{
    char content[SENSOR_DATA_BYTES];
} SensorData;

void _init_train_server() 
{
    _clock_server_tid = WhoIs(CLOCK_SERVER_NAME);
    _uart1_rx_server_tid = WhoIs(UART1_RX_SERVER_NAME);
    _uart1_tx_server_tid = WhoIs(UART1_TX_SERVER_NAME);
    _uart2_rx_server_tid = WhoIs(UART2_RX_SERVER_NAME);
    _uart2_tx_server_tid = WhoIs(UART2_TX_SERVER_NAME);
    _acknowledged = 1;
}

void _init_rails() {
    Putc(_uart1_tx_server_tid, COM1, TRAIN_START);
    Delay(_clock_server_tid, INTER_COMMANDS_DELAY_TICKS);

    for (int sw = 1; sw <= SWITCH_ONE_WAY_COUNT; sw++) {
        Putc(_uart1_tx_server_tid, COM1, SWITCH_STRAIGHT);
        Putc(_uart1_tx_server_tid, COM1, (char) sw);
        Delay(_clock_server_tid, SWITCH_END_DELAY_TICKS);
	}

    int data[4][2] = {
        {SWITCH_BRANCH, SWITCH_TWO_WAY_1a}, 
        {SWITCH_STRAIGHT, SWITCH_TWO_WAY_1b}, 
        {SWITCH_BRANCH, SWITCH_TWO_WAY_2a}, 
        {SWITCH_STRAIGHT,SWITCH_TWO_WAY_2b}
        };

    for (int i = 0; i <4; i++) {
        Putc(_uart1_tx_server_tid, COM1, data[i][0]);
        Putc(_uart1_tx_server_tid, COM1, (char) data[i][1]);
        Delay(_clock_server_tid, SWITCH_END_DELAY_TICKS);
    }
    Putc(_uart1_tx_server_tid, COM1, SWITCH_END);
    Delay(_clock_server_tid, INTER_COMMANDS_DELAY_TICKS);
}

void _process_sensor_data(char label, int data) {
    for (int i = 0; i < SENSOR_BITS_PER_MODULE; i++) {
        if (data & (1 << i)) {
            char str[3] = {label, '0' + (SENSOR_BITS_PER_MODULE - i) / 10, '0' + (SENSOR_BITS_PER_MODULE - i) % 10};
            for (int i = 0; i < 3; i++) {
                Putc(_uart2_tx_server_tid, COM2, str[i]);
			}
        }
    }
}

void train_data_processor() 
{
    char c;
    int count = 0;
    char buffer[2];
    while ( (c = Getc(_uart1_rx_server_tid, COM1)) > -1) {
        buffer[count % 2] = c;
        if (count % 2) {
            _process_sensor_data('A' + count / 2, buffer[0] << 8 | buffer[1]);
        }
        count++;
        _acknowledged = count / SENSOR_MODULE_BYTES_COUNT;
        count %= SENSOR_MODULE_BYTES_COUNT;
    }
}

void train_server() 
{
    _init_train_server();
    Create(TRAIN_DATA_PROCESSOR_PRIORITY, train_data_processor);
    _init_rails();
    for (;;) {
        if (_acknowledged) {
            // log("ticks: %d", Time(_clock_server_tid));
            Putc(_uart1_tx_server_tid, COM1, (char) SENSOR_DATA_QUERY_BYTE);
            _acknowledged = 0;
        }
        Delay(_clock_server_tid, 1);
    }
}
