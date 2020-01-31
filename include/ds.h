#ifndef LMCVITTI_Y247PAN_QUEUE
#define LMCVITTI_Y247PAN_QUEUE

#define QUEUE_SIZE 128

typedef struct
{
    int queue[QUEUE_SIZE + 1];
    int size;
    int index;
} Queue;

// priority queue heap implementation functions
void pq_insert(Queue *queue_struct, int tid);
int pq_pop(Queue *queue_struct);
void pq_remove(Queue *queue_struct, int tid) ;

void dump_queue(Queue *queue_struct);

void enqueue(Queue *queue_struct, int value);
int deque(Queue *queue_struct);

// unordered list implementation functions
int ul_add(Queue *queue_struct, int tid);
int ul_remove(Queue *queue_struct, int tid);
int ul_pop(Queue *queue_struct);

#endif