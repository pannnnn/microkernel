#include <shared.h>
#include <user.h>
#include <lib_ts7200.h>
#include <stdio.h>
#include <ds.h>

typedef struct
{
    char content[COMMAND_MAX_LEN];
    int len;
} CommandBuffer;

typedef enum
{
    CT_FETCH_COMMAND,
    CT_TRAIN_NORMAL,
    CT_TRAIN_REVERSE,
    CT_SWITCH_NORMAL,
    CT_SWITCH_END,
    CT_SENSOR_FETCH,
} COMMAND_TYPE;

typedef struct
{
    COMMAND_TYPE type;
    int id;
    char content[2];
    int len;
    int ticks;
} Command;

static int _clock_server_tid = -1;
static int _uart1_rx_server_tid = -1;
static int _uart1_tx_server_tid = -1;
static int _uart2_rx_server_tid = -1;
static int _uart2_tx_server_tid = -1;
static int _command_server_tid = -1;
static int _train_speed = 0;

static Command cmd_buffer[QUEUE_SIZE];

int _command_comparator1(int cmd_id) {
    return cmd_buffer[cmd_id].ticks;
}

int _command_comparator2(int dummy_value) {
    return 0;
}

void _init_command_server() 
{
	RegisterAs(COMMAND_SERVER_NAME);
    _clock_server_tid = WhoIs(CLOCK_SERVER_NAME);
    _uart1_rx_server_tid = WhoIs(UART1_RX_SERVER_NAME);
    _uart1_tx_server_tid = WhoIs(UART1_TX_SERVER_NAME);
    _uart2_rx_server_tid = WhoIs(UART2_RX_SERVER_NAME);
    _uart2_tx_server_tid = WhoIs(UART2_TX_SERVER_NAME);
    _command_server_tid = MyTid();
    _train_speed = 0;
}

void _process_sensor_data(char label, int data) 
{
    for (int i = 0; i < SENSOR_BITS_PER_MODULE; i++) {
        if (data & (1 << i)) {
            char str[4] = {label, '0' + (SENSOR_BITS_PER_MODULE - i) / 10, '0' + (SENSOR_BITS_PER_MODULE - i) % 10, '\0'};
            PutStr(str);
        }
    }
}

