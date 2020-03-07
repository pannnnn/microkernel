#include <shared.h>
#include <user.h>
#include <lib_ts7200.h>
#include <lib_periph_bwio.h>
#include <display.h>
#include <ds.h>
#include <track_data.h>
#include <trainset_data.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char content[COMMAND_MAX_LEN];
    int len;
} CommandBuffer;

typedef enum {
    TT_TRACK_A,
    TT_TRACK_B
} TRACK_TYPE;

typedef enum {
    CT_FETCH_COMMAND,
    CT_TRAIN_NORMAL,
    CT_TRAIN_REVERSE,
    CT_SWITCH_NORMAL,
    CT_SWITCH_END,
    CT_SENSOR_FETCH,
    CT_SENSOR_UPDATE,
    CT_GOTO_TARGET,
    CT_CHANGE_TRACK
} COMMAND_TYPE;

typedef struct {
    COMMAND_TYPE type;
    int id;
    char content[2];
    int len;
    int ticks;
} Command;

typedef struct {
    int length;
    char content[TRACK_MAX];
} Instructions;

static int _clock_server_tid = -1;
static int _uart1_rx_server_tid = -1;
static int _uart1_tx_server_tid = -1;
static int _uart2_rx_server_tid = -1;
static int _track_server_tid = -1;
static int _train_speed = 0;
static int _track_type = TT_TRACK_A;

static Command cmd_buffer[QUEUE_SIZE];

int _command_comparator1(int cmd_id) {
    return cmd_buffer[cmd_id].ticks;
}

int _command_comparator2(int dummy_value) {
    return 0;
}

void _init_track_server() 
{
	RegisterAs(TRACK_SERVER_NAME);
    _clock_server_tid = WhoIs(CLOCK_SERVER_NAME);
    _uart1_rx_server_tid = WhoIs(UART1_RX_SERVER_NAME);
    _uart1_tx_server_tid = WhoIs(UART1_TX_SERVER_NAME);
    _uart2_rx_server_tid = WhoIs(UART2_RX_SERVER_NAME);
    _track_server_tid = MyTid();
    _train_speed = 0;
    _track_type = TT_TRACK_A;
    u_memset(cmd_buffer, 0, QUEUE_SIZE * sizeof(Command));
}

