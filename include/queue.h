#ifndef LMCVITTI_Y247PAN_QUEUE
#define LMCVITTI_Y247PAN_QUEUE

#define QUEUE_SIZE 1024

typedef struct
{
    int queue[QUEUE_SIZE +1];
    int size;
} Queue;

// priority queue heap implementation functions
void pq_insert(Queue *queue_struct, int tid);
int pq_pop(Queue *queue_struct);
void pq_remove(Queue *queue_struct, int tid) ;

void dump_queue(Queue *queue_struct);

#endif