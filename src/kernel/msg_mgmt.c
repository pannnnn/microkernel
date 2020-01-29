#include <kernel.h>
#include <queue.h>
#include <shared.h>

// declared as global variable in main.c
extern KernelState _kernel_state;

void _unblock_receiver(TaskDescriptor *receiver_td, TaskDescriptor *sender_td) {
	int message_length = (receiver_td->msglen < sender_td->msglen) ? receiver_td->msglen : sender_td->msglen;
	// copy message to receiver buffer
	charstr_copy(sender_td->msg, receiver_td->msg, message_length);
	// copy sender id to receiver	
	(*receiver_td->sender_id_ptr) = sender_td->id;

	// remove receiver from the receive list and add to ready queue
	ul_remove(&_kernel_state.receive_queue, receiver_td->id);
	pq_insert(&_kernel_state.ready_queue, receiver_td->id);

	// insert return value for the receiver task onto receiver stack
	// becomes the return value of Receive(...)
	task_return(receiver_td, sender_td->msglen);
}

int sys_send(int tid, int *msg, int msglen, int *reply, int rplen) {
	TaskDescriptor *receiver_td = get_td(tid);	
	int sender_id = _kernel_state.scheduled_tid;
	TaskDescriptor *sender_td = get_td(sender_id);

	if (tid > KERNEL_STACK_TD_CAP || receiver_td->state == EXITED) {
		// the task associated with the id doesn't exist or has stopped
		// becomes the return value of Send(...)
		task_return(sender_td, -1);
		// put the task back on the ready queue
		pq_insert(&_kernel_state.ready_queue, sender_id);
		return -1;
	}
	// TODO: determine in what situations -2 (the transaction couldn't be completed)
	// needs to be returned

	// Store send information in the sender's task descriptor
	sender_td->msg = msg;
	sender_td->msglen = msglen;
	sender_td->rpl = reply;
	sender_td->rpllen = rplen;

	if (ul_remove(&_kernel_state.receive_queue, tid) == 0) {
		// the receiver isn't ready & waiting yet; block on send list
		ul_add(&_kernel_state.send_queue, sender_id);
		// Register the send request in receiver's send list
		ol_add(&receiver_td->sending, sender_id);
		return 0;
	}

	// the receiver is on the receive list waiting for a message

	// add the sender to the reply list to wait for the reply
	ul_add(&_kernel_state.reply_queue, sender_id);

	// unblock and prep receiver
	_unblock_receiver(receiver_td, sender_td);
	return 1;
}

int sys_receive(int *tid, int *msg, int msglen) {
	TaskDescriptor *receiver_td = get_td(_kernel_state.scheduled_tid);
	int sender_id = ol_remove(&receiver_td->sending);
	
	// Store message info in receiver's task descriptor
	receiver_td->msg = msg;
	receiver_td->msglen = msglen;
	receiver_td->sender_id_ptr = tid;

	if (sender_id >= 0) {
		// no senders are waiting for this task to receive
		// add task to receive list to wait for sender
		ul_add(&_kernel_state.receive_queue, _kernel_state.scheduled_tid);
		return 0;
	}

	// found a sender waiting (and removed it from the sender's sending queue)

	// remove the sender from the (kernel's) send queue
	ul_remove(&_kernel_state.send_queue, sender_id);
	// add the sender to the reply queue to wait for the reply
	ul_add(&_kernel_state.reply_queue, sender_id);

	// unblock and prep receiver
	_unblock_receiver(receiver_td, get_td(sender_id));
	return 1;
}

int sys_reply(int tid, int *reply, int rplen) {
	TaskDescriptor *sender_td = get_td(tid);

	// TODO: this won't work if we reclaim td memory space eventually
	if (tid > KERNEL_STACK_TD_CAP || sender_td->state == EXITED) {
		// the task associated with the id doesn't exist or has stopped
		// this becomes the return value of Reply(...)
		return -1;
	}
	// remove tid (sender id) from reply list
	if (ul_remove(&_kernel_state.reply_queue, tid) == 0) {
		// could not find tid on reply list
		// this becomes the return value of Reply(...)
		return -2;
	}

	// copy reply into sender reply buffer
	int message_length = (rplen < sender_td->rpllen) ? rplen : sender_td->rpllen;
	charstr_copy(reply, sender_td->rpl, message_length);

	// unblock sender task by adding to ready queue
	pq_insert(&_kernel_state.ready_queue, tid);

	// insert return value for the sender onto sender stack
	// becomes the return value of Send(...)
	task_return(sender_td, rplen);

	// becoems the return value of Reply(...)
	return sender_td->rpllen;
}