#include <user.h>
#include <lib_periph_bwio.h>

static int _name_server_tid = -1;

int RegisterAs(const char *name) {
    if (_name_server_tid == -1) return -1;
    NameServerMessage ns_message;
    ns_message.operation = NS_REGISTERAS;
    ns_message.name = name;
    ns_message.tid = MyTid();
    int result = Send(_name_server_tid, (const char *) &ns_message, sizeof(ns_message), (char *)&ns_message, sizeof(ns_message));
    if (result < 0) return -1;
    return 0;
}

int WhoIs(const char *name) {
    if (_name_server_tid == -1) return -1;
    NameServerMessage ns_message;
    ns_message.operation = NS_WHOIS;
    ns_message.name = name;
    int result = Send(_name_server_tid, (const char *) &ns_message, sizeof(ns_message), (char *)&ns_message, sizeof(ns_message));
    if (result < 0) return -1;
    return ns_message.tid;
}

void name_server() {
    // log("Name Server: Initializing ...");
    unsigned int _hash_table[HASHSIZE][2];
    init_hash_table(_hash_table, HASHSIZE);
    _name_server_tid = MyTid();
    // log("Name server tid <%d>", _name_server_tid);
    int client_tid, result;
    NameServerMessage ns_message;
    HashEntry *entry;
    while (Receive(&client_tid, (char *) &ns_message, sizeof(ns_message))) {
        // debug("Name Server: get request of <%d> with key <%s> value <%d>", ns_message.operation,ns_message.name , ns_message.tid);
        switch (ns_message.operation)
        {
        case NS_REGISTERAS:
            put(_hash_table, HASHSIZE, ns_message.name, (unsigned int) ns_message.tid);
            break;
        case NS_WHOIS:
            entry = get(_hash_table, HASHSIZE, ns_message.name);
            ns_message.tid = (int) entry->value;
            break;
        default:
            break;
        }
        result = Reply(client_tid, (const char *) &ns_message, sizeof(ns_message));
        if (result > -1) {
            debug("Name Server: result = <%d>", result);
        } else if(result == -1) {
            debug("Name Server: tid is not the task id of an existing task.");
        } else if(result == -2) {
            debug("Name Server: send-receive-reply transaction could not be completed.");
        }
    }
}