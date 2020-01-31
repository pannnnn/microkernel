#include <kernel.h>
#include <ds.h>
#include <lib_periph_bwio.h>


// bubble up: sorting the heap after removing a node
void percolate_up(Queue *queue_struct, int index) {
    int *queue = queue_struct->queue;
    if (index > queue_struct->size) return;
    while (index/2 > 0) {
        int tid = queue[index];
        TaskDescriptor *td = get_td(tid);
        int parent_tid = queue[index/2];
        TaskDescriptor *parent_td = get_td(parent_tid);
        if (td->priority > parent_td->priority || 
            (td->priority == parent_td->priority && td->scheduled_count < parent_td->scheduled_count)) {
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
    TaskDescriptor *left_child_td = get_td(left_child_tid);
    int right_child_tid = queue[right_child_index];
    TaskDescriptor *right_child_td = get_td(right_child_tid);

    if (left_child_td->priority > right_child_td->priority || 
        (left_child_td->priority == right_child_td->priority && 
        left_child_td->scheduled_count < right_child_td->scheduled_count)) {
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
        TaskDescriptor *td = get_td(tid);
        int min_child_index = min_child(queue_struct, index);
        int min_child_tid = queue[min_child_index];
        TaskDescriptor *min_child_td = get_td(min_child_tid);
        if (td->priority < min_child_td->priority || 
            (td->priority == min_child_td->priority && td->scheduled_count > min_child_td->scheduled_count)) {
            queue[min_child_index] = tid;
            queue[index] = min_child_tid;
        }
        index = min_child_index;
    }
}

// add a node into the priority queue
void pq_insert(Queue *queue_struct, int tid) {
    int *queue = queue_struct->queue;
    int index = ++queue_struct->size;
    queue[index] = tid;
    percolate_up(queue_struct, index);
}

// return the top (root) node from the priority queue
int pq_pop(Queue *queue_struct) {
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

// flush the priority queue; used for debugging
void dump_queue(Queue *queue_struct) {
    bwprintf( COM2, "Queue:");
    for (int i = 1; i <= queue_struct->size; i++) {
        bwprintf( COM2, " %d", queue_struct->queue[i]);
    }
    bwprintf( COM2, "\n\r");
}