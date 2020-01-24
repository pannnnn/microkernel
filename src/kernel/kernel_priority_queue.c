#include <kernel.h>
#include <lib_periph_bwio.h>

extern KernelState _kernel_state;

void percolate_up(int index) {
    int *queue = _kernel_state.queue;
    if (index > _kernel_state.queue_size) return;
    while (index/2 != 0) {
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

void percolate_down(int index) {
    int *queue = _kernel_state.queue;
    if (index >= _kernel_state.queue_size) return;
    while (index * 2 <= _kernel_state.queue_size) {
        int tid = queue[index];
        TaskDescriptor *td = get_td(tid);
        int min_child_index = min_child(index);
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

void pq_insert(int tid) {
    int *queue = _kernel_state.queue;
    int index = ++_kernel_state.queue_size;
    queue[index] = tid;
    percolate_up(index);
}

int pq_pop() {
    int *queue = _kernel_state.queue;
    int result = queue[1];
    queue[1] = queue[_kernel_state.queue_size--];
    percolate_down(1);
    return result;
}

void pq_remove(int tid)  {
    int *queue = _kernel_state.queue;
    for (int i = 1; i <= _kernel_state.queue_size; i++) {
        if (queue[i] == tid) {
            queue[i] = queue[_kernel_state.queue_size--];
            // bwprintf( COM2, "\n\rBefore <%d> <%d>\n\r", i, queue[i]);
            percolate_down(i);
            percolate_up(i);
            // bwprintf( COM2, "\n\rAfter <%d> <%d>\n\r", i, queue[i]);
            return;
        }
    }
}

int min_child(int index) {
    int left_child_index = index * 2;
    int right_child_index = index * 2 + 1;
    if (right_child_index > _kernel_state.queue_size) return left_child_index;

    int *queue = _kernel_state.queue;
    int left_child_tid = queue[left_child_index];
    TaskDescriptor *left_child_td = get_td(left_child_tid);
    int right_child_tid = queue[right_child_index];
    TaskDescriptor *right_child_td = get_td(right_child_tid);

    if (left_child_td->priority > right_child_td->priority || 
        (left_child_td->priority = right_child_td->priority && 
        left_child_td->scheduled_count < right_child_td->scheduled_count)) {
            return left_child_index;
    }

    return right_child_index;
}