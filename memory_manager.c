#include "memory_manager.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>  // Include for size_t

#define MAX_BLOCKS 1000  // Maximum number of blocks
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
    printf("Memory pool initialized with size: %zu\n", size);
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

            printf("Allocating memory at block %zu, size: %zu\n", i, size);

            // Return a pointer to the allocated memory using size_t for the offset
            return (char*)memoryPool + (i * MIN_SIZE);  // Use i * MIN_SIZE directly
        }
    }

    printf("Error: No suitable block found for size %zu\n", size);
    return NULL;  // No suitable block found
}

// Free memory
void mem_free(void* ptr) {
    if (ptr == NULL) return;  // Ignore freeing NULL pointers

    // Calculate the index of the block in the pool
    size_t blockIndex = ((char*)ptr - (char*)memoryPool) / MIN_SIZE;

    printf("Freeing memory at block %zu\n", blockIndex);

    // Mark the block as free
    blockMetaArray[blockIndex].isFree = 1;

    // Attempt to merge with the next block if it's free
    if (blockIndex + 1 < blockCount && blockMetaArray[blockIndex + 1].isFree) {
        blockMetaArray[blockIndex].size += blockMetaArray[blockIndex + 1].size;

        for (size_t i = blockIndex + 1; i < blockCount - 1; ++i) {
            blockMetaArray[i] = blockMetaArray[i + 1];
        }
        blockCount--;
    }

    // Merge with the previous block if it's free
    if (blockIndex > 0 && blockMetaArray[blockIndex - 1].isFree) {
        blockMetaArray[blockIndex - 1].size += blockMetaArray[blockIndex].size;

        for (size_t i = blockIndex; i < blockCount - 1; ++i) {
            blockMetaArray[i] = blockMetaArray[i + 1];
        }
        blockCount--;
    }
}

// Resize memory
void* mem_resize(void* ptr, size_t newSize) {
    if (ptr == NULL) {
        return mem_alloc(newSize);
    }

    size_t blockIndex = ((char*)ptr - (char*)memoryPool) / MIN_SIZE;
    BlockMeta* block = &blockMetaArray[blockIndex];

    if (block->size >= newSize) {
        return ptr;
    }

    if (blockIndex + 1 < blockCount && blockMetaArray[blockIndex + 1].isFree) {
        BlockMeta* nextBlock = &blockMetaArray[blockIndex + 1];

        if (block->size + nextBlock->size >= newSize) {
            block->size += nextBlock->size;
            nextBlock->isFree = 0;
            return ptr;
        }
    }

    void* new_block = mem_alloc(newSize);
    if (new_block != NULL) {
        memcpy(new_block, ptr, block->size);
        mem_free(ptr);
    }
    return new_block;
}

// Deinitialize the memory pool
void mem_deinit() {
    free(memoryPool);
    memoryPool = NULL;
    pool_size = 0;
    blockCount = 0;
    printf("Memory pool deinitialized.\n");
}
