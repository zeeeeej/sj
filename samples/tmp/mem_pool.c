
#include "mem_pool.h"
// 初始化内存池
MemoryPool* create_memory_pool(size_t block_size, size_t block_count) {
    MemoryPool* pool = (MemoryPool*)malloc(sizeof(MemoryPool));
    if (!pool) return NULL;

    pool->block_size = block_size;
    pool->block_count = block_count;
    pool->pool = malloc(block_size * block_count);
    if (!pool->pool) {
        free(pool);
        return NULL;
    }

    pool->free_list = pool->pool;

    // 初始化空闲链表
    MemoryBlock* current = pool->free_list;
    for (size_t i = 0; i < block_count - 1; ++i) {
        current->next = (MemoryBlock*)((char*)current + block_size);
        current = current->next;
    }
    current->next = NULL;

    return pool;
}

// 从内存池中分配内存
void* memory_pool_alloc(MemoryPool* pool) {
    if (!pool->free_list) return NULL; // 没有可用块

    MemoryBlock* block = pool->free_list;
    pool->free_list = block->next;
    return block;
}

// 将内存块释放回内存池
void memory_pool_free(MemoryPool* pool, void* ptr) {
    if (!ptr) return;

    MemoryBlock* block = (MemoryBlock*)ptr;
    block->next = pool->free_list;
    pool->free_list = block;
}

// 销毁内存池
void destroy_memory_pool(MemoryPool* pool) {
    if (pool) {
        free(pool->pool);
        free(pool);
    }
}