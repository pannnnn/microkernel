#include <kernel.h>
#include <user.h>
#include <shared.h>
#include <lib_periph_bwio.h>

// declared as global variable in main.c
extern KernelState _kernel_state;

void mem_init_all_heap_info() 
{
    _kernel_state.s_heap_info.heap_block_size = S_HEAP_BLOCK_SIZE;
    _kernel_state.s_heap_info.heap_block_count = S_HEAP_BLOCK_COUNT;
    _kernel_state.s_heap_info.heap_block_used = NULL;
    _kernel_state.s_heap_info.heap_block_unused = NULL;
    _kernel_state.s_heap_info.heap_region_addr = S_HEAP_REGION;

    _kernel_state.m_heap_info.heap_block_size = M_HEAP_BLOCK_SIZE;
    _kernel_state.m_heap_info.heap_block_count = M_HEAP_BLOCK_COUNT;
    _kernel_state.m_heap_info.heap_block_used = NULL;
    _kernel_state.m_heap_info.heap_block_unused = NULL;
    _kernel_state.m_heap_info.heap_region_addr = M_HEAP_REGION;

    _kernel_state.l_heap_info.heap_block_size = L_HEAP_BLOCK_SIZE;
    _kernel_state.l_heap_info.heap_block_count = L_HEAP_BLOCK_COUNT;
    _kernel_state.l_heap_info.heap_block_used = NULL;
    _kernel_state.l_heap_info.heap_block_unused = NULL;
    _kernel_state.l_heap_info.heap_region_addr = L_HEAP_REGION;
}

unsigned int _get_heap_info(HEAP_TYPE heap_type) 
{
    switch (heap_type) {
        case SMALL:
            return (unsigned int) &_kernel_state.s_heap_info;
        case MEDIUM:
            return (unsigned int) &_kernel_state.m_heap_info;
        case LARGE:
            return (unsigned int) &_kernel_state.l_heap_info;
        default:
            return NULL;
    }
}

void mem_init_heap_region(HEAP_TYPE heap_type)
{
    HeapInfo *heap_info = (HeapInfo *) _get_heap_info(heap_type);

    heap_info->heap_block_used = NULL;

    unsigned int prev_block = heap_info->heap_region_addr - heap_info->heap_block_size;
    heap_info->heap_block_unused = prev_block;

    BlockMeta *prev_block_meta = (BlockMeta *) prev_block;
    prev_block_meta->heap_type = heap_type;
    prev_block_meta->prev = NULL;
    for (int i = 2; i <= heap_info->heap_block_count; i++) {
        BlockMeta *block_meta = (BlockMeta *) (heap_info->heap_region_addr - i * heap_info->heap_block_size);
        prev_block_meta->next = block_meta;
        block_meta->heap_type = heap_type;
        block_meta->prev = prev_block_meta;
        prev_block_meta = block_meta;
    }
    prev_block_meta->next = NULL;

    BlockMeta *head_block_meta = (BlockMeta *) heap_info->heap_block_unused;
    BlockMeta *tail_block_meta = prev_block_meta;
    head_block_meta->prev = tail_block_meta;
    tail_block_meta->next = head_block_meta;
}

char *_mem_get_block(HEAP_TYPE heap_type) 
{
    HeapInfo *heap_info = (HeapInfo *) _get_heap_info(heap_type);

    if (heap_info->heap_block_unused == NULL) return NULL;

    char *block = (char *) heap_info->heap_block_unused;
    BlockMeta *block_meta = (BlockMeta *) block;
    
    BlockMeta *prev_block_meta = (BlockMeta *) block_meta->prev;
    BlockMeta *next_block_meta = (BlockMeta *) block_meta->next;
    if (block_meta == next_block_meta) {
        heap_info->heap_block_unused = NULL;
    } else {
        next_block_meta->prev = prev_block_meta;
        prev_block_meta->next = next_block_meta;
        heap_info->heap_block_unused = (unsigned int) next_block_meta;
    }

    if (heap_info->heap_block_used == NULL) {
        block_meta->prev = block_meta;
        block_meta->next = block_meta;
    } else {
        BlockMeta *used_block_meta = (BlockMeta *) heap_info->heap_block_used;
        BlockMeta *next_used_block_meta = used_block_meta->next;
        used_block_meta->next = block_meta;
        next_used_block_meta->prev = block_meta;
        block_meta->prev = used_block_meta;
        block_meta->next = next_used_block_meta;
    }
    heap_info->heap_block_used = (unsigned int) block_meta;

    char *block_body = (char *) ( block + sizeof(BlockMeta));
    return block_body;
}

void sys_free(char *ptr) 
{
    if (ptr == NULL) return;

    BlockMeta *block_meta = (BlockMeta *) ( ptr - sizeof(BlockMeta) );
    HeapInfo *heap_info = (HeapInfo *) _get_heap_info(block_meta->heap_type);

    if (heap_info == NULL) return;

    BlockMeta *prev_block_meta = block_meta->prev;
    BlockMeta *next_block_meta = block_meta->next;

    if (block_meta == next_block_meta) {
        heap_info->heap_block_used = NULL;
    } else {
        next_block_meta->prev = prev_block_meta;
        prev_block_meta->next = next_block_meta;
        heap_info->heap_block_used = (unsigned int) next_block_meta;
    }

    if (heap_info->heap_block_unused == NULL) {
        block_meta->prev = block_meta;
        block_meta->next = block_meta;
    } else {
        BlockMeta *unused_block_meta = (BlockMeta *) heap_info->heap_block_unused;
        BlockMeta *next_unused_block_meta = unused_block_meta->next;
        unused_block_meta->next = block_meta;
        next_unused_block_meta->prev = block_meta;
        block_meta->prev = unused_block_meta;
        block_meta->next = next_unused_block_meta;
    }
    heap_info->heap_block_unused = (unsigned int) block_meta;
}

char *sys_malloc(int size) 
{
    int requested_size = size + HEAP_META_SIZE;
    if (requested_size <= S_HEAP_BLOCK_SIZE) {
        return _mem_get_block(SMALL);
    } else if (requested_size <= M_HEAP_BLOCK_SIZE) {
        return _mem_get_block(MEDIUM);
    } else if (requested_size <= L_HEAP_BLOCK_SIZE) {
        return _mem_get_block(LARGE);
    } else {
        return NULL;
    }
}

void dump_heap_used(HEAP_TYPE heap_type) 
{
    HeapInfo *heap_info = (HeapInfo *) _get_heap_info(heap_type);

    BlockMeta *block_meta = (BlockMeta *) heap_info->heap_block_used;

    for (int i = 1; i <= heap_info->heap_block_count; i++) {
        log("Heap prev addr<%x> addr <%x> next addr <%x>", block_meta->prev, block_meta, block_meta->next);
        block_meta = block_meta->next;
    }
    // log("%x", heap_info->heap_block_unused);
}