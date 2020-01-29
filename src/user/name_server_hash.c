#include <user.h>
#include <shared.h>


unsigned hash(char *s)
{
    unsigned hash_value;
    for (hash_value = 0; *s != '\0'; s++)
      hash_value = *s + 31 * hash_value;
    return hash_value % HASHSIZE;
}

NSHashEntry *get(char *name) {
    NSHashEntry *entry;
    for (entry = hash_table[hash(name)]; entry != NULL; entry = entry->next) {
        if (strcmp(name, entry->name) == 0)
          return entry;
    }
    return NULL;
}

NSHashEntry *set(char *name, int tid) {
    // TODO: add newly create entry to bucket
    NSHashEntry *entry;
    unsigned hashval;
    if ((entry = get(name)) == NULL) {
        entry = (NSHashEntry *) malloc(sizeof(NSHashEntry));
    } else {
        entry->name = name;
    }
}