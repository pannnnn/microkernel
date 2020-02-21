#include <shared.h>
#include <user.h>
#include <lib_ts7200.h>
#include <stdio.h>

static int _command_server_tid = -1;

typedef struct
{
    char content[COMMAND_MAX_LEN];
    int len;
} Command;

static int _uart1_rx_server_tid = -1;
static int _uart1_tx_server_tid = -1;
static int _uart2_rx_server_tid = -1;
static int _uart2_tx_server_tid = -1;
static int _clock_server_tid = -1;
static int _train_speed = 0;

void _process_command(Command *cmd) {
    if (cmd->content[0] == 'q' && cmd->len == 1) {
        Exit();
    }
    if (cmd->content[2] != ' ') return;
    if (cmd->content[3] < '0' && cmd->content[3] > '9') return;
    // TODO: should we check valid range for train_number and switch_number
    if (cmd->content[0] == 't' && cmd->content[1] == 'r') {
        int train_number = -1, train_speed = -1;
        if (cmd->content[4] == ' ') {
            train_number = cmd->content[3] - '0';
            if (cmd->content[5] < '0' && cmd->content[5] > '9') return;
            if (cmd->len == 6) {
                train_speed = cmd->content[5] - '0';
            } else if (cmd->len == 7) {
                if (cmd->content[6] < '0' && cmd->content[6] > '9') return;
                train_speed = (cmd->content[5] - '0') * 10 + (cmd->content[6] - '0');
            }
        } else if (cmd->content[5] == ' ') {
            if (cmd->content[4] < '0' && cmd->content[4] > '9') return;
            train_number = (cmd->content[3] - '0') * 10 + (cmd->content[4] - '0');
            if (cmd->content[6] < '0' && cmd->content[6] > '9') return;
            if (cmd->len == 7) {
                train_speed = cmd->content[6] - '0';
            } else if (cmd->len == 8) {
                if (cmd->content[7] < '0' && cmd->content[7] > '9') return;
                train_speed = (cmd->content[6] - '0') * 10 + (cmd->content[7] - '0');
            }
        }

        if (train_number != -1 && train_speed != -1) {
            Putc(_uart1_tx_server_tid, COM1, (char) train_speed);
            Putc(_uart1_tx_server_tid, COM1, (char) train_number);
            if (train_speed >= TRAIN_MIN_SPEED && train_speed <= TRAIN_MAX_SPEED) {
                _train_speed = train_speed;
            } else if (train_speed >= TRAIN_LIGHTS_ON + TRAIN_MIN_SPEED && train_speed <= TRAIN_LIGHTS_ON + TRAIN_MAX_SPEED) {
                _train_speed = train_speed - TRAIN_LIGHTS_ON;
            }
        }
    } else if (cmd->content[0] == 'r' && cmd->content[1] == 'v') {
        int train_number = -1;
        if (cmd->len == 4) {
            train_number = cmd->content[3] - '0';
        } else if (cmd->len == 5) {
            if (cmd->content[4] < '0' && cmd->content[4] > '9') return;
            train_number = (cmd->content[3] - '0') * 10 + (cmd->content[4] - '0');
        }
        if (train_number != -1) {
            Putc(_uart1_tx_server_tid, COM1, (char) 0);
            Putc(_uart1_tx_server_tid, COM1, (char) train_number);
            Putc(_uart1_tx_server_tid, COM1, (char) TRAIN_REVERSE_DIRECTION);
            Putc(_uart1_tx_server_tid, COM1, (char) train_number);
            Putc(_uart1_tx_server_tid, COM1, (char) _train_speed);
            Putc(_uart1_tx_server_tid, COM1, (char) train_number);
        }
    } else if (cmd->content[0] == 's' && cmd->content[1] == 'w') {
        int switch_number = -1;
        char switch_direction = '0';
        if (cmd->content[4] == ' ') {
            switch_number = cmd->content[3] - '0';
            if ((cmd->content[5] != 'S' && cmd->content[5] != 'C') || cmd->len != 6) return;
            switch_direction = cmd->content[5];
        } else if (cmd->content[5] == ' ') {
            if (cmd->content[4] < '0' && cmd->content[4] > '9') return;
            switch_number = (cmd->content[3] - '0') * 10 + (cmd->content[4] - '0');
            if ((cmd->content[6] != 'S' && cmd->content[6] != 'C') || cmd->len != 7) return;
            switch_direction = cmd->content[6];
        } else if (cmd->content[6] == ' ') {
            if (cmd->content[4] < '0' && cmd->content[4] > '9') return;
            if (cmd->content[5] < '0' && cmd->content[5] > '9') return;
            switch_number = (cmd->content[3] - '0') * 100 + (cmd->content[4] - '0') * 10 + (cmd->content[5] - '0');
            if ((cmd->content[7] != 'S' && cmd->content[7] != 'C') || cmd->len != 8) return;
            switch_direction = cmd->content[7];
        }

        if (switch_number != -1 && switch_direction != '0') {
            switch_direction = switch_direction == 'S' ? SWITCH_STRAIGHT : SWITCH_BRANCH;
            Putc(_uart1_tx_server_tid, COM1, switch_direction);
            Putc(_uart1_tx_server_tid, COM1, (char) switch_number);
            Delay(_clock_server_tid, SWITCH_END_DELAY_IN_TEN_MILLSEC);
            Putc(_uart1_tx_server_tid, COM1, SWITCH_END);
        }
    }

}

void _init_command_server() 
{
	RegisterAs(COMMAND_SERVER_NAME);
    _command_server_tid = WhoIs(COMMAND_SERVER_NAME);
}

void train_server() 
{
    _uart1_rx_server_tid = WhoIs(UART2_RX_SERVER_NAME);
    _uart1_tx_server_tid = WhoIs(UART1_TX_SERVER_NAME);
    _uart2_tx_server_tid = WhoIs(UART1_TX_SERVER_NAME);
    Putc(_uart1_tx_server_tid, COM1, TRAIN_START);
}

void command_server() {
    _clock_server_tid = WhoIs(CLOCK_SERVER_NAME);
    _uart2_rx_server_tid = WhoIs(UART2_RX_SERVER_NAME);
    _uart1_tx_server_tid = WhoIs(UART1_TX_SERVER_NAME);
    unsigned char c;
    Command cmd = {.content = {0}, .len = 0};
    while ( (c = Getc(_uart2_rx_server_tid, COM2)) > -1) {
        if (c != TERMINAL_ENTER_KEY_CODE && cmd.len < COMMAND_MAX_LEN) {
            cmd.content[cmd.len++] = c;
        } else if (c == TERMINAL_ENTER_KEY_CODE && cmd.len < COMMAND_MAX_LEN) {
            process_command(&cmd);
            for (int i = 0; i < COMMAND_MAX_LEN; i++) cmd.content[i]= 0;
            cmd.len = 0;
        }
    }
}