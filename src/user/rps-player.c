#include <rps.h>
#include <lib_periph_timer.h>
#include <user.h>
#include <shared.h>
#include <lib_periph_bwio.h>
#include <lib_ts7200.h>

int _rps_signup(int rps_server_id) {
	char msg[] = "a";
	char reply[MESSAGE_SIZE];
	int result = Send(rps_server_id, (const char*) &msg, 1, reply, MESSAGE_SIZE);
	if (result < 0) {
		return 0;
	}
	if (result > 6) bwprintf(COM2, "unexpected message length from server: %s\n\r", reply);
	return reply[1];
}

char _rps_play(int rps_server_id, char my_id) {
	int rand = getTimerVal(TIM3) % 3; // generate a "random" number from 0 to 3
	if (rand < 0) rand = rand*-1;
	char msg[2];
	msg[1] = my_id;
	switch (rand) {
		case 0:
			msg[0] = 'r';
			break;
		case 1:
			msg[0] = 'p';
			break;
		case 2:
			msg[0] = 's';
			break;
	}
	int my_tid = MyTid();
	bwprintf(COM2, "<%d> plays <%c>\n\r", my_tid, msg[0]);

	char reply[MESSAGE_SIZE];
	Send(rps_server_id, msg, 2, reply, MESSAGE_SIZE);	
	return reply[0];
}

void _rps_quit(int rps_server_id, char my_id) {
	char msg[2];
	msg[0] = 'q';
	msg[1] = my_id;
	char reply[MESSAGE_SIZE];
	Send(rps_server_id, (const char*) msg, 2, reply, MESSAGE_SIZE);
	Exit();
}

void rps_player_main() {
	int win = 0, tie = 0, loss = 0;

	int rps_server_id = WhoIs("rps_server");
	// int rps_server_id = 2;
	
	char my_id = _rps_signup(rps_server_id); // BLOCKING

	for (int i=0; i<MATCHUPS_PER_PLAYER; i++) {
		char result = _rps_play(rps_server_id, my_id); // BLOCKING
		
		switch (result) {
			case 'w':
				win++;
				break;
			case 't':
				tie++;
				break;
			case 'l':
				loss++;
				break;
			default:
				// something went wrong
				// try again
				i--;
				break;
		}
	}
	// print something?
	bwprintf(COM2, "game record w: %d t: %d l: %d", win, tie, loss);
	_rps_quit(rps_server_id, my_id); // DESTROYS TASK
}