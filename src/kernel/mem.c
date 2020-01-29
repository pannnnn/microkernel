#include <kernel.h>
#include <user.h>
#include <shared.h>
#include <lib_mem.h>

// declared as global variable in main.c
extern KernelState _kernel_state;

void _get_heap_info(HeapInfo *heap_info, HEAP_TYPE heap_type) {
    switch (heap_type) {
        case SMALL:
            heap_info->heap_block_size = S_HEAP_BLOCK_SIZE;
            heap_info->heap_block_count = S_HEAP_BLOCK_COUNT;
            heap_info->heap_block_used = _kernel_state.s_block_used;
            heap_info->heap_block_unused = _kernel_state.s_block_unused;
            heap_info->heap_region_addr = S_HEAP_REGION;
            break;
        case MEDIUM:
            heap_info->heap_block_size = M_HEAP_BLOCK_SIZE;
            heap_info->heap_block_count = M_HEAP_BLOCK_COUNT;
            heap_info->heap_block_used = _kernel_state.m_block_used;
            heap_info->heap_block_unused = _kernel_state.m_block_unused;
            heap_info->heap_region_addr = M_HEAP_REGION;
            break;
        case LARGE:
            heap_info->heap_block_size = L_HEAP_BLOCK_SIZE;
            heap_info->heap_block_count = L_HEAP_BLOCK_COUNT;
            heap_info->heap_block_used = _kernel_state.l_block_used;
            heap_info->heap_block_unused = _kernel_state.l_block_unused;
            heap_info->heap_region_addr = L_HEAP_REGION;
            break;
        default:
            break;
    }
}

char *_mem_get_block(HEAP_TYPE heap_type) 
{
    HeapInfo hi;
    _get_heap_info(&hi, heap_type);

    if (hi.heap_block_unused == NULL) return NULL;

    char *block = (char *) hi.heap_block_unused;
    BlockMeta *block_meta = (BlockMeta *) block;
    
    BlockMeta *prev_block_meta = (BlockMeta *) block_meta->prev;
    BlockMeta *next_block_meta = (BlockMeta *) block_meta->next;
    if (prev_block_meta == next_block_meta) {
        hi.heap_block_unused = NULL;
    } else {
        next_block_meta->prev = prev_block_meta;
        prev_block_meta->next = next_block_meta;
        hi.heap_block_unused = (char *) next_block_meta;
    }

    if (hi.heap_block_used == NULL) {
        block_meta->prev = block_meta;
        block_meta->next = block_meta;
    } else {
        BlockMeta *used_block_meta = (BlockMeta *) hi.heap_block_used;
        BlockMeta *next_used_block_meta = used_block_meta->next;
        used_block_meta->next = block_meta;
        next_used_block_meta->prev = block_meta;
        block_meta->prev = used_block_meta;
        block_meta->next = next_used_block_meta;
    }
    hi.heap_block_used = (char *) block_meta;

    char *block_body = (char *) ( block + sizeof(BlockMeta));
    return block_body;
}

void mem_init_task_descriptors() 
{
    _kernel_state.td_queue_size = 0;
    for (int i = 0; i < KERNEL_STACK_TD_LIMIT; i++) {
        _kernel_state.td_user_stack_availability[i] = 0;
    }
}

void mem_init_heap_region(HEAP_TYPE heap_type)
{
    HeapInfo hi;
    _get_heap_info(&hi, heap_type);
    hi.heap_block_used = NULL;

    char *prev_block = (char *) ( hi.heap_region_addr - hi.heap_block_size );
    hi.heap_block_unused = prev_block;

    BlockMeta *prev_block_meta = (BlockMeta *) prev_block;
    prev_block_meta->id = 1;
    prev_block_meta->heap_type = heap_type;
    prev_block_meta->prev = NULL;
    for (int i = 2; i <= hi.heap_block_count; i++) {
        BlockMeta *block_meta = (BlockMeta *) hi.heap_region_addr - i * hi.heap_block_size;
        prev_block_meta->next = block_meta;
        block_meta->id = i;
        block_meta->heap_type = heap_type;
        block_meta->prev = prev_block_meta;
        prev_block_meta = block_meta;
    }
    prev_block_meta->next = NULL;

    BlockMeta *head_block_meta = (BlockMeta *) hi.heap_block_unused;
    BlockMeta *tail_block_meta = prev_block_meta;
    head_block_meta->prev = tail_block_meta;
    tail_block_meta->next = head_block_meta;
}

void mem_free(char *ptr) 
{
    BlockMeta *block_meta = (BlockMeta *) ( ptr - sizeof(BlockMeta) );
    HeapInfo hi;
    _get_heap_info(&hi, block_meta->heap_type);

    BlockMeta *curr_block_meta = (BlockMeta *) hi.heap_block_used;
    while(curr_block_meta != NULL && curr_block_meta->id != block_meta->id) {
        curr_block_meta = curr_block_meta->next;
    }
    if (curr_block_meta == NULL) return;

    BlockMeta *prev_block_meta = curr_block_meta->next;
    BlockMeta *next_block_meta = curr_block_meta->prev;

    if (prev_block_meta == next_block_meta) {
        hi.heap_block_used = NULL;
    } else {
        next_block_meta->prev = prev_block_meta;
        prev_block_meta->next = next_block_meta;
        hi.heap_block_used = (char *) next_block_meta;
    }

    if (hi.heap_block_unused == NULL) {
        block_meta->prev = block_meta;
        block_meta->next = block_meta;
    } else {
        BlockMeta *unused_block_meta = (BlockMeta *) hi.heap_block_unused;
        BlockMeta *next_unused_block_meta = unused_block_meta->next;
        unused_block_meta->next = block_meta;
        next_unused_block_meta->prev = block_meta;
        block_meta->prev = unused_block_meta;
        block_meta->next = next_unused_block_meta;
    }
    hi.heap_block_unused = (char *) block_meta;
}

char *mem_malloc(int size) 
{
    int requested_size = size + HEAP_META_SIZE;
    if (requested_size < S_HEAP_BLOCK_SIZE) {
        return _mem_get_block(SMALL);
    } else if (requested_size < M_HEAP_BLOCK_SIZE) {
        return _mem_get_block(MEDIUM);
    } else if (requested_size < L_HEAP_BLOCK_SIZE) {
        return _mem_get_block(LARGE);
    } else {
        return NULL;
    }
}