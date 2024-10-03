#include "memory_manager.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h> // Make sure to include this for bool type

typedef struct memoryBlock {
    size_t size;
    bool is_free;
    struct memoryBlock* nextBlock;
} memoryBlock;

memoryBlock* memoryPool = NULL;
size_t poolSize = 0;
size_t allocatedSize = 0;
void* memory_address;

void mem_init(size_t size) {
    memory_address = malloc(size);
    if (!memory_address) {
        printf("Memory initializing failed!\n");
        exit(EXIT_FAILURE);
    }

    memoryPool = (memoryBlock*)memory_address;
    memoryPool->size = size - sizeof(memoryBlock); // Adjust the initial block size
    memoryPool->is_free = true; // Set the first block as free
    memoryPool->nextBlock = NULL; // No next block yet

    poolSize = size;
    printf("Memory initialized! Size: %zu bytes\n", size);
}

void* mem_alloc(size_t size) {
    if (size <= 0 || poolSize < allocatedSize + size) {
        printf("Not enough space! Allocation failed!\n");
        return NULL; // Return NULL instead of calling exit
    }

    memoryBlock* current = memoryPool; // Start from the beginning of the memory pool

    while (current) {
        if (current->is_free && size <= current->size) {
            // Split the block if there's enough space for the next block
            if (current->size >= size + sizeof(memoryBlock)) {
                memoryBlock* newMemBlock = (memoryBlock*)((char*)current + sizeof(memoryBlock) + size);
                newMemBlock->size = current->size - size - sizeof(memoryBlock);
                newMemBlock->is_free = true; // Set the new block as free
                newMemBlock->nextBlock = current->nextBlock; // Link the new block to the next one

                current->size = size; // Resize the current block
                current->nextBlock = newMemBlock; // Link the new block
            }

            current->is_free = false; // Mark the current block as allocated
            allocatedSize += current->size + sizeof(memoryBlock); // Update allocated size
            printf("Memory allocated! Size: %zu bytes\n", current->size);
            return (void*)((char*)current + sizeof(memoryBlock)); // Return pointer to the usable memory
        }
        current = current->nextBlock; // Move to the next block
    }

    printf("Failed to allocate memory!\n");
    return NULL; // Return NULL if allocation failed
}

void mem_free(void* block) {
    if (!block) {
        printf("Cannot free null block\n");
        return;
    }

    memoryBlock* thisBlock = (memoryBlock*)((char*)block - sizeof(memoryBlock));
    thisBlock->is_free = true; // Mark the block as free
    printf("Memory block freed. Size: %zu bytes\n", thisBlock->size);
    allocatedSize -= thisBlock->size + sizeof(memoryBlock); // Update allocated size
}

void* mem_resize(void* block, size_t size) {
    if (!block) {
        printf("Can't resize a non-existing block!\n");
        return mem_alloc(size); // If block is NULL, allocate a new block
    }

    memoryBlock* walker = (memoryBlock*)((char*)block - sizeof(memoryBlock));
    if (walker->size >= size) {
        return block; // If the current block is big enough, return it
    }

    void* newBlock = mem_alloc(size);
    if (newBlock) {
        size_t copy_size = (walker->size < size) ? walker->size : size;
        memcpy(newBlock, block, copy_size); // Copy the existing data to the new block
        mem_free(block); // Free the old block
    }
    return newBlock; // Return the new block pointer
}

void mem_deinit() {
    if (memory_address) {
        free(memory_address);
        memoryPool = NULL;
        memory_address = NULL;   
        poolSize = 0;
        allocatedSize = 0; // Reset allocated size
        printf("Memory deinitialized.\n");
    }
}