void _process_command(CommandBuffer *cmdBuf) 
{
    if (cmdBuf->content[0] == 'q' && cmdBuf->len == 1) {
        Exit();
    }

    char *operation = strtok(cmdBuf->content, " ");
    if (!strcmp(operation, "tr")) {
    	char *train_number_c = strtok(NULL, " ");
		char *train_speed_c = strtok(NULL, " ");
        if (strtok(NULL, " ") != NULL) return;
        int train_number = (int) strtol(train_number_c, (char **)NULL, 10);
		int train_speed = (int) strtol(train_speed_c, (char **)NULL, 10);

        if (train_speed >= TRAIN_MIN_SPEED && train_speed <= TRAIN_MAX_SPEED) {
            _train_speed = train_speed;
        } else if (train_speed >= TRAIN_LIGHTS_ON + TRAIN_MIN_SPEED && train_speed <= TRAIN_LIGHTS_ON + TRAIN_MAX_SPEED) {
            _train_speed = train_speed - TRAIN_LIGHTS_ON;
        } else if (train_speed == TRAIN_REVERSE_DIRECTION_LIGHTS_ON || train_speed == TRAIN_REVERSE_DIRECTION_LIGHTS_OFF) {
            _train_speed = 0;
        } else {
            return;
        }
        Command cmd = {.type = CT_TRAIN_NORMAL, .content = {_train_speed, train_number}, .len = 2};
        int result = Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
        if (result < 0) u_error("something went wrong here");
    } else if(!strcmp(operation, "rv")) {
    	char *train_number_c = strtok(NULL, " ");
        if (strtok(NULL, " ") != NULL) return;
		int train_number = (int) strtol(train_number_c, (char **)NULL, 10);

        Command cmd1 = {.type = CT_TRAIN_REVERSE, .content = {0, train_number}, .len = 2};
        int result = Send(_track_server_tid, (const char *) &cmd1, sizeof(cmd1), (char *)&cmd1, sizeof(cmd1));
        if (result < 0) u_error("something went wrong here");
        Command cmd2 = {.type = CT_TRAIN_NORMAL, .content = {TRAIN_REVERSE_DIRECTION_LIGHTS_ON, train_number}, .len = 2};
        result = Send(_track_server_tid, (const char *) &cmd2, sizeof(cmd2), (char *)&cmd2, sizeof(cmd2));
        if (result < 0) u_error("something went wrong here");
        Command cmd3 = {.type = CT_TRAIN_NORMAL, .content = {_train_speed, train_number}, .len = 2};
        result = Send(_track_server_tid, (const char *) &cmd3, sizeof(cmd3), (char *)&cmd3, sizeof(cmd3));
        if (result < 0) u_error("something went wrong here");
    } else if(!strcmp(operation, "sw")) {
		char *switch_number_c = strtok(NULL, " ");
		char *switch_direction_s = strtok(NULL, " ");
        if (strtok(NULL, " ") != NULL) return;
        if (strlen(switch_direction_s) != 1 || (switch_direction_s[0] != 'S' && switch_direction_s[0] != 'C')) return;

		int switch_number = (int) strtol(switch_number_c, (char **)NULL, 10);
        char switch_direction = switch_direction_s[0];

        if (switch_number <= 0 || (switch_number > SWITCH_ONE_WAY_COUNT && switch_number < SWITCH_TWO_WAY_1a) || switch_number > SWITCH_TWO_WAY_2b) return;

        switch_direction = switch_direction == 'S' ? SWITCH_STRAIGHT : SWITCH_BRANCH;

        Command cmd = {.type = CT_SWITCH_NORMAL, .content = {switch_direction, switch_number}, .len = 2};
        int result = Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
        if (result < 0) u_error("something went wrong here");
        cmd.type = CT_SWITCH_END;
        cmd.content[0] = SWITCH_END;
        cmd.len = 1;
        result = Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
        if (result < 0) u_error("something went wrong here");
    
        char str[2] = { switch_direction, (char) switch_number};
        update_switch(str, 2);
    } else if(!strcmp(operation, "goto")) {
        int node_number;
    	char *train_number_c = strtok(NULL, " ");
        char *node_label = strtok(NULL, " ");
        if (strtok(NULL, " ") != NULL) return;
        int train_number = (int) strtol(train_number_c, (char **)NULL, 10);
        if (!strncmp(node_label, "EN", 2)) {
            int entry_number = (int) strtol(&node_label[2], (char **)NULL, 10);
            if (entry_number <= 0 || entry_number > 10) return;
            if (_track_type == TT_TRACK_A) {
                node_number = 122 + 2 * entry_number;
            } else {
                if (entry_number == 6 || entry_number == 8) return;
                else if (entry_number == 7) node_number = 134;
                else if (entry_number == 9) node_number = 136;
                else if (entry_number == 10) node_number = 138;
                else node_number = 122 + 2 * entry_number;
            }
        } else if (!strncmp(node_label, "EX", 2)) {
            int exit_number = (int) strtol(&node_label[2], (char **)NULL, 10);
            if (exit_number <= 0 || exit_number > 10) return;
            if (_track_type == TT_TRACK_A) {
                node_number = 122 + 2 * exit_number + 1;
            } else {
                if (exit_number == 6 || exit_number == 8) return;
                if (exit_number == 7) node_number = 135;
                else if (exit_number == 9) node_number = 137;
                else if (exit_number == 10) node_number = 139;
                else node_number = 122 + 2 * exit_number + 1;
            }
        } else if (node_label[0] >= 'A' && node_label[0] <= 'E') {
            int sensor_number = (int) strtol(&node_label[1], (char **)NULL, 10);
            if (sensor_number < 1 || sensor_number > 16) return;
            node_number = (node_label[0] - 'A') * 16 + sensor_number - 1;
        } else {
            int switch_number = (int) strtol(node_label, (char **)NULL, 10);
            if (switch_number <= 0 || (switch_number > SWITCH_ONE_WAY_COUNT && switch_number < SWITCH_TWO_WAY_1a) || switch_number > SWITCH_TWO_WAY_2b) return;
            if (switch_number >= SWITCH_TWO_WAY_1a && switch_number <= SWITCH_TWO_WAY_2b) {
                node_number = 116 + switch_number - SWITCH_TWO_WAY_1a;
            } else {
                node_number = 79 + switch_number;
            }
        }

        Command cmd = {.type = CT_GOTO_TARGET, .content = {train_number, node_number}, .len = 2};
        int result = Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
        if (result < 0) u_error("something went wrong here");
    } else if(!strcmp(operation, "track")) {
        TRACK_TYPE track_type;
    	char *train_label = strtok(NULL, " ");
        if (strtok(NULL, " ") != NULL) return;
        if (strlen(train_label) != 1 || train_label[0] != 'a' || train_label[1] != 'b') return;
        if (train_label[0] != 'a') track_type = TT_TRACK_A;
        if (train_label[0] != 'b') track_type = TT_TRACK_B;
        Command cmd = {.type = CT_CHANGE_TRACK, .content = {track_type}, .len = 1};
        int result = Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
        if (result < 0) u_error("something went wrong here");
    } else {
        u_error("Invalid command");
        return;
    }
}

