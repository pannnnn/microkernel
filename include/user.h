#ifndef LMCVITTI_Y247PAN_USER
#define LMCVITTI_Y247PAN_USER

/*
 * Include section
 */

/*
 * Macro definition
 */
#define HASHSIZE 101

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

typedef struct _NSHashEntry
{
	struct _NSHashEntry *next;
    char *name;
    int tid;
} NSHashEntry;

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

int NameServerTid();
int RegisterAs(const char *name);
int WhoIs(const char *name);

unsigned hash(char *s);
NSHashEntry *get(char *name);
NSHashEntry *set(char *name, int tid);

void function_wrapper(void (*function)());
void user_task_0();
void user_task_test();

#endif