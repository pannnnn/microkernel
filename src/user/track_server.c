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
    CT_FETCH_COMMAND,
    CT_TRAIN_NORMAL,
    CT_TRAIN_REVERSE,
    CT_SWITCH_NORMAL,
    CT_SWITCH_END,
    CT_SENSOR_FETCH,
    CT_SENSOR_UPDATE,
    CT_GOTO_TARGET,
    CT_CHANGE_TRACK,
    CT_SET_TRAIN_NUMBER,
    CT_SET_VELOCITY_LEVEL
} COMMAND_TYPE;

typedef struct {
    COMMAND_TYPE type;
    int id;
    char content[2];
    int len;
    int ticks;
    int priority;
} Command;

static int _clock_server_tid = -1;
static int _uart1_rx_server_tid = -1;
static int _uart1_tx_server_tid = -1;
static int _uart2_rx_server_tid = -1;
static int _track_server_tid = -1;
static int _train_speed = 0;
static int curr_ticks = 0;
static int _track_type = TT_TRACK_A;
static int _track_a_loop_switch[TRACK_A_LOOP_SWITCH];
static track_node _track[TRACK_MAX];
static Train_Node _trainset[TRAIN_MAX];
static int _switch_map[TRACK_MAX];
static int distance_to_destination[TRACK_MAX];

static Command cmd_buffer[QUEUE_SIZE];

int _command_comparator1(int cmd_id) 
{
    return cmd_buffer[cmd_id].ticks;
}

int _command_comparator2(int cmd_id) 
{
    return cmd_buffer[cmd_id].priority;
}

void _init_switches() 
{
    // this is to close the loop for the train to run
    _track_a_loop_switch[0] = 8;
    _track_a_loop_switch[1] = 9;
    _track_a_loop_switch[2] = 14;
    _track_a_loop_switch[3] = 15;
    _track_a_loop_switch[4] = 11;
    _track_a_loop_switch[5] = 5;
    _track_a_loop_switch[6] = 18;
    
    int_memset(_switch_map, 0, TRACK_MAX);

    int node_number;
    for (int sw = 1; sw <= SWITCH_ONE_WAY_COUNT; sw++) {
        node_number = switch_number_to_node_number(sw);
        _switch_map[node_number] = DIR_STRAIGHT;
    }
    for (int sw = 153; sw < 157; sw++) {
        node_number = switch_number_to_node_number(sw);
        _switch_map[node_number] = DIR_STRAIGHT;
    }
    for (int i = 0; i < TRACK_A_LOOP_SWITCH; i++) {
        node_number = switch_number_to_node_number(_track_a_loop_switch[i]);
        _switch_map[node_number] = DIR_CURVED;
    }
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
    int_memset(distance_to_destination, -1, TRACK_MAX);

    _init_switches();
    init_tracka(_track);
    init_trainset(_trainset);
}

void _track_a_loop() {
    Command cmd;
    for (int i = 0; i < TRACK_A_LOOP_SWITCH; i++) 
    {
        cmd.type = CT_SWITCH_NORMAL;
        cmd.len = 2;
        cmd.content[0] = SWITCH_BRANCH;
        cmd.content[1] = _track_a_loop_switch[i];
        cmd.priority = 0;
        Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
    }
}

int _get_train_index(int train_number) 
{
    switch (train_number)
    {
        case 1:
            return 0;
        case 24:
            return 1;
        case 58:
            return 2;
        case 74:
            return 3;
        case 78:
            return 4;
        case 79:
            return 5;
        default:
            return -1;
    }
}

int _get_distance(int start_node_number, int end_node_number) 
{
    if (_track[start_node_number].type != NODE_SENSOR || _track[start_node_number].type != NODE_SENSOR) return -1;
    int count = 0, distance = 0, curr_node_number = start_node_number;
    while(curr_node_number != end_node_number) {
        if (_track[curr_node_number].type != NODE_BRANCH) {
            distance += _track[curr_node_number].edge[DIR_AHEAD].dist;
            curr_node_number = _track[curr_node_number].edge[DIR_AHEAD].dest - _track;
        } else {
            if (_track[curr_node_number].type == NODE_EXIT) return -1;
            int dir_type = _switch_map[curr_node_number];
            distance += _track[curr_node_number].edge[dir_type].dist;
            curr_node_number = _track[curr_node_number].edge[dir_type].dest - _track;
        }
        count += 1;
        if (count == 20) return -1;
    }
    return distance;
}