void _process_command(CommandBuffer *cmdBuf) 
{
    if (cmdBuf->content[0] == 'q' && cmdBuf->len == 1) {
        Exit();
    }
    if (cmdBuf->content[2] != ' ') return;
    if (cmdBuf->content[3] < '0' || cmdBuf->content[3] > '9') return;
    // TODO: should we check valid range for train_number and switch_number
    if (cmdBuf->content[0] == 't' && cmdBuf->content[1] == 'r') {
        int train_number = -1, train_speed = -1;
        if (cmdBuf->content[4] == ' ') {
            train_number = cmdBuf->content[3] - '0';
            if (cmdBuf->content[5] < '0' || cmdBuf->content[5] > '9') return;
            if (cmdBuf->len == 6) {
                train_speed = cmdBuf->content[5] - '0';
            } else if (cmdBuf->len == 7) {
                if (cmdBuf->content[6] < '0' || cmdBuf->content[6] > '9') return;
                train_speed = (cmdBuf->content[5] - '0') * 10 + (cmdBuf->content[6] - '0');
            }
        } else if (cmdBuf->content[5] == ' ') {
            if (cmdBuf->content[4] < '0' || cmdBuf->content[4] > '9') return;
            train_number = (cmdBuf->content[3] - '0') * 10 + (cmdBuf->content[4] - '0');
            if (cmdBuf->content[6] < '0' || cmdBuf->content[6] > '9') return;
            if (cmdBuf->len == 7) {
                train_speed = cmdBuf->content[6] - '0';
            } else if (cmdBuf->len == 8) {
                if (cmdBuf->content[7] < '0' || cmdBuf->content[7] > '9') return;
                train_speed = (cmdBuf->content[6] - '0') * 10 + (cmdBuf->content[7] - '0');
            }
        }

        if (train_number != -1 && train_speed != -1) {
            if (train_speed >= TRAIN_MIN_SPEED && train_speed <= TRAIN_MAX_SPEED) {
                _train_speed = train_speed;
            } else if (train_speed >= TRAIN_LIGHTS_ON + TRAIN_MIN_SPEED && train_speed <= TRAIN_LIGHTS_ON + TRAIN_MAX_SPEED) {
                _train_speed = train_speed - TRAIN_LIGHTS_ON;
            }
            Command cmd = {.type = CT_TRAIN_NORMAL, .content = {_train_speed, train_number}, .len = 2};
            int result = Send(_command_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
            if (result < 0) error("something went wrong here");
        }
    } else if (cmdBuf->content[0] == 'r' && cmdBuf->content[1] == 'v') {
        int train_number = -1;
        if (cmdBuf->len == 4) {
            train_number = cmdBuf->content[3] - '0';
        } else if (cmdBuf->len == 5) {
            if (cmdBuf->content[4] < '0' || cmdBuf->content[4] > '9') return;
            train_number = (cmdBuf->content[3] - '0') * 10 + (cmdBuf->content[4] - '0');
        }
        if (train_number != -1) {
            Command cmd = {.type = CT_TRAIN_REVERSE, .content = {0, train_number}, .len = 2};
            int result = Send(_command_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
            if (result < 0) error("something went wrong here");
            cmd.type = CT_TRAIN_NORMAL;
            cmd.content[0] = TRAIN_REVERSE_DIRECTION;
            result = Send(_command_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
            if (result < 0) error("something went wrong here");
            cmd.content[0] = _train_speed;
            result = Send(_command_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
            if (result < 0) error("something went wrong here");
        }
    } else if (cmdBuf->content[0] == 's' && cmdBuf->content[1] == 'w') {
        int switch_number = -1;
        char switch_direction = '0';
        if (cmdBuf->content[4] == ' ') {
            switch_number = cmdBuf->content[3] - '0';
            if ((cmdBuf->content[5] != 'S' && cmdBuf->content[5] != 'C') || cmdBuf->len != 6) return;
            switch_direction = cmdBuf->content[5];
        } else if (cmdBuf->content[5] == ' ') {
            if (cmdBuf->content[4] < '0' || cmdBuf->content[4] > '9') return;
            switch_number = (cmdBuf->content[3] - '0') * 10 + (cmdBuf->content[4] - '0');
            if ((cmdBuf->content[6] != 'S' && cmdBuf->content[6] != 'C') || cmdBuf->len != 7) return;
            switch_direction = cmdBuf->content[6];
        } else if (cmdBuf->content[6] == ' ') {
            if (cmdBuf->content[4] < '0' || cmdBuf->content[4] > '9') return;
            if (cmdBuf->content[5] < '0' || cmdBuf->content[5] > '9') return;
            switch_number = (cmdBuf->content[3] - '0') * 100 + (cmdBuf->content[4] - '0') * 10 + (cmdBuf->content[5] - '0');
            if ((cmdBuf->content[7] != 'S' && cmdBuf->content[7] != 'C') || cmdBuf->len != 8) return;
            switch_direction = cmdBuf->content[7];
        }

        if (switch_number != -1 && switch_direction != '0') {
            switch_direction = switch_direction == 'S' ? SWITCH_STRAIGHT : SWITCH_BRANCH;

            Command cmd = {.type = CT_SWITCH_NORMAL, .content = {switch_direction, switch_number}, .len = 2};
            int result = Send(_command_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
            if (result < 0) error("something went wrong here");
            cmd.type = CT_SWITCH_END;
            cmd.content[0] = SWITCH_END;
            cmd.len = 1;
            result = Send(_command_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
            if (result < 0) error("something went wrong here");
        }
    }
}

void rails_task() 
{
    Command cmd = {.type = CT_TRAIN_NORMAL, .content = {TRAIN_START}, .len = 1};
    Send(_command_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));

    cmd.type = CT_SWITCH_NORMAL;
    cmd.len = 2;
    for (int sw = 1; sw <= SWITCH_ONE_WAY_COUNT; sw++) {
        cmd.content[0] = SWITCH_STRAIGHT;
        cmd.content[1] = sw;
        Send(_command_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
	}

    int data[4][2] = {
        {SWITCH_BRANCH, SWITCH_TWO_WAY_1a}, 
        {SWITCH_STRAIGHT, SWITCH_TWO_WAY_1b}, 
        {SWITCH_BRANCH, SWITCH_TWO_WAY_2a}, 
        {SWITCH_STRAIGHT,SWITCH_TWO_WAY_2b}
        };

    for (int i = 0; i < 4; i++) {
        cmd.content[0] = data[i][0];
        cmd.content[1] = data[i][1];
        Send(_command_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
    }

    cmd.type = CT_SWITCH_END;
    cmd.len = 1;
    cmd.content[0] = SWITCH_END;
    Send(_command_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
}

void sensor_executor() 
{
    Command cmd = {.type = CT_SENSOR_FETCH, .content = { SENSOR_DATA_FETCH_BYTE }, .len = 1};
    char c;
    int count = 0, acknowledged = 0;
    char buffer[2];
    // trigger first sensor data
    int result = Send(_command_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
    if (result < 0) error("something went wrong here");
    while ( (c = Getc(_uart1_rx_server_tid, COM1) ) > -1) {
        buffer[count % 2] = c;
        if (count % 2) {
            _process_sensor_data('A' + count / 2, buffer[0] << 8 | buffer[1]);
        }
        count++;
        acknowledged = count / SENSOR_MODULE_BYTES_COUNT;
        count %= SENSOR_MODULE_BYTES_COUNT;
        if (acknowledged) {
            result = Send(_command_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
            if (result < 0) error("something went wrong here");
        }
    }
}

void terminal_executor() 
{
    CommandBuffer cmdBuf = {.content = {0}, .len = 0};
    char c;
    while ( (c = Getc(_uart2_rx_server_tid, COM2)) > -1) {
        Putc(_uart2_tx_server_tid, COM2, c);
        if (c != TERMINAL_ENTER_KEY_CODE && cmdBuf.len < COMMAND_MAX_LEN) {
            cmdBuf.content[cmdBuf.len++] = c;
        } else if (c == TERMINAL_ENTER_KEY_CODE) {
            if (cmdBuf.len <= COMMAND_MAX_LEN) {
                _process_command(&cmdBuf);
            }
            for (int i = 0; i < COMMAND_MAX_LEN; i++) cmdBuf.content[i]= 0;
            cmdBuf.len = 0;
        }
    }
}

void command_executor() 
{
    Command cmd = {.type = CT_FETCH_COMMAND};
    Queue command_queue = {.size = 0, .index = 0, .get_arg1 = _command_comparator1, .get_arg2 = _command_comparator2};
    int curr_ticks = 0, train_ticks = 0;
    int cmd_id, delay = INTER_COMMANDS_DELAY_TICKS;
    while(Send(_command_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd)) > -1) {
        switch (cmd.type)
        {
        case CT_FETCH_COMMAND:
        case CT_TRAIN_NORMAL:
        case CT_TRAIN_REVERSE:
        case CT_SWITCH_END:
        case CT_SENSOR_FETCH:
            delay = INTER_COMMANDS_DELAY_TICKS;
            break;
        case CT_SWITCH_NORMAL:
            delay = SWITCH_END_DELAY_TICKS;
            break;
        default:
            break;
        }

        if (cmd.type == CT_TRAIN_NORMAL) {
            cmd_buffer[cmd.id].ticks = train_ticks;
        }  else {
            cmd_buffer[cmd.id].ticks = curr_ticks;
        }

        if (cmd.type != CT_FETCH_COMMAND) {
            pq_insert(&command_queue, cmd.id);
        }

        if ((cmd_id = pq_get_min(&command_queue)) != -1) {
            cmd = cmd_buffer[cmd_id];
            // if (cmd.type == CT_TRAIN_NORMAL) {
            //     log("curr_ticks %d, cmd ticks %d", curr_ticks, cmd.ticks);
            // }
            if (cmd.ticks <= curr_ticks) {
                // if (cmd.type == CT_TRAIN_NORMAL) {
                //     log("begin to pop");
                // }
                pq_pop(&command_queue);
                for (int i = 0; i < cmd.len; i++) {
                    Putc(_uart1_tx_server_tid, COM1, cmd.content[i]);
                }
                if (cmd.type == CT_TRAIN_REVERSE) {
                    train_ticks = curr_ticks + TRAIN_STOP_DELAY_TICKS;
                    // log("curr_ticks %d, train_ticks %d", curr_ticks, train_ticks);
                }
                if (cmd.type == CT_TRAIN_NORMAL) {
                    train_ticks = curr_ticks + INTER_COMMANDS_DELAY_TICKS;
                }
            }
        }
        curr_ticks = Delay(_clock_server_tid, delay);
        // reset to fetch new command
        cmd.type = CT_FETCH_COMMAND;
    }
}

void command_server() 
{
    _init_command_server();
    Create(SENSOR_EXECUTOR_PRIORITY, sensor_executor);
    Create(TERMINAL_EXECUTOR_PRIORITY, terminal_executor);
    Create(COMMAND_EXECUTOR_PRIORITY, command_executor);
    Create(RAILS_TASK_PRIORITY, rails_task);
    Queue cmd_queue = {.size = 0, .index = 0};
    int client_tid, cmd_id, id = 0;
    while (Receive(&client_tid, (char *) &(cmd_buffer[id]), sizeof(cmd_buffer[id]))) {
        switch (cmd_buffer[id].type)
        {
        case CT_FETCH_COMMAND:
            if ( (cmd_id = deque(&cmd_queue)) != -1 ) {
                Reply(client_tid, (const char *) &(cmd_buffer[cmd_id]), sizeof(cmd_buffer[cmd_id]));
            } else {
                Reply(client_tid, (const char *) &(cmd_buffer[id]), sizeof(cmd_buffer[id]));
            }
            break;
        case CT_TRAIN_NORMAL:
        case CT_TRAIN_REVERSE:
        case CT_SWITCH_NORMAL:
        case CT_SWITCH_END:
        case CT_SENSOR_FETCH:
            enqueue(&cmd_queue, id);
            cmd_buffer[id].id = id;
            Reply(client_tid, (const char *) &(cmd_buffer[id]), sizeof(cmd_buffer[id]));
            id++;
            id %= QUEUE_SIZE;
            break;
        default:
            break;
        }
   }
}