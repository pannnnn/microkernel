#include <ds.h>
#include <shared.h>

void enqueue(Queue *queue_struct, int value) {
	// not check overwriting since range of task guarantees that
	queue_struct->queue[queue_struct->size++] = value;
	// circular buffered queue wrap-around
	if (queue_struct->size >= QUEUE_SIZE) queue_struct->size = 0;
}

int deque(Queue *queue_struct) {
	if (queue_struct->index == queue_struct->size) return -1;

	int result = queue_struct->queue[queue_struct->index++];
	// circular buffered queue wrap-around
	if (queue_struct->index >= QUEUE_SIZE) queue_struct->index = 0;

	return result;
}