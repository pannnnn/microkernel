#include <user.h>
#include <lib_periph_bwio.h>

static int _name_server_tid = -1;

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
    int result = Send(_name_server_tid, (const char *) &ns_message, sizeof(ns_message), (char *)&ns_message, sizeof(ns_message));
    if (result < 0) return -2;
    return ns_message.tid;
}

void NameServer() {
    bwprintf( COM2, "\n\rName Server: Initializing ...\n\r");
    unsigned int _hash_table[HASHSIZE][2];
    init_hash_table(_hash_table, HASHSIZE);
    _name_server_tid = MyTid();
    int client_tid, result;
    NSMessage ns_message;
    HashEntry *entry;
    while (Receive(&client_tid, (char *) &ns_message, sizeof(ns_message))) {
        bwprintf( COM2, "\n\rName Server: Receive Message %s\n\r", ns_message.name);
        switch (ns_message.operation)
        {
        case REGISTERAS:
            put(_hash_table, HASHSIZE, ns_message.name, (unsigned int) ns_message.tid);
            break;
        case WHOIS:
            entry = get(_hash_table, HASHSIZE, ns_message.name);
            ns_message.tid = (int) entry->value;
            break;
        default:
            break;
        }
        result = Reply(client_tid, (const char *) &ns_message, sizeof(ns_message));
        if (result > -1) {
            bwprintf( COM2, "\n\rName Server: result = <%d>\n\r", result);
        } else if(result == -1) {
            bwprintf( COM2, "\n\rName Server: tid is not the task id of an existing task.\n\r");
        } else if(result == -2) {
            bwprintf( COM2, "\n\rName Server: send-receive-reply transaction could not be completed.\n\r");
        }
    }
}