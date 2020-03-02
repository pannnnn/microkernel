#include <kernel.h>
#include <ds.h>
#include <lib_periph_bwio.h>

// bubble up: sorting the heap after removing a node
void percolate_up(Queue *queue_struct, int index) {
    int *queue = queue_struct->queue;
    if (index > queue_struct->size) return;
    while (index/2 > 0) {
        int tid = queue[index];
        int parent_tid = queue[index/2];

        int child_arg1 = queue_struct->get_arg1(tid);
        int parent_arg1 = queue_struct->get_arg1(parent_tid);
        int child_arg2 = queue_struct->get_arg2(tid);
        int parent_arg2 = queue_struct->get_arg2(parent_tid);

        if (child_arg1 < parent_arg1 || (child_arg1 == parent_arg1 && child_arg2 < parent_arg2)) {
            queue[index/2] = tid;
            queue[index] = parent_tid;
        }
        index /= 2;
    }
}

// find the minimum child node of the given node
// helper function for percolate_down
int min_child(Queue *queue_struct, int index) {
    int left_child_index = index * 2;
    int right_child_index = index * 2 + 1;
    if (right_child_index > queue_struct->size) return left_child_index;

    int *queue = queue_struct->queue;
    int left_child_tid = queue[left_child_index];
    int right_child_tid = queue[right_child_index];

    int left_child_arg1 = queue_struct->get_arg1(left_child_tid);
    int right_child_arg1 = queue_struct->get_arg1(right_child_tid);
    int left_child_arg2 = queue_struct->get_arg2(left_child_tid);
    int right_child_arg2 = queue_struct->get_arg2(right_child_tid);

    if (left_child_arg1 < right_child_arg1 || 
        (left_child_arg1 == right_child_arg1 && left_child_arg2 < right_child_arg2)) {
            return left_child_index;
    }

    return right_child_index;
}

// bubble down: sorting the heap after adding a node
void percolate_down(Queue *queue_struct, int index) {
    int *queue = queue_struct->queue;
    if (index >= queue_struct->size) return;
    while (index * 2 <= queue_struct->size) {
        int tid = queue[index];
        int min_child_index = min_child(queue_struct, index);
        int min_child_tid = queue[min_child_index];

        int parent_arg1 = queue_struct->get_arg1(tid);
        int min_child_arg1 = queue_struct->get_arg1(min_child_tid);
        int parent_arg2 = queue_struct->get_arg2(tid);
        int min_child_arg2 = queue_struct->get_arg2(min_child_tid);
        
        if (parent_arg1 > min_child_arg1 || 
            (parent_arg1 == min_child_arg1 && parent_arg2 > min_child_arg2)) {
            queue[min_child_index] = tid;
            queue[index] = min_child_tid;
        }
        index = min_child_index;
    }
}

// add a node into the priority queue
void pq_insert(Queue *queue_struct, int tid) {
    int *queue = queue_struct->queue;
    // to prevent duplicate tids
    // TODO: check ready status instead of for loop
    for (int i = 1; i < queue_struct->size; i++) {
        if (queue[i] == tid) return;
    }
    int index = ++queue_struct->size;
    queue[index] = tid;
    percolate_up(queue_struct, index);
}

// return the top (root) node from the priority queue
int pq_pop(Queue *queue_struct) {
    if (queue_struct->size == 0) return -1;
    int *queue = queue_struct->queue;
    int result = queue[1];
    queue[1] = queue[queue_struct->size--];
    percolate_down(queue_struct, 1);
    return result;
}

// remove a node from the priority queue
void pq_remove(Queue *queue_struct, int tid)  {
    int *queue = queue_struct->queue;
    for (int i = 1; i <= queue_struct->size; i++) {
        if (queue[i] == tid) {
            queue[i] = queue[queue_struct->size--];
            percolate_down(queue_struct, i);
            percolate_up(queue_struct, i);
            return;
        }
    }
}

int pq_get_min(Queue *queue_struct) {
    if (queue_struct->size == 0) return -1;
    return (queue_struct->queue)[1];
}

// flush the priority queue; used for debugging
void dump_queue(Queue *queue_struct) {
    bwprintf( COM2, "Queue:");
    for (int i = 1; i <= queue_struct->size; i++) {
        bwprintf( COM2, " %d", queue_struct->queue[i]);
    }
    bwprintf( COM2, "\r\n");
}