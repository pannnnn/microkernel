#ifndef LMCVITTI_Y247PAN_QUEUE
#define LMCVITTI_Y247PAN_QUEUE

#define QUEUE_SIZE 128

typedef struct
{
    int queue[QUEUE_SIZE + 1];
    int size;
    int index;
    int (*get_arg1)(int param);
    int (*get_arg2)(int param);
} Queue;


// priority queue heap implementation functions
void pq_insert(Queue *queue_struct, int tid);
int pq_pop(Queue *queue_struct);
void pq_remove(Queue *queue_struct, int tid) ;

// fifo queue implementation functions
void enqueue(Queue *queue_struct, int value);
int deque(Queue *queue_struct);

// debug queue
void dump_queue(Queue *queue_struct);
#endif