void _update_switch_map(int switch_direction, int switch_number) 
{
    int node_number = switch_number_to_node_number(switch_number);
    if (node_number == -1) return;
    _switch_map[node_number] = switch_direction - SWITCH_STRAIGHT;
}

int _get_next_hop_node_number(int start_node_number)
{
    int total_distance = 0, curr_node_number = start_node_number;
    while (total_distance < 500) {
        if (_track[curr_node_number].type != NODE_BRANCH) {
            if (_track[curr_node_number].type == NODE_EXIT) return -1;
            total_distance += _track[curr_node_number].edge[DIR_AHEAD].dist;
            curr_node_number = _track[curr_node_number].edge[DIR_AHEAD].dest - _track;
        } else {
            int dir_type = _switch_map[curr_node_number];
            total_distance += _track[curr_node_number].edge[dir_type].dist;
            curr_node_number = _track[curr_node_number].edge[dir_type].dest - _track;
        }
    }
    return curr_node_number;
}

int _get_next_sensor_node_number(int start_node_number) {
    int curr_node_number = _track[start_node_number].edge[DIR_AHEAD].dest - _track;
    while (_track[curr_node_number].type != NODE_SENSOR) {
        if (_track[curr_node_number].type != NODE_BRANCH) {
            if (_track[curr_node_number].type == NODE_EXIT) return -1;
            curr_node_number = _track[curr_node_number].edge[DIR_AHEAD].dest - _track;
        } else {
            int dir_type = _switch_map[curr_node_number];
            curr_node_number = _track[curr_node_number].edge[dir_type].dest - _track;
        }
    }
    return curr_node_number;
}

