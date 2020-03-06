#define TRAIN_MAX 5

#define SPEED_RANGE 3

typedef struct {
    int speed_range[SPEED_RANGE];
} Measurement;

typedef struct {
    int number;
    int speed;
    int velocity;
    Measurement measurement;
} Train_Node;

void init_trainset(Train_Node *trainset);