#include "memory_manager.h"

#define MAX_BLOCKS 1000 // Define a limit for the max number of blocks we can have in our array
#define MIN_SIZE 16     // Define a limit for the min size of a block to ensure it can hold metadata and avoid being too small during splitting

// A struct to hold the metadata of each block
typedef struct {
    size_t size;
    int isFree;
} BlockMeta; // This ensures we don't need to write "struct" every time we create a BlockMeta struct

void* memoryPool = NULL;                 // A pointer to the entire memory pool
BlockMeta blockMetaArray[MAX_BLOCKS];    // An array to hold the metadata of each block in the pool, with a max limit of 1000 blocks
size_t pool_size = 0;                    // The total size of the memory pool
size_t blockCount = 0;                   // A counter for the number of blocks in the pool

// Function to initialize the memory pool
void mem_init(size_t size) {
    memoryPool = malloc(size); // Allocate memory of the requested size and return a pointer to the start
    pool_size = size;
    if (!memoryPool) { // If memoryPool is NULL, we failed to allocate memory, so terminate the program
        printf("Failed to initialize memory pool.\n");
        exit(1); // Exit code 1 signifies an error occurred
    }

    // Create a block that spans the entire memory pool
    blockMetaArray[0].size = size;
    blockMetaArray[0].isFree = 1; // Mark the block as free for allocation
    blockCount = 1;

    printf("Memory pool initialized with size: %zu\n", size);
}

// Function to allocate a block of the requested size
void* mem_alloc(size_t size) {
    size_t offset = 0;
    for (size_t i = 0; i < blockCount; ++i) {
        if (blockMetaArray[i].isFree && blockMetaArray[i].size >= size) {
            size_t remainingSize = blockMetaArray[i].size - size;

            // Check if the remaining size is large enough to split into a new block
            if (remainingSize >= MIN_SIZE) {
                blockMetaArray[i].size = size;
                blockMetaArray[i].isFree = 0; // Mark the block as allocated (not free)

                // Create a new block with the remaining size
                blockMetaArray[blockCount].size = remainingSize; // Remaining memory forms a new block
                blockMetaArray[blockCount].isFree = 1;           // Mark the new block as free
                blockCount++;
            } else {
                // If no splitting is possible, allocate the entire block
                blockMetaArray[i].isFree = 0;
            }

            // Return a pointer to the allocated block within memoryPool
            return (char*)memoryPool + offset;
        }

        // Update offset with the current block's size
        offset += blockMetaArray[i].size;
    }

    printf("Error: No suitable block found for size %zu\n", size);
    return NULL;
}

// Function to free an allocated block
void mem_free(void* ptr) {
    if (ptr == NULL) return;

    size_t offset = 0;
    for (size_t i = 0; i < blockCount; ++i) {
        // Check if the pointer matches the start of a block in memoryPool
        if ((char*)memoryPool + offset == (char*)ptr) {
            printf("Freeing memory at block %zu\n", i);
            blockMetaArray[i].isFree = 1;

            // Try to merge with the next block if it is free
            if (i + 1 < blockCount && blockMetaArray[i + 1].isFree) {
                blockMetaArray[i].size += blockMetaArray[i + 1].size; // Merge the sizes
                for (size_t j = i + 1; j < blockCount - 1; ++j) {
                    blockMetaArray[j] = blockMetaArray[j + 1];
                }
                blockCount--;
            }

            // Try to merge with the previous block if it is free
            if (i > 0 && blockMetaArray[i - 1].isFree) {
                blockMetaArray[i - 1].size += blockMetaArray[i].size; // Merge the sizes
                for (size_t j = i; j < blockCount - 1; ++j) {
                    blockMetaArray[j] = blockMetaArray[j + 1];
                }
                blockCount--;
            }
            return;
        }

        // Update the offset with the current block's size
        offset += blockMetaArray[i].size;
    }

    printf("Error: Pointer not found in memory pool.\n");
}

// Function to resize an allocated block
void* mem_resize(void* ptr, size_t newSize) {
    if (ptr == NULL) {
        return mem_alloc(newSize); // If the pointer is NULL, allocate a new block
    }

    size_t offset = 0;
    for (size_t i = 0; i < blockCount; ++i) {
        // Find the block that matches the pointer
        if ((char*)memoryPool + offset == (char*)ptr) {
            BlockMeta* block = &blockMetaArray[i];

            if (block->size >= newSize) {
                return ptr; // If the current block is large enough, no need to resize
            }

            // Check if we can expand the block into the next block if it is free
            if (i + 1 < blockCount && blockMetaArray[i + 1].isFree) {
                BlockMeta* nextBlock = &blockMetaArray[i + 1];
                if (block->size + nextBlock->size >= newSize) {
                    block->size += nextBlock->size; // Merge the current block with the next block
                    blockMetaArray[i + 1].isFree = 0; // Mark the next block as not free
                    return ptr;
                }
            }

            // If resizing is not possible, allocate a new block
            void* new_block = mem_alloc(newSize);
            if (new_block != NULL) {
                memcpy(new_block, ptr, block->size); // Copy data to the new block
                mem_free(ptr);                       // Free the old block
            }
            return new_block;
        }

        // Update the offset with the current block's size
        offset += blockMetaArray[i].size;
    }

    printf("Error: Pointer not found in memory pool for resize.\n");
    return NULL;
}

// Function to deinitialize the memory pool
void mem_deinit() {
    free(memoryPool);      // Free the allocated memory from the pool
    memoryPool = NULL;     // Set the pointer to NULL
    pool_size = 0;         // Reset the pool size
    blockCount = 0;        // Reset the block count
    printf("Memory pool deinitialized.\n");
}