void _get_instructions(int *paths, int path_count, Queue *cmd_queue, int *id) 
{
    int total_distance = 0, should_switch_end = 0;
    distance_to_destination[paths[0]] = 0;
    for (int i = 0; i < path_count - 1; i++) {
        if (_track[paths[i]].type != NODE_BRANCH) {
            total_distance += _track[paths[i]].edge[DIR_AHEAD].dist;
            // this is actually distance to source, we will revert it later
            distance_to_destination[paths[i+1]] = total_distance;
            continue;
        }
        int expected_next_node_number = paths[i+1];
        int dir_type = _switch_map[paths[i]];
        int next_node_number = _track[paths[i]].edge[dir_type].dest - _track;

        if (expected_next_node_number != next_node_number) {
            total_distance += _track[paths[i]].edge[dir_type ^ 1].dist;
            enqueue(cmd_queue, *id);
            cmd_buffer[*id].id = *id;
            cmd_buffer[*id].type = CT_SWITCH_NORMAL;
            cmd_buffer[*id].len = 2;
            cmd_buffer[*id].content[0] = (dir_type ^ 1) + SWITCH_STRAIGHT;
            cmd_buffer[*id].content[1] = _track[paths[i]].num;
            cmd_buffer[*id].priority = 0;
            (*id)++;
            (*id) %= QUEUE_SIZE;
            _switch_map[paths[i]] = dir_type ^ 1;
            should_switch_end = 1;
        } else {
            total_distance += _track[paths[i]].edge[dir_type].dist;
        }
        // this is actually distance to source, we will revert it later
        distance_to_destination[paths[i+1]] = total_distance;
    }
    for (int i = 0; i < path_count; i++) {
        distance_to_destination[paths[i]] = total_distance - distance_to_destination[paths[i]];
    }
    if (should_switch_end) {
        enqueue(cmd_queue, *id);
        cmd_buffer[*id].id = *id;
        cmd_buffer[*id].type = CT_SWITCH_END;
        cmd_buffer[*id].len = 1;
        cmd_buffer[*id].content[0] = SWITCH_END;
        cmd_buffer[*id].priority = 0;
        (*id)++;
        (*id) %= QUEUE_SIZE;
    }
    // revert to get distance to destination
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
        Command cmd = {.type = CT_TRAIN_NORMAL, .content = {train_speed, train_number}, .len = 2, .priority = 1};
        int result = Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
        if (result < 0) u_error("something went wrong here");
    } else if(!strcmp(operation, "rv")) {
    	char *train_number_c = strtok(NULL, " ");
        if (strtok(NULL, " ") != NULL) return;
		int train_number = (int) strtol(train_number_c, (char **)NULL, 10);

        Command cmd1 = {.type = CT_TRAIN_REVERSE, .content = {0, train_number}, .len = 2, .priority = 1};
        int result = Send(_track_server_tid, (const char *) &cmd1, sizeof(cmd1), (char *)&cmd1, sizeof(cmd1));
        if (result < 0) u_error("something went wrong here");
        Command cmd2 = {.type = CT_TRAIN_NORMAL, .content = {TRAIN_REVERSE_DIRECTION_LIGHTS_ON, train_number}, .len = 2, .priority = 1};
        result = Send(_track_server_tid, (const char *) &cmd2, sizeof(cmd2), (char *)&cmd2, sizeof(cmd2));
        if (result < 0) u_error("something went wrong here");
        Command cmd3 = {.type = CT_TRAIN_NORMAL, .content = {_train_speed, train_number}, .len = 2, .priority = 1};
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

        Command cmd = {.type = CT_SWITCH_NORMAL, .content = {switch_direction, switch_number}, .len = 2, .priority = 1};
        int result = Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
        if (result < 0) u_error("something went wrong here");
        cmd.type = CT_SWITCH_END;
        cmd.content[0] = SWITCH_END;
        cmd.len = 1;
        cmd.priority = 1;
        result = Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
        if (result < 0) u_error("something went wrong here");
    } else if(!strcmp(operation, "goto")) {
        int node_number;
        char *node_label = strtok(NULL, " ");
        if (strtok(NULL, " ") != NULL) return;
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

        Command cmd = {.type = CT_GOTO_TARGET, .content = {node_number}, .len = 1};
        int result = Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
        if (result < 0) u_error("something went wrong here");
    } else if(!strcmp(operation, "track")) {
        TRACK_TYPE track_type;
    	char *train_label = strtok(NULL, " ");
        if (strtok(NULL, " ") != NULL) return;
        if (strlen(train_label) != 1 || (train_label[0] != 'a' && train_label[0] != 'b')) return;
        if (train_label[0] == 'a') track_type = TT_TRACK_A;
        if (train_label[0] == 'b') track_type = TT_TRACK_B;
        Command cmd = {.type = CT_CHANGE_TRACK, .content = {track_type}, .len = 1};
        int result = Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
        if (result < 0) u_error("something went wrong here");
    } else if(!strcmp(operation, "train")) {
    	char *train_number_c = strtok(NULL, " ");
        int train_number = (int) strtol(train_number_c, (char **)NULL, 10);
        int result = _get_train_index(train_number);
        if (result == -1) {
            u_info("Invalid train number %d", train_number);
        } else {
            int train_index = result;
            Command cmd = {.type = CT_SET_TRAIN_NUMBER, .content = {train_index}, .len = 1};
            int result = Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
            if (result < 0) u_error("something went wrong here");
        }
    } else if(!strcmp(operation, "level")) {
    	char *velocity_level_c = strtok(NULL, " ");
        int velocity_level = (int) strtol(velocity_level_c, (char **)NULL, 10);
        if (velocity_level < 0 || velocity_level > 2) {
            u_info("Invalid velocity level %d. [0,1,2]", velocity_level);
        } else {
            Command cmd = {.type = CT_SET_VELOCITY_LEVEL, .content = {velocity_level}, .len = 1};
            int result = Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
            if (result < 0) u_error("something went wrong here");
        }
    } else {
        u_error("Invalid command");
        return;
    }
}

