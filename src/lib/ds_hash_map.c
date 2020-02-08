#include <user.h>
#include <shared.h>
#include <stdio.h>

void init_hash_table(unsigned int (*hash_table)[2], int hash_size) {
    for (int i = 0; i < hash_size; i++) {
        hash_table[i][0] = NULL;
        hash_table[i][1] = NULL;
    }
}

int _strcmp(const char *str1, const char *str2) 
{
    for (int i = 0; ; i++) {
        if (str1[i] == '\0' && str2[i] == '\0') {
            return 0;
        }
        if (str1[i] == str2[i]) {
            continue;
        } else {
            return -1;
        }
    }
    return -1;
}

unsigned int _hash(const char *s)
{
    unsigned int hash_value;
    for (hash_value = 0; *s != '\0'; s++)
      hash_value = *s + 31 * hash_value;
    return hash_value % HASHSIZE;
}

HashEntry *get(unsigned int (*hash_table)[2], int hash_size, const char *key) 
{
    HashEntry *entry;
    for (entry = (HashEntry *) hash_table[_hash(key)][0]; entry != NULL; entry = entry->next) {
        if (_strcmp(key, entry->key) == 0) {
          return entry;
        }
    }
    return NULL;
}

void put(unsigned int (*hash_table)[2], int hash_size, const char *key, unsigned int value) 
{
    HashEntry *entry;
    unsigned int hash = _hash(key);
    if ((entry = get(hash_table, hash_size, key)) == NULL) {
        HashEntry *new_entry = (HashEntry *) Malloc(sizeof(HashEntry));
        new_entry->key = key;
        new_entry->value = value;

        if (hash_table[hash][0] == NULL) {
            hash_table[hash][0] = (unsigned int) new_entry;
            new_entry->prev = NULL;
            new_entry->next = NULL;
        } else {
            HashEntry *tail = (HashEntry *) hash_table[hash][1];
            new_entry->prev = tail;
            tail->next = new_entry;
        }
        hash_table[hash][1] = (unsigned int) new_entry;
    } else {
        entry->value = value;
    }
}

int remove(unsigned int (*hash_table)[2], int hash_size, char *key) {
    HashEntry *entry;
    unsigned int hash = _hash(key);
    if ((entry = get(hash_table, hash_size, key)) == NULL) {
        return -1;
    } else {
        if (entry->prev == NULL && entry->next == NULL) {
            hash_table[hash][0] = NULL;
            hash_table[hash][1] = NULL;
        } else if (entry->prev == NULL) {
            entry->next->prev = NULL;
            hash_table[hash][0] = (unsigned int) entry->next;
        } else if (entry->next == NULL) {
            entry->prev->next = NULL;
            hash_table[hash][1] = (unsigned int) entry->prev;
        } else {
            entry->prev->next = entry->next;
            entry->next->prev = entry->prev;
        }
        Free((char *)entry);
        return 0;
    }
}

void dump_hash_map(unsigned int (*hash_table)[2]) {
    HashEntry *entry;
    bwprintf( COM2, "\n\rHash map");
    for (int i = 0; i < HASHSIZE; i++) {
        for (entry = (HashEntry *) hash_table[i][0]; entry != NULL; entry = entry->next) {
            bwprintf( COM2, "\n\rkey [%s] value [%d]", entry->key, entry->value);
        }
    }
    bwprintf( COM2, "\n\r");
}