#include "memory_manager.h"

#define MAX_BLOCKS 1000 // Maximum number of blocks
#define MIN_SIZE 16     // Minimum size for a block

// A struct to hold metadata of each block
typedef struct {
    size_t size;
    int isFree;
} BlockMeta;

// Global variables for memory management
void* memoryPool = NULL;                 // Pointer to memory pool
BlockMeta blockMetaArray[MAX_BLOCKS];    // Metadata array
size_t pool_size = 0;                    // Size of the pool
size_t blockCount = 0;                   // Number of blocks in the pool

// Mutex for thread-safe memory manager operations
pthread_mutex_t memory_mutex = PTHREAD_MUTEX_INITIALIZER;

// Initialize the memory pool
void mem_init(size_t size) {
    pthread_mutex_init(&memory_mutex, NULL);  // Initialize the mutex

    memoryPool = malloc(size);
    pool_size = size;
    if (!memoryPool) {
        printf("Failed to initialize memory pool.\n");
        exit(1);
    }

    blockMetaArray[0].size = size;
    blockMetaArray[0].isFree = 1;
    blockCount = 1;

    printf("Memory pool initialized with size: %zu\n", size);
}

void* mem_alloc(size_t size) {
    pthread_mutex_lock(&memory_mutex);  // Lock the mutex

    size_t offset = 0;
    for (size_t i = 0; i < blockCount; ++i) {
        if (blockMetaArray[i].isFree && blockMetaArray[i].size >= size) {
            size_t remainingSize = blockMetaArray[i].size - size;

            // If the remaining size can fit a new block, split it
            if (remainingSize >= MIN_SIZE) {
                blockMetaArray[i].size = size;
                blockMetaArray[i].isFree = 0;  // Mark the block as allocated

                // Create a new block from the remaining space
                blockMetaArray[blockCount].size = remainingSize;
                blockMetaArray[blockCount].isFree = 1;  // Mark the new block as free
                blockCount++;
            } else {
                // If the block is exactly the right size or cannot be split, allocate the entire block
                blockMetaArray[i].isFree = 0;
            }

            pthread_mutex_unlock(&memory_mutex);  // Unlock the mutex
            return (char*)memoryPool + offset;
        }

        // Update the offset to point to the next block
        offset += blockMetaArray[i].size;
    }

    pthread_mutex_unlock(&memory_mutex);  // Unlock the mutex
    printf("Error: No suitable block found for size %zu\n", size);
    return NULL;
}


// Free allocated memory
void mem_free(void* ptr) {
    if (ptr == NULL) return;

    pthread_mutex_lock(&memory_mutex);  // Lock the mutex

    size_t offset = 0;
    for (size_t i = 0; i < blockCount; ++i) {
        if ((char*)memoryPool + offset == (char*)ptr) {
            blockMetaArray[i].isFree = 1;

            if (i + 1 < blockCount && blockMetaArray[i + 1].isFree) {
                blockMetaArray[i].size += blockMetaArray[i + 1].size;
                for (size_t j = i + 1; j < blockCount - 1; ++j) {
                    blockMetaArray[j] = blockMetaArray[j + 1];
                }
                blockCount--;
            }

            if (i > 0 && blockMetaArray[i - 1].isFree) {
                blockMetaArray[i - 1].size += blockMetaArray[i].size;
                for (size_t j = i; j < blockCount - 1; ++j) {
                    blockMetaArray[j] = blockMetaArray[j + 1];
                }
                blockCount--;
            }

            pthread_mutex_unlock(&memory_mutex);  // Unlock the mutex
            return;
        }

        offset += blockMetaArray[i].size;
    }

    pthread_mutex_unlock(&memory_mutex);  // Unlock the mutex
    printf("Error: Pointer not found in memory pool.\n");
}

// Resize memory
void* mem_resize(void* ptr, size_t newSize) {
    if (ptr == NULL) return mem_alloc(newSize);

    pthread_mutex_lock(&memory_mutex);  // Lock the mutex

    size_t offset = 0;
    for (size_t i = 0; i < blockCount; ++i) {
        if ((char*)memoryPool + offset == (char*)ptr) {
            BlockMeta* block = &blockMetaArray[i];

            if (block->size >= newSize) {
                pthread_mutex_unlock(&memory_mutex);  // Unlock the mutex
                return ptr;
            }

            if (i + 1 < blockCount && blockMetaArray[i + 1].isFree) {
                BlockMeta* nextBlock = &blockMetaArray[i + 1];
                if (block->size + nextBlock->size >= newSize) {
                    block->size += nextBlock->size;
                    blockMetaArray[i + 1].isFree = 0;
                    pthread_mutex_unlock(&memory_mutex);  // Unlock the mutex
                    return ptr;
                }
            }

            void* new_block = mem_alloc(newSize);
            if (new_block != NULL) {
                memcpy(new_block, ptr, block->size);
                mem_free(ptr);
            }

            pthread_mutex_unlock(&memory_mutex);  // Unlock the mutex
            return new_block;
        }

        offset += blockMetaArray[i].size;
    }

    pthread_mutex_unlock(&memory_mutex);  // Unlock the mutex
    printf("Error: Pointer not found in memory pool for resize.\n");
    return NULL;
}

// Deinitialize the memory pool
void mem_deinit() {
    pthread_mutex_destroy(&memory_mutex);  // Destroy the mutex

    free(memoryPool);
    memoryPool = NULL;
    pool_size = 0;
    blockCount = 0;

    printf("Memory pool deinitialized.\n");
}