int _issue_stop_command(int curr_node_number, Queue *cmd_queue, int *id, int train_index, int velocity_level) {
    int next_node_number = _get_next_sensor_node_number(curr_node_number);
    if (next_node_number == -1)  {
        u_error("[goto] Can not get next node number for %s", _track[curr_node_number].name);
        return -1;
    }
    int dtd = distance_to_destination[curr_node_number];
    int next_dtd = distance_to_destination[next_node_number];
    if (dtd == -1 || next_dtd == -1) {
        u_debug("[goto] Enter uncovered path from %s to %s", _track[curr_node_number].name, _track[next_node_number].name);
        return -1;
    }

    int stopping_distance = _trainset[train_index].curr_stopping_distance;
    int velocity = _trainset[train_index].curr_velocity;
    if (dtd > stopping_distance && next_dtd < stopping_distance) {
        enqueue(cmd_queue, *id);
        cmd_buffer[*id].id = *id;
        cmd_buffer[*id].type = CT_TRAIN_NORMAL;
        cmd_buffer[*id].len = 2;
        cmd_buffer[*id].content[0] = 0;
        cmd_buffer[*id].content[1] = _trainset[train_index].number;
        cmd_buffer[*id].priority = 0;

        int unfinished_distance = dtd - stopping_distance;
        int ticks_to_destination = unfinished_distance * 100 / velocity;
        cmd_buffer[*id].ticks = curr_ticks + ticks_to_destination;

        (*id)++;
        (*id) %= QUEUE_SIZE;
        int_memset(distance_to_destination, -1, TRACK_MAX);
        return 0;
    }
    // 1 means not reaching closely to destination, keep pooling sensors
    return 1;
}


void calibrate_speed(int train_index, int velocity_level, int prev_node_number, int curr_node_number) {
    Train_Node *train = &_trainset[train_index];
    if (train->last_read_time == 0) {
        train->last_read_time = read_timer();
        return;
    }
    if (prev_node_number == -1 || curr_node_number == -1) return;
    
    train->time_elapsed = get_time_elapsed_with_update(&train->last_read_time);

    int d = _get_distance(prev_node_number, curr_node_number) * 1000 / train->time_elapsed;
    if (d == -1)  {
        u_info("[calibrate_speed] check sensor  %s and %s", _track[prev_node_number].name, _track[curr_node_number].name);
        return;
    }
    int c = train->measurement[_track_type].velocity[velocity_level];
    train->measurement[_track_type].velocity[velocity_level] = (7 * c + d) >> 3;
    u_info("[calibration] Converging velocity %d", train->measurement[_track_type].velocity[velocity_level]);
}

void routing(int src_index, int dest_index, Queue *cmd_queue, int *id) {
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

    next_index = _track[src_index].edge[DIR_AHEAD].dest - _track;
    D[next_index] = _track[src_index].edge[DIR_AHEAD].dist;
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
        if (_track[min_index].type != NODE_BRANCH) {
            if (_track[min_index].type != NODE_EXIT) {
                next_index = _track[min_index].edge[DIR_AHEAD].dest - _track;
                next_D = D[min_index] + _track[min_index].edge[DIR_AHEAD].dist;
                if (D[next_index] > next_D) {
                    D[next_index] = next_D;
                    prev[next_index] = prev_index;
                }
            }
        } else {
            next_index = _track[min_index].edge[DIR_STRAIGHT].dest - _track;
            next_D = D[min_index] + _track[min_index].edge[DIR_STRAIGHT].dist;
            if (D[next_index] > next_D) {
                D[next_index] = next_D;
                prev[next_index] = prev_index;
            }
            next_index = _track[min_index].edge[DIR_CURVED].dest - _track;
            next_D = D[min_index] + _track[min_index].edge[DIR_CURVED].dist;
            if (D[next_index] > next_D) {
                D[next_index] = next_D;
                prev[next_index] = prev_index;
            }
        }
    }

    if (unreachable) {
        complementary_index = dest_index ^ 1;
        if (prev[complementary_index] != -1) {
            u_info("[routing] Destination changed %s -> %s", _track[dest_index].name, _track[complementary_index].name);
        } else {
            u_info("[routing] Explore the other direction by changing source %s -> %s", _track[src_index].name, _track[src_index ^ 1].name);
            routing(src_index ^ 1, dest_index, cmd_queue, id);
            return;
        }
    }

    General_Buffer format_buffer = {.index = 0};
    int index = dest_index, path_count = 0;
    int paths[TRACK_MAX], ordered_path[TRACK_MAX];
    if (unreachable) index = complementary_index;
    paths[path_count++] = index;
    while (prev[index] != index) {
        paths[path_count++] = prev[index];
        index = prev[index];
    }
    for (int i = path_count - 1; i >= 0; i--) {
        u_sprintf(&format_buffer, "%s ", _track[paths[i]].name);
        ordered_path[path_count - 1 - i] = paths[i];
    }
    format_buffer.content[format_buffer.index] = '\0';
    u_info("[routing] Path: %s", format_buffer.content);
    _get_instructions(ordered_path, path_count, cmd_queue, id);
}

