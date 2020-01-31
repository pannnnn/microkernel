#include <kernel.h>
#include <ds.h>
#include <shared.h>
#include <lib_periph_bwio.h>
#include <lib_ts7200.h>

// declared as global variable in main.c
extern KernelState _kernel_state;

void sys_send(int tid, char *msg, int msglen, char *reply, int rplen) {
	if (tid >= KERNEL_STACK_TD_LIMIT || tid <= 0 || _kernel_state.td_user_stack_availability[tid] == 0) {
		// bwprintf(COM2, "\n\rattempting to send to tid that doesn't exist\n\r");
		set_result(get_td(_kernel_state.scheduled_tid), 0xFFFFFFFF);
	}
	if (tid == _kernel_state.scheduled_tid) {
		// bwprintf(COM2, "\n\rattempting to send to self\n\r");
		set_result(get_td(_kernel_state.scheduled_tid), 0xFFFFFFFE);
	};

	int sender_tid = _kernel_state.scheduled_tid;
	TaskDescriptor *sender_td = get_td(sender_tid);
	sender_td->message.replied_message = reply;
	sender_td->message.replied_message_length = rplen;

	TaskDescriptor *receiver_td = get_td(tid);
	if (receiver_td->state == RECEIVE_WAIT) {
		receiver_td->state = READY;
		sender_td->state = REPLY_WAIT;
		*receiver_td->message.receiver_reserved_sid = sender_tid;
		int copied_length = MIN(msglen, receiver_td->message.receive_message_length);
		charstr_copy(msg, receiver_td->message.receive_message, copied_length);
		pq_insert(&_kernel_state.ready_queue, receiver_td->id);
		set_result(receiver_td, copied_length);
	} else {
		sender_td->message.sent_message = msg;
		sender_td->message.sent_message_length = msglen;
		sender_td->state = SEND_WAIT;
		enqueue(&receiver_td->inbox, sender_tid);
	}
}

void sys_receive(int *tid, char *msg, int msglen) {
	TaskDescriptor *receiver_td = get_td(_kernel_state.scheduled_tid);
	int sender_tid = deque(&receiver_td->inbox);

	if (sender_tid == -1) {
		receiver_td->state = RECEIVE_WAIT;
		receiver_td->message.receiver_reserved_sid = tid;
		receiver_td->message.receive_message = msg;
		receiver_td->message.receive_message_length = msglen;
	} else {
		*tid = sender_tid;
		TaskDescriptor *sender_td = get_td(sender_tid);
		if (sender_td->state == SEND_WAIT) {
			sender_td->state = REPLY_WAIT;
			int copied_length = MIN(sender_td->message.sent_message_length, msglen);
			charstr_copy(sender_td->message.sent_message, msg, copied_length);
			pq_insert(&_kernel_state.ready_queue, receiver_td->id);
			set_result(receiver_td, copied_length);
		}
	}
}

int sys_reply(int tid, char *reply, int rplen) {
	if (tid >= KERNEL_STACK_TD_LIMIT || tid <= 0 || _kernel_state.td_user_stack_availability[tid] == 0) {
		return -1;
	}
	TaskDescriptor *sender_td = get_td(tid);
	if (sender_td->state != REPLY_WAIT) {
		return -2;
	}
	sender_td->state = READY;
	int copied_length = MIN(sender_td->message.replied_message_length, rplen);
	charstr_copy(reply, sender_td->message.replied_message, copied_length);
	pq_insert(&_kernel_state.ready_queue, sender_td->id);
	set_result(sender_td, copied_length);
	return copied_length;
}