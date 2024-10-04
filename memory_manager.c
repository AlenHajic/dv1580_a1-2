#include "memory_manager.h"

#define MAX_BLOCKS 1000 
#define MIN_SIZE 16    

typedef struct {
    size_t size;   
    int isFree;    
} BlockMeta;

void* memoryPool = NULL;
BlockMeta blockMetaArray[MAX_BLOCKS];  
size_t pool_size = 0;
size_t blockCount = 0;


void mem_init(size_t size) {
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
    for (size_t i = 0; i < blockCount; ++i) {
        if (blockMetaArray[i].isFree && blockMetaArray[i].size >= size) {
            
            size_t remainingSize = blockMetaArray[i].size - size;

            if (remainingSize >= MIN_SIZE + sizeof(BlockMeta)) {
                blockMetaArray[i].size = size;
                blockMetaArray[i].isFree = 0;

                blockMetaArray[blockCount].size = remainingSize;
                blockMetaArray[blockCount].isFree = 1;
                blockCount++;
            } 
            else
            {
                blockMetaArray[i].isFree = 0;
            }

            printf("Allocating memory at block %zu, size: %zu\n", i, size);

            return (char*)memoryPool + (i * MIN_SIZE);
        }
    }

    printf("Error: No suitable block found for size %zu\n", size);
    return NULL;  
}

void mem_free(void* ptr) {
    if (ptr == NULL) return;  

    size_t blockIndex = ((char*)ptr - (char*)memoryPool) / MIN_SIZE;

    printf("Freeing memory at block %zu\n", blockIndex);

    blockMetaArray[blockIndex].isFree = 1;

    if (blockIndex + 1 < blockCount && blockMetaArray[blockIndex + 1].isFree) {
        blockMetaArray[blockIndex].size += blockMetaArray[blockIndex + 1].size;

        for (size_t i = blockIndex + 1; i < blockCount - 1; ++i) {
            blockMetaArray[i] = blockMetaArray[i + 1];
        }
        blockCount--;
    }

    if (blockIndex > 0 && blockMetaArray[blockIndex - 1].isFree) {
        blockMetaArray[blockIndex - 1].size += blockMetaArray[blockIndex].size;

        for (size_t i = blockIndex; i < blockCount - 1; ++i) {
            blockMetaArray[i] = blockMetaArray[i + 1];
        }
        blockCount--;
    }
}

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

void mem_deinit() {
    free(memoryPool);
    memoryPool = NULL;
    pool_size = 0;
    blockCount = 0;
    printf("Memory pool deinitialized.\n");
}
