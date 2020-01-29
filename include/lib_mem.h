#include <kernel.h>

/*
 * Struct definition
 */
typedef struct _BlockMeta {
    struct _BlockMeta *prev;
    struct _BlockMeta *next;
    int id;
    HEAP_TYPE heap_type;
} BlockMeta;

typedef struct {
    int heap_block_size;
    int heap_block_count;
    char *heap_block_used;
    char *heap_block_unused;
    unsigned int heap_region_addr;
} HeapInfo;

/*
 * Function definition
 */
void mem_init_task_descriptors();
void mem_init_heap_region(HEAP_TYPE heap_type);
void mem_free(char *ptr);
char *mem_malloc(int size);