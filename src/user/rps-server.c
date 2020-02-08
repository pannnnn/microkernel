#include <rps.h>
#include <user.h>
#include <shared.h>
#include <stdio.h>
#include <lib_ts7200.h>

void _init_rps_server() {
	RegisterAs("rps_server");
}

void _reply_paired(int id_1, int tid_1, int id_2, int tid_2) {
	debug("<%d> partners with <%d>\n\r", tid_1, tid_2);
	char reply[2];
	reply[0] = 'm';
	reply[1] = (char) id_1;
	Reply(tid_1, reply, 2);
	reply[1] = (char) id_2;
	Reply(tid_2, reply, 2);
}

void _reply_results(int winner_tid, int loser_tid, int is_tie) {
	if (is_tie == 1) {
		debug("<%d> tied with <%d>\n\r", winner_tid, loser_tid);
	} else {
		debug("<%d> won and <%d> lost this round\n\r", winner_tid, loser_tid);
	}
	char reply[1];
	reply[0] = (is_tie == 1) ? 't' : 'w';
	Reply(winner_tid, reply, 1);
	reply[0] = (is_tie == 1) ? 't' : 'l';
	Reply(loser_tid, reply, 1);
	bwgetc(COM2);
}

void rps_server_main() {
	_init_rps_server();
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
		int sender_id;
		char msg[MESSAGE_SIZE];
		Receive(&sender_id, msg, MESSAGE_SIZE);
		int sender, partner;

		switch (msg[0]) {
			case 'a':
				sender = num_registered++;
				player_tids[sender] = sender_id;
				if (unpaired == -1) {
					unpaired = sender;
					break;
				}
				partner = unpaired;
				unpaired = -1;
				pairings[partner] = sender;
				pairings[sender] = partner;
				_reply_paired(sender, sender_id, partner, player_tids[partner]);
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
				if (partner >= 0) pairings[partner] = -1;
				char reply = 'q';
				num_players--;
				Reply(sender_id, &reply, 1);
				break;
		}
	}	
}