#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "memory_manager.h"

#define MAX_BLOCKS 100  // Maximum number of blocks
#define MIN_SIZE 16     // Minimum block size to split

typedef struct {
    size_t size;   // Size of the block
    int isFree;    // 1 if free, 0 if allocated
} BlockMeta;

void* memoryPool = NULL;
BlockMeta blockMetaArray[MAX_BLOCKS];  // Array to store metadata for each block
size_t pool_size = 0;
size_t blockCount = 0;

// Initialize the memory pool and metadata array
void mem_init(size_t size) {
    memoryPool = malloc(size);
    pool_size = size;
    if (!memoryPool) {
        printf("Failed to initialize memory pool.\n");
        exit(1);
    }

    // Initialize the metadata array with a single large free block
    blockMetaArray[0].size = size;
    blockMetaArray[0].isFree = 1;
    blockCount = 1;
}

// Allocate memory
void* mem_alloc(size_t size) {
    for (size_t i = 0; i < blockCount; ++i) {
        if (blockMetaArray[i].isFree && blockMetaArray[i].size >= size) {
            // Allocate memory in this block
            size_t remainingSize = blockMetaArray[i].size - size;

            // If there's enough space to split the block, create a new metadata entry
            if (remainingSize >= MIN_SIZE + sizeof(BlockMeta)) {
                blockMetaArray[i].size = size;
                blockMetaArray[i].isFree = 0;

                blockMetaArray[blockCount].size = remainingSize;
                blockMetaArray[blockCount].isFree = 1;
                blockCount++;
            } else {
                // Otherwise, allocate the entire block
                blockMetaArray[i].isFree = 0;
            }

            // Return a pointer to the allocated memory
            return (char*)memoryPool + (i * MIN_SIZE);
        }
    }

    return NULL;  // No suitable block found
}

// Free memory
void mem_free(void* ptr) {
    if (ptr == NULL) return;

    // Calculate the index of the block in the pool
    size_t blockIndex = ((char*)ptr - (char*)memoryPool) / MIN_SIZE;

    // Mark the block as free
    if (blockIndex < blockCount) {
        blockMetaArray[blockIndex].isFree = 1;
    }
}

// Deinitialize the memory pool
void mem_deinit() {
    free(memoryPool);
    memoryPool = NULL;
    pool_size = 0;
    blockCount = 0;
}
