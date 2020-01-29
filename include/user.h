#ifndef LMCVITTI_Y247PAN_USER
#define LMCVITTI_Y247PAN_USER

/*
 * Include section
 */


/*
 * Macro definition
 */
#define HASHSIZE 101
#define STRING_MAX_LENGTH 256

/*
 * Enum definition
 */
typedef enum
{
    REGISTERAS = 0,
    WHOIS
} NS_OPERATION;

/*
 * Struct definition
 */
typedef struct 
{
	NS_OPERATION operation;
    const char *name;
    int tid;
} NSMessage;

typedef struct _HashEntry
{
	struct _HashEntry *prev;
	struct _HashEntry *next;
    const char *key;
    unsigned int value;
} HashEntry;

// task creation
int Create(int priority, void (*function)());
int MyTid();
int MyParentTid();
void Yield();
void Exit();

// message passing
int Send(int tid, const char *msg, int msglen, char *reply, int rplen);
int Receive(int *tid, char *msg, int msglen);
int Reply(int tid, const char *reply, int rplen);

void NameServer();
int RegisterAs(const char *name);
int WhoIs(const char *name);

char *Malloc(int size);
void Free(char *ptr);

void init_hash_table(unsigned int (*hash_table)[2], int hash_size);
HashEntry *get(unsigned int (*hash_table)[2], int hash_size, const char *key);
void *put(unsigned int (*hash_table)[2], int hash_size, const char *key, unsigned int value);
int remove(unsigned int (*hash_table)[2], int hash_size, char *key);

void function_wrapper(void (*function)());
void user_task_0();
void user_task_test();

#endif