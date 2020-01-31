#include <rps.h>
#include <user.h>
#include <shared.h>
#include <lib_periph_bwio.h>
#include <lib_ts7200.h>

void init_rps_server() {
	//RegisterAs("rps_server");
}

void _reply_paired(int id_1, int tid_1, int id_2, int tid_2) {
	bwprintf(COM2, "<%d> partners with <%d>\n\r", tid_1, tid_2);
	char reply[2];
	reply[0] = 'm';
	reply[1] = (char) id_1;
	Reply(tid_1, reply, 2);
	reply[1] = (char) id_2;
	Reply(tid_2, reply, 2);
}

void _reply_results(int winner_tid, int loser_tid, int is_tie) {
	if (is_tie == 1) bwprintf(COM2, "<%d> tied with <%d>", winner_tid, loser_tid);
	else bwprintf(COM2, "<%d> won and <%d> lost this round", winner_tid, loser_tid);
	char reply = (is_tie == 1) ? 't' : 'w';
	Reply(winner_tid, &reply, 1);
	reply = (is_tie == 1) ? 't' : 'l';
	Reply(loser_tid, &reply, 1);
	bwgetc(COM2);
}

void rps_server_main() {
	init_rps_server();
	int num_registered = 0;
	int num_players = MAX_NUM_PLAYERS;

	int player_tids[MAX_NUM_PLAYERS];
	int pairings[MAX_NUM_PLAYERS];
	char moves[MAX_NUM_PLAYERS];

	for (int i=0; i<MAX_NUM_PLAYERS; i++) {
		moves[i] = '!';
	}

	int unpaired = -1;

	while(num_players > 0) {
		int *sender_id;
		char msg[MESSAGE_SIZE];
		Receive(sender_id, msg, MESSAGE_SIZE);
		int sender, partner;

		switch (msg[0]) {
			case 'a':
				player_tids[num_registered++] = *sender_id;
				sender = num_registered-1;
				if (unpaired == -1) {
					unpaired = sender;
					break;
				}
				pairings[unpaired] = sender;
				pairings[sender] = unpaired;
				unpaired = -1;
				_reply_paired(sender, *sender_id, unpaired, player_tids[unpaired]);
				break;
			case 'r':
				sender = (int) msg[1];
				partner = pairings[sender];
				switch(moves[partner]) {
					case 'r':
						// rock rock = tie
						_reply_results(player_tids[sender], player_tids[partner], 1);
						break;
					case 'p':
						// paper beats rock
						_reply_results(player_tids[partner], player_tids[sender], 0);
						break;
					case 's':
						// rock beats scissors
						_reply_results(player_tids[sender], player_tids[partner], 0);
						break;
					default:
						// partner hasn't moved yet
						moves[sender] = 'r';
						break;
				}
				moves[partner] = '!';
				break;
			case 'p':
				sender = (int) msg[1];
				partner = pairings[sender];
				switch(moves[partner]) {
					case 'r':
						// paper beats rock
						_reply_results(player_tids[sender], player_tids[partner], 0);
						break;
					case 'p':
						// paper paper tie
						_reply_results(player_tids[sender], player_tids[partner], 1);
						break;
					case 's':
						// scissors beats paper
						_reply_results(player_tids[partner], player_tids[sender], 0);
						break;
					default:
						// partner hasn't moved yet
						moves[sender] = 'p';
						break;
				}
				moves[partner] = '!';
				break;
			case 's':
				sender = (int) msg[1];
				partner = pairings[sender];
				switch(moves[partner]) {
					case 'r':
						// rock beats scissors
						_reply_results(player_tids[partner], player_tids[sender], 0);
						break;
					case 'p':
						// scissors beats paper
						_reply_results(player_tids[sender], player_tids[partner], 0);
						break;
					case 's':
						// scissors scissors tie
						_reply_results(player_tids[sender], player_tids[partner], 1);
						break;
					default:
						// partner hasn't moved yet
						moves[sender] = 's';
						break;
				}
				moves[partner] = '!';
				break;
			case 'q':
				sender = (int) msg[1];
				partner = pairings[sender];
				pairings[partner] = -1;
				char reply = 'q';
				Reply(*sender_id, &reply, 1);
				break;
		}
	}	
}