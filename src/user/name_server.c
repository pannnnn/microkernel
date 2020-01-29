#include <user.h>

static int _name_server_tid = -1;
static unsigned int _hash_table[HASHSIZE];

int RegisterAs(const char *name) {
    if (_name_server_tid == -1) return -1;
    NSMessage ns_message;
    ns_message.operation = REGISTERAS;
    ns_message.name = name;
    ns_message.tid = MyTid();
    int result = Send(_name_server_tid, (const char *) &ns_message, sizeof(ns_message), (char *)&ns_message, sizeof(ns_message));
    if (result < 0) return -2;
    return 0;
}

int WhoIs(const char *name) {
    if (_name_server_tid == -1) return -1;
    NSMessage ns_message;
    ns_message.operation = WHOIS;
    ns_message.name = name;
    int result = Send(_name_server_tid, (char *) &ns_message, sizeof(ns_message), (char *)&ns_message, sizeof(ns_message));
    if (result < 0) return -2;
    return ns_message.tid;
}

void NameServer() {
    init_hash_table(_hash_table, HASHSIZE);
    _name_server_tid = MyTid();
    int client_tid, result;
    NSMessage ns_message;
    while (Receive(&client_tid, (char *) &ns_message, sizeof(ns_message))) {
        switch (ns_message.operation)
        {
        case REGISTERAS:
            /* code */
            break;
        case WHOIS:
            /* code */
            break;
        default:
            break;
        }
        result = Reply(client_tid, (const char *) &ns_message, sizeof(ns_message));
    }
}