void rails_task() 
{
    Command cmd = {.type = CT_TRAIN_NORMAL, .content = {TRAIN_START}, .len = 1, .priority = 1};
    Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));

    for (int sw = 1; sw <= SWITCH_ONE_WAY_COUNT; sw++) {
        cmd.type = CT_SWITCH_NORMAL;
        cmd.len = 2;
        if (sw == 8 || sw == 9 || sw == 14 || sw == 15 || sw == 11 || sw == 5 || sw == 18) {
            cmd.content[0] = SWITCH_BRANCH;
        } else {
            cmd.content[0] = SWITCH_STRAIGHT;
        }
        cmd.content[1] = sw;
        cmd.priority = 1;
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
        cmd.priority = 1;
        Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
    }

    cmd.type = CT_SWITCH_END;
    cmd.len = 1;
    cmd.content[0] = SWITCH_END;
    cmd.priority = 1;
    Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
}

void sensor_executor() 
{
    char c, sensor_label, sensor_number, sensor_value;
    int count = 0, result, acknowledged;
    char byte[SENSOR_MODULE_BYTES_COUNT];
    char_memset(byte, 0, SENSOR_MODULE_BYTES_COUNT);
    Command cmd = {.type = CT_SENSOR_FETCH, .content = { SENSOR_DATA_FETCH_BYTE }, .len = 1};
    result = Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd));
    if (result < 0) u_error("something went wrong here");
    while ( (c = Getc(_uart1_rx_server_tid, COM1) ) > -1) {
        sensor_label = 'A' + count / 2;
        for (int i = 0; i < 8; i++) {
            if ((c & (1 << (7 - i))) && !(byte[count] & (1 << (7 - i)))) {
                cmd.type = CT_SENSOR_UPDATE;
                cmd.len = 1;
                sensor_number = 8 * count + i;
                // sensor number is always between 0-79
                cmd.content[0] = sensor_number;
                cmd.priority = 1;
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
        byte[count] = c;
        count++;
        acknowledged = count / SENSOR_MODULE_BYTES_COUNT;
        count %= SENSOR_MODULE_BYTES_COUNT;
        if (acknowledged) {
            cmd.type = CT_SENSOR_FETCH;
            cmd.len = 1;
            cmd.content[0] = SENSOR_DATA_FETCH_BYTE;
            cmd.priority = 1;
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
    curr_ticks = 0;
    Command cmd = {.type = CT_FETCH_COMMAND, .priority = 1};
    Queue command_queue = {.size = 0, .index = 0, .get_arg1 = _command_comparator1, .get_arg2 = _command_comparator2};
    int train_ticks = 0;
    int cmd_id;
    while(Send(_track_server_tid, (const char *) &cmd, sizeof(cmd), (char *)&cmd, sizeof(cmd)) > -1) {
        if (cmd_buffer[cmd.id].priority != 0) {
            if (cmd.type == CT_TRAIN_REVERSE) {
                cmd_buffer[cmd.id].ticks = train_ticks;
                train_ticks += TRAIN_STOP_DELAY_TICKS;
            } else if (cmd.type == CT_TRAIN_NORMAL) {
                cmd_buffer[cmd.id].ticks = train_ticks;
                train_ticks += INTER_COMMANDS_DELAY_TICKS;
            }  else if (cmd.type != CT_FETCH_COMMAND) {
                cmd_buffer[cmd.id].ticks = curr_ticks;
            }
        } else {
            u_debug("[command] id %d get priority zero task with type %d", cmd_buffer[cmd.id].id, cmd_buffer[cmd.id].type);
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
                    update_switch(cmd.content, 2);
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
        cmd.priority = 1;
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

    Queue cmd_queue = {.size = 0, .index = 0};
    int client_tid, cmd_id, id = 0, train_index = 0, velocity_level = 0, prev_node_number = -1, curr_node_number = -1, goto_mode = 0;
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
            if (cmd_buffer[id].type == CT_SWITCH_NORMAL) {
                _update_switch_map(cmd_buffer[id].content[0], cmd_buffer[id].content[1]);
            }
            enqueue(&cmd_queue, id);
            cmd_buffer[id].id = id;
            id += 1;
            id %= QUEUE_SIZE;
            Reply(client_tid, (const char *) &(cmd_buffer[id]), sizeof(cmd_buffer[id]));
            break;
        case CT_SENSOR_UPDATE:
            curr_node_number = cmd_buffer[id].content[0];
            if (goto_mode) {
                velocity_level = _trainset[train_index].curr_level;
                int result = _issue_stop_command(curr_node_number, &cmd_queue, &id, train_index, _trainset[train_index].curr_level);
                if (result == 0) {
                    u_info("[goto] Schedule stop command");
                    goto_mode = 0;
                }
                if (result == 1) {
                    u_info("[goto] Distance to destination %d", distance_to_destination[curr_node_number]);
                }
            }
            // this is to calibrate velocity
            // calibrate_speed(train_index, velocity_level, prev_node_number, curr_node_number);
            prev_node_number = curr_node_number;
            Reply(client_tid, (const char *) &(cmd_buffer[id]), sizeof(cmd_buffer[id]));
            break;
        case CT_GOTO_TARGET:
            if (!train_index) {
                u_error("[goto] Please set train number before goto");
            } else{
                int dst_node_number = cmd_buffer[id].content[0];
                if (curr_node_number != -1) {
                    int next_hop_node_number = _get_next_hop_node_number(curr_node_number);
                    if (next_hop_node_number != -1) {
                        routing(next_hop_node_number, dst_node_number, &cmd_queue, &id);
                        goto_mode = 1;
                    } else {
                        u_info("[goto] Can not predict the path");
                    }
                } else {
                    u_info("[goto] Make train move before goto");
                }
            }
            Reply(client_tid, (const char *) &(cmd_buffer[id]), sizeof(cmd_buffer[id]));
            break;
        case CT_CHANGE_TRACK:
            _track_type = cmd_buffer[id].content[0];
            Reply(client_tid, (const char *) &(cmd_buffer[id]), sizeof(cmd_buffer[id]));
            if (_track_type == TT_TRACK_A) {
                u_info("[track] Track a set");
                init_tracka(_track);
            }
            if (_track_type == TT_TRACK_B) {
                u_info("[track] Track a set");
                init_trackb(_track);
            }
            break;
        case CT_SET_TRAIN_NUMBER:
            train_index = cmd_buffer[id].content[0];
            u_info("[train] Set train number to %d", _trainset[train_index].number);
            Reply(client_tid, (const char *) &(cmd_buffer[id]), sizeof(cmd_buffer[id]));
            break;
        case CT_SET_VELOCITY_LEVEL:
            if (!train_index) {
                u_error("[level] Please set train number before level");
            } else {
                velocity_level = cmd_buffer[id].content[0];
                _trainset[train_index].curr_level = velocity_level;
                _trainset[train_index].curr_speed = _trainset[train_index].measurement[_track_type].speed[velocity_level];
                _trainset[train_index].curr_velocity = _trainset[train_index].measurement[_track_type].velocity[velocity_level];
                _trainset[train_index].curr_stopping_distance = _trainset[train_index].measurement[_track_type].stopping_distance[velocity_level];
                enqueue(&cmd_queue, id);
                cmd_buffer[id].id = id;
                cmd_buffer[id].type =  CT_TRAIN_NORMAL;
                cmd_buffer[id].len = 2;
                cmd_buffer[id].content[0] = _trainset[train_index].curr_speed;
                cmd_buffer[id].content[1] = _trainset[train_index].number;
                cmd_buffer[id].priority = 1;
                id += 1;
                id %= QUEUE_SIZE;
                u_info("[level] Set speed to %d velocity %d", _trainset[train_index].curr_speed, _trainset[train_index].curr_velocity);
            }
            Reply(client_tid, (const char *) &(cmd_buffer[id]), sizeof(cmd_buffer[id]));
            break;
        default:
            break;
        }
   }
}