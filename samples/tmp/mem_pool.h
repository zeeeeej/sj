#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct MemoryBlock {
    struct MemoryBlock* next;
} MemoryBlock;

typedef struct {
    MemoryBlock* free_list;
    size_t block_size;
    size_t block_count;
    void* pool;
} MemoryPool;

MemoryPool* create_memory_pool(size_t block_size, size_t block_count);
void destroy_memory_pool(MemoryPool* pool);
void* allocate_memory(MemoryPool* pool);
void free_memory(MemoryPool* pool, void* ptr);