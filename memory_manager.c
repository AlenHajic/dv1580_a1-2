#include "memory_manager.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    if (ptr == NULL) return;  // Ignore freeing NULL pointers

    // Calculate the index of the block in the pool
    size_t blockIndex = ((char*)ptr - (char*)memoryPool) / MIN_SIZE;

    // Mark the block as free
    blockMetaArray[blockIndex].isFree = 1;

    // Attempt to merge with the next block if it's free
    if (blockIndex + 1 < blockCount && blockMetaArray[blockIndex + 1].isFree) {
        // Merge the current block with the next block
        blockMetaArray[blockIndex].size += blockMetaArray[blockIndex + 1].size;
        
        // Shift the metadata array to remove the merged block
        for (size_t i = blockIndex + 1; i < blockCount - 1; ++i) {
            blockMetaArray[i] = blockMetaArray[i + 1];
        }
        blockCount--;  // Decrease block count after merging
    }

    // Optionally: Attempt to merge with the previous block if it's free
    if (blockIndex > 0 && blockMetaArray[blockIndex - 1].isFree) {
        // Merge the previous block with the current block
        blockMetaArray[blockIndex - 1].size += blockMetaArray[blockIndex].size;
        
        // Shift the metadata array to remove the merged block
        for (size_t i = blockIndex; i < blockCount - 1; ++i) {
            blockMetaArray[i] = blockMetaArray[i + 1];
        }
        blockCount--;  // Decrease block count after merging
    }
}


// Resize memory
void* mem_resize(void* ptr, size_t newSize) {
    if (ptr == NULL) {
        // If the pointer is NULL, allocate new memory
        return mem_alloc(newSize);
    }

    // Calculate the index of the block in the pool
    size_t blockIndex = ((char*)ptr - (char*)memoryPool) / MIN_SIZE;
    BlockMeta* block = &blockMetaArray[blockIndex];

    // If the current block is large enough, return the same block
    if (block->size >= newSize) {
        return ptr;
    }

    // Check if we can expand into the next block (if it's free)
    if (blockIndex + 1 < blockCount && blockMetaArray[blockIndex + 1].isFree) {
        BlockMeta* nextBlock = &blockMetaArray[blockIndex + 1];

        // Check if merging the two blocks would be enough
        if (block->size + nextBlock->size >= newSize) {
            block->size += nextBlock->size;
            nextBlock->isFree = 0;  // Mark the next block as unavailable
            return ptr;
        }
    }

    // Allocate a new block, copy the contents, and free the old block
    void* new_block = mem_alloc(newSize);
    if (new_block != NULL) {
        memcpy(new_block, ptr, block->size);  // Copy the old data to the new block
        mem_free(ptr);  // Free the old block
    }
    return new_block;
}

// Deinitialize the memory pool
void mem_deinit() {
    free(memoryPool);  // Free the memory pool
    memoryPool = NULL;
    pool_size = 0;
    blockCount = 0;
}