void routing(int src_index, track_node *track, Instructions *instructions, int dest_index) {
    int prev_index, next_index, next_D, min_index, min_D = MAXIMUM_SIGNED_INT;
    int prev[TRACK_MAX], D[TRACK_MAX], visited[TRACK_MAX];
    int_memset(prev, -1, TRACK_MAX);
    int_memset(D, MAXIMUM_SIGNED_INT, TRACK_MAX);
    int_memset(visited, 0, TRACK_MAX);
    
    // initialize
    D[src_index] = 0;
    visited[src_index] = 1;
    prev[src_index] = src_index;
    prev_index = src_index;

    next_index = track[src_index].edge[DIR_AHEAD].dest - track;
    D[next_index] = track[src_index].edge[DIR_AHEAD].dist;
    prev[next_index] = prev_index;

    // traverse
    int unreachable = 0, complementary_index;
    while (!visited[dest_index]) {
        // find w not in N' such that D(w) is a minimum
        for (int i = 0; i < TRACK_MAX; i++) {
            if (min_D > D[i] && !visited[i]) {
                min_D = D[i];
                min_index = i;
            }
        }
        if (min_D == MAXIMUM_SIGNED_INT) {
            unreachable = 1;
            break;
        };
        // add w to N'
        visited[min_index] = 1;
        min_D = MAXIMUM_SIGNED_INT;
        prev_index = min_index;

        // update D(v) for all v adjacent to w and not in N'
        if (track[min_index].type != NODE_BRANCH) {
            if (track[min_index].type != NODE_EXIT) {
                next_index = track[min_index].edge[DIR_AHEAD].dest - track;
                next_D = D[min_index] + track[min_index].edge[DIR_AHEAD].dist;
                if (D[next_index] > next_D) {
                    D[next_index] = next_D;
                    prev[next_index] = prev_index;
                }
            }
        } else {
            next_index = track[min_index].edge[DIR_STRAIGHT].dest - track;
            next_D = D[min_index] + track[min_index].edge[DIR_STRAIGHT].dist;
            if (D[next_index] > next_D) {
                D[next_index] = next_D;
                prev[next_index] = prev_index;
            }
            next_index = track[min_index].edge[DIR_CURVED].dest - track;
            next_D = D[min_index] + track[min_index].edge[DIR_CURVED].dist;
            if (D[next_index] > next_D) {
                D[next_index] = next_D;
                prev[next_index] = prev_index;
            }
        }
    }

    if (unreachable) {
        complementary_index = dest_index ^ 1;
        if (prev[complementary_index] != -1) {
            u_info("[routing] Destination changed %s -> %s", track[dest_index].name, track[complementary_index].name);
            prev[dest_index] = complementary_index;
        } else {
            u_info("[routing] Explore the other direction by changing source %s -> %s", track[src_index].name, track[src_index ^ 1].name);
            routing(src_index ^ 1, track, instructions, dest_index);
            return;
        }
    }

    General_Buffer format_buffer = {.index = 0};
    int index = dest_index, path_count = 0;
    int paths[TRACK_MAX];
    paths[path_count++] = index;
    while (prev[index] != index) {
        paths[path_count++] = prev[index];
        index = prev[index];
    }
    for (int i = path_count - 1; i >= 0; i--) {
        u_sprintf(&format_buffer, "%s ", track[paths[i]].name);
    }
    format_buffer.content[format_buffer.index] = '\0';
    u_info("[routing] Path: %s", format_buffer.content);
}

