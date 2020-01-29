#include <user.h>
#include <shared.h>

void init_hash_table(unsigned int *hash_table, int hash_size) {
    for (int i = 0; i < hash_size; i++) {
        hash_table[i] = NULL;
    }
}

unsigned _hash(char *s)
{
    unsigned hash_value;
    for (hash_value = 0; *s != '\0'; s++)
      hash_value = *s + 31 * hash_value;
    return hash_value % HASHSIZE;
}

HashEntry *get(unsigned int *hash_table, int hash_size, char *key) 
{
    HashEntry *entry;
    for (entry = hash_table[_hash(key)]; entry != NULL; entry = entry->next) {
        if (strcmp(key, entry->key) == 0)
          return entry;
    }
    return NULL;
}

HashEntry *set(unsigned int *hash_table, int hash_size, char *key, int value) 
{
    HashEntry *entry;
    unsigned hashval;
    if ((entry = get(hash_table, hash_size, key)) == NULL) {
        HashEntry *new_entry = (HashEntry *) Malloc(sizeof(HashEntry));
        new_entry->key = key;
        new_entry->value = value;
        new_entry->next = NULL;
        entry->next = new_entry;
    } else {
        entry->key = value;
    }
    return entry;
}