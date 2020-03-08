
#define TRAIN_MAX 6
#define SPEED_RANGE 3

typedef enum {
    TT_TRACK_A,
    TT_TRACK_B
} TRACK_TYPE;

typedef struct {
    int speed[SPEED_RANGE];
    int velocity[SPEED_RANGE];
    int stopping_distance[SPEED_RANGE];
} Measurement;

typedef struct {
    int number;
    int curr_level;
    int curr_speed;
    int curr_velocity;
    int curr_stopping_distance;
    unsigned int last_read_time;
    unsigned int time_elapsed;
    Measurement measurement[2];
} Train_Node;

int switch_number_to_node_number(int switch_number);
void init_trainset(Train_Node *trainset);