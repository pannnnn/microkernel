#include <ds.h>

int ul_add(Queue *queue_struct, int tid) {
	// no space in queue
	if (queue_struct->size >= QUEUE_SIZE) return 0;
	queue_struct->queue[queue_struct->size++] = tid;
	return 1;
}

int ul_remove(Queue *queue_struct, int tid) {
	int *queue = queue_struct->queue;
	for (int i=0; i<queue_struct->size; i++) {
		if (queue[i] == tid) {
			queue[i] = queue[--queue_struct->size];
			return 1;
		}
	}
	// could not find given tid
	return 0;
}