void rails_task() 
{
    Command cmd = {.type = CT_TRAIN_NORMAL, .content = {TRAIN_START}, .len = 1};
    Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));

    for (int sw = 1; sw <= SWITCH_ONE_WAY_COUNT; sw++) {
        cmd.type = CT_SWITCH_NORMAL;
        cmd.len = 2;
        cmd.content[0] = SWITCH_STRAIGHT;
        cmd.content[1] = sw;
        update_switch(cmd.content, 2);
        Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
	}

    int data[4][3] = {
        {SWITCH_BRANCH, SWITCH_TWO_WAY_1a}, 
        {SWITCH_STRAIGHT, SWITCH_TWO_WAY_1b}, 
        {SWITCH_BRANCH, SWITCH_TWO_WAY_2a}, 
        {SWITCH_STRAIGHT,SWITCH_TWO_WAY_2b}
    };

    for (int i = 0; i < 4; i++) {
        cmd.type = CT_SWITCH_NORMAL;
        cmd.len = 2;
        cmd.content[0] = data[i][0];
        cmd.content[1] = data[i][1];
        update_switch(cmd.content, 2);
        Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
    }

    cmd.type = CT_SWITCH_END;
    cmd.len = 1;
    cmd.content[0] = SWITCH_END;
    Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
}

void sensor_executor() 
{
    char c, sensor_label, sensor_number, sensor_value;
    int count = 0, result, acknowledged;
    Command cmd = {.type = CT_SENSOR_FETCH, .content = { SENSOR_DATA_FETCH_BYTE }, .len = 1};
    result = Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
    if (result < 0) u_error("something went wrong here");
    while ( (c = Getc(_uart1_rx_server_tid, COM1) ) > -1) {
        sensor_label = 'A' + count / 2;
        for (int i = 0; i < 8; i++) {
            if (c & (1 << (7 - i))) {
                cmd.type = CT_SENSOR_UPDATE;
                cmd.len = 1;
                sensor_number = 8 * count + i;
                // sensor number is always between 0-79
                cmd.content[0] = sensor_number;
                result = Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
                if (result < 0) u_error("something went wrong here");

                sensor_value = i + 1 + 8 * (count % 2);
                char str[4] = {sensor_label, '\0', '\0', '\0'};
                if (sensor_value < 10) {
                    str[1] = '0' + sensor_value % 10;
                } else {
                    str[1] = '0' + sensor_value / 10;
                    str[2] = '0' + sensor_value % 10;
                }
                update_sensor(str, 4);
            }
        }
        count++;
        acknowledged = count / SENSOR_MODULE_BYTES_COUNT;
        count %= SENSOR_MODULE_BYTES_COUNT;
        if (acknowledged) {
            cmd.type = CT_SENSOR_FETCH;
            cmd.len = 1;
            cmd.content[0] = SENSOR_DATA_FETCH_BYTE;
            result = Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
            if (result < 0) u_error("something went wrong here");
        }
    }
}

void terminal_executor() 
{
    CommandBuffer cmdBuf = {.content = {0}, .len = 0};
    char c;
    int cursor_index = 0;
    while ( (c = Getc(_uart2_rx_server_tid, COM2)) > -1) {
        if (c == BACKSPACE_KEY_CODE) {
            if (cursor_index > 0) {
                PutStr(BACKSPACE, sizeof(BACKSPACE) - 1);
                PutStr(&c, 1);
                cursor_index--;
            }
            if (cmdBuf.len > 0 && cmdBuf.len <= COMMAND_MAX_LEN) cmdBuf.len--;
        } else if (c != TERMINAL_ENTER_KEY_CODE && cmdBuf.len < COMMAND_MAX_LEN) {
            PutStr(&c, 1);
            cmdBuf.content[cmdBuf.len++] = c;
            cursor_index++;
        } else if (c == TERMINAL_ENTER_KEY_CODE) {
            PutStr(CLEAR_START_OF_LINE, sizeof(CLEAR_START_OF_LINE) - 1);
            PutStr(PATCH_COMMAND_LINE_PREFIX, sizeof(PATCH_COMMAND_LINE_PREFIX) - 1);
            if (cmdBuf.len < COMMAND_MAX_LEN) {
                cmdBuf.content[cmdBuf.len] = '\0';
                _process_command(&cmdBuf);
            }
            for (int i = 0; i < COMMAND_MAX_LEN; i++) cmdBuf.content[i]= 0;
            cmdBuf.len = 0;
            cursor_index = 0;
        } else if (cursor_index < TERMINAL_OUTPUT_MAX_LEN) {
            cursor_index++;
            PutStr(&c, 1);
        }
    }
}

void command_executor() 
{
    Command cmd = {.type = CT_FETCH_COMMAND};
    Queue command_queue = {.size = 0, .index = 0, .get_arg1 = _command_comparator1, .get_arg2 = _command_comparator2};
    int curr_ticks = 0, train_ticks = 0;
    int cmd_id;
    while(Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd)) > -1) {
        if (cmd.type == CT_TRAIN_REVERSE) {
            cmd_buffer[cmd.id].ticks = train_ticks;
            train_ticks += TRAIN_STOP_DELAY_TICKS;
        } else if (cmd.type == CT_TRAIN_NORMAL) {
            cmd_buffer[cmd.id].ticks = train_ticks;
            train_ticks += INTER_COMMANDS_DELAY_TICKS;
        }  else if (cmd.type != CT_FETCH_COMMAND) {
            cmd_buffer[cmd.id].ticks = curr_ticks;
        }

        if (cmd.type != CT_FETCH_COMMAND) {
            pq_insert(&command_queue, cmd.id);
        }

        if ((cmd_id = pq_get_min(&command_queue)) != -1) {
            cmd = cmd_buffer[cmd_id];
            if (cmd.ticks <= curr_ticks) {
                pq_pop(&command_queue);
                for (int i = 0; i < cmd.len; i++) {
                    Putc(_uart1_tx_server_tid, COM1, cmd.content[i]);
                }
                if (cmd.type == CT_TRAIN_NORMAL) {
                    if (cmd.content[0] == TRAIN_START) {
                        u_info("[%d][tr] Enable trainset", curr_ticks);
                    } else if (cmd.content[0] == TRAIN_REVERSE_DIRECTION_LIGHTS_ON) {
                        u_info("[%d][rv] Train %d reverse", curr_ticks, cmd.content[1]);
                    } else {
                        u_info("[%d][tr] Train %d set speed to %d", curr_ticks, cmd.content[1], cmd.content[0]);
                    }
                }
                if (cmd.type == CT_TRAIN_REVERSE) {
                    u_info("[%d][tr] Train %d stops ...", curr_ticks, cmd.content[1]);
                }
                if (cmd.type == CT_SWITCH_NORMAL) {
                    u_info("[%d][sw] Switch %d set to %c", curr_ticks, cmd.content[1], cmd.content[0] == SWITCH_STRAIGHT ? SWITCH_STRAIGHT_SYMBOL : SWITCH_BRANCH_SYMBOL);
                }
                if (cmd.type == CT_SWITCH_END) {
                    u_info("[%d][sw] Switch ended", curr_ticks);
                }
            }
        }

        curr_ticks = Delay(_clock_server_tid, INTER_COMMANDS_DELAY_TICKS);

        if (train_ticks < curr_ticks) train_ticks = curr_ticks;
        // reset to fetch new command
        cmd.type = CT_FETCH_COMMAND;
    }
}

void track_server() 
{
    _init_track_server();
    Create(SENSOR_EXECUTOR_PRIORITY, sensor_executor);
    Create(TERMINAL_EXECUTOR_PRIORITY, terminal_executor);
    Create(COMMAND_EXECUTOR_PRIORITY, command_executor);
    Create(RAILS_TASK_PRIORITY, rails_task);

    track_node track[TRACK_MAX];
    Train_Node trainset[TRAIN_MAX];
    init_tracka(track);
    init_trainset(trainset);

    int sensor_node_number = -1;
    Instructions instructions = {.length = 0};
    
    Queue cmd_queue = {.size = 0, .index = 0};
    int client_tid, cmd_id, train_number, node_number, id = 0;
    TRACK_TYPE track_type;
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
            id += 1;
            id %= QUEUE_SIZE;
            Reply(client_tid, (const char *) &(cmd_buffer[id]), sizeof(cmd_buffer[id]));
            break;
        case CT_SENSOR_UPDATE:
            if (sensor_node_number != cmd_buffer[id].content[0]) {
                sensor_node_number = cmd_buffer[id].content[0];
            }
            Reply(client_tid, (const char *) &(cmd_buffer[id]), sizeof(cmd_buffer[id]));
            break;
        case CT_GOTO_TARGET:
            train_number = cmd_buffer[id].content[0];
            node_number = cmd_buffer[id].content[1];
            if (sensor_node_number != -1) {
                routing(sensor_node_number, track, &instructions, node_number);
            }
            Reply(client_tid, (const char *) &(cmd_buffer[id]), sizeof(cmd_buffer[id]));
            break;
        case CT_CHANGE_TRACK:
            track_type = cmd_buffer[id].content[0];
            _track_type = track_type;
            if (track_type == TT_TRACK_A) init_tracka(track);
            if (track_type == TT_TRACK_B) init_trackb(track);
            Reply(client_tid, (const char *) &(cmd_buffer[id]), sizeof(cmd_buffer[id]));
            break;
        default:
            break;
        }
   }
}