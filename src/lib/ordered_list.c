#include <queue.h>

// NOTE: for the ordered list implementation, the size element is used as a write 
// index and "index" as a read index. size does not represent the number of elements
// currently in the list

void ol_add(Queue *queue_struct, int value) {
	// circular buffered queue wrap-around
	if (queue_struct->size >= QUEUE_SIZE) queue_struct->size = 0;
	queue_struct->queue[queue_struct->size++] = value;
}

int ol_remove(Queue *queue_struct) {
	// list is empty
	if (queue_struct->size == queue_struct->index) return -1;

	int result = queue_struct->queue[queue_struct->index++];
	// circular buffered queue wrap-around
	if (queue_struct->index >= QUEUE_SIZE) queue_struct->index = 0;

	return result;
}