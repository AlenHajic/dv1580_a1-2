#include "memory_manager.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MIN_SIZE 16  // Minimum size for splitting a block

typedef struct Block {
    size_t size;
    int isFree;
} Block;

void* memoryPool = NULL;
size_t pool_size = 0;
size_t current_allocated_size = 0;

// Initialize the memory pool
void mem_init(size_t size) {
    if (size == 0) {
        printf("Error: Size for initialization was 0\n");
        exit(1);
    }

    memoryPool = malloc(size);
    pool_size = size;
    if (memoryPool == NULL) {
        printf("Error: Memory pool failed to initialize\n");
        exit(1);
    }

    // Initialize the first block as a free block
    Block* initialBlock = (Block*)memoryPool;
    initialBlock->size = size - sizeof(Block);
    initialBlock->isFree = 1;
    current_allocated_size = 0;  // Start with no allocations
}

// Allocate memory block
void* mem_alloc(size_t size) {
    if (size <= 0 || current_allocated_size + size + sizeof(Block) > pool_size) {
        return NULL;  // Not enough memory
    }

    Block* block = (Block*)memoryPool;

    // Traverse the memory pool using pointer arithmetic
    while ((char*)block < (char*)memoryPool + pool_size) {
        // If the block is free and large enough
        if (block->isFree && block->size >= size) {
            // Split the block if there’s enough space for the next block
            if (block->size >= size + sizeof(Block) + MIN_SIZE) {
                Block* nextBlock = (Block*)((char*)block + sizeof(Block) + size);
                nextBlock->size = block->size - size - sizeof(Block);
                nextBlock->isFree = 1;
                block->size = size;
            }

            block->isFree = 0;  // Mark block as allocated
            current_allocated_size += block->size + sizeof(Block);  // Update allocated size

            return (void*)((char*)block + sizeof(Block));  // Return usable memory
        }

        // Move to the next block
        block = (Block*)((char*)block + sizeof(Block) + block->size);
    }

    return NULL;  // No suitable block found
}

// Free memory block
void mem_free(void* ptr) {
    if (ptr == NULL) return;  // Don't free NULL pointers

    Block* block = (Block*)((char*)ptr - sizeof(Block));
    if (!block->isFree) {
        block->isFree = 1;  // Mark block as free
        current_allocated_size -= block->size + sizeof(Block);  // Update cumulative allocated size
    }

    // Optionally, you can add logic here for coalescing adjacent free blocks
}

// Resize memory block
void* mem_resize(void* block, size_t size) {
    if (block == NULL) return mem_alloc(size);  // If block is NULL, allocate new memory

    Block* header = (Block*)((char*)block - sizeof(Block));

    // If the current block is large enough
    if (header->size >= size) {
        return block;
    } else {
        // Attempt to merge with the next block if it's free and large enough
        Block* nextBlock = (Block*)((char*)header + sizeof(Block) + header->size);
        if ((char*)nextBlock < (char*)memoryPool + pool_size && nextBlock->isFree == 1 && header->size + nextBlock->size + sizeof(Block) >= size) {
            header->size += sizeof(Block) + nextBlock->size;  // Merge blocks
            return block;
        } else {
            // Allocate new memory, copy contents, and free the old block
            void* new_block = mem_alloc(size);
            if (new_block != NULL) {
                memcpy(new_block, block, header->size);
                mem_free(block);
            }
            return new_block;
        }
    }
}

// Deinitialize the memory pool
void mem_deinit() {
    free(memoryPool);
    memoryPool = NULL;
    pool_size = 0;
    current_allocated_size = 0;
}
