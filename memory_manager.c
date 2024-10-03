#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_SIZE 16  // Minimum size for splitting a block

// Block structure with a nextBlock pointer
typedef struct Block {
    size_t size;        // Size of the block
    int isFree;         // Whether the block is free (1) or allocated (0)
    struct Block* next; // Pointer to the next block in the pool
} Block;

void* memoryPool = NULL;  // Pointer to the memory pool
size_t pool_size;         // Size of the memory pool
size_t current_allocated_size = 0;  // Tracks the current total allocated size

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

    // Set up the initial block as a free block
    Block* initialBlock = (Block*)memoryPool;
    initialBlock->size = size - sizeof(Block);
    initialBlock->isFree = 1;
    initialBlock->next = NULL;  // No next block yet
}

// Allocate a block of memory
void* mem_alloc(size_t size) {
    if (size <= 0 || current_allocated_size + size + sizeof(Block) > pool_size) {
        return NULL;  // Not enough memory
    }

    Block* current = (Block*)memoryPool;

    // First-fit allocation strategy
    while (current != NULL) {
        // Check if the block is free and large enough
        if (current->isFree && current->size >= size) {
            // Split the block if there's enough space
            if (current->size >= size + sizeof(Block) + MIN_SIZE) {
                Block* nextBlock = (Block*)((char*)current + sizeof(Block) + size);
                nextBlock->size = current->size - size - sizeof(Block);
                nextBlock->isFree = 1;
                nextBlock->next = current->next;  // Link the new block to the next one
                current->next = nextBlock;  // Link the current block to the new block
                current->size = size;
            }

            current->isFree = 0;  // Mark block as allocated
            current_allocated_size += size + sizeof(Block);  // Update allocated size
            return (void*)((char*)current + sizeof(Block));  // Return usable memory pointer
        }

        // Move to the next block
        current = current->next;
    }

    return NULL;  // No suitable block found
}

// Free a block of memory
void mem_free(void* ptr) {
    if (ptr == NULL) return;  // Don't free NULL pointers

    Block* block = (Block*)((char*)ptr - sizeof(Block));
    block->isFree = 1;  // Mark the block as free
    current_allocated_size -= block->size + sizeof(Block);  // Update allocated size

    // Coalesce with the next block if it's also free
    if (block->next != NULL && block->next->isFree) {
        block->size += sizeof(Block) + block->next->size;  // Merge the blocks
        block->next = block->next->next;  // Link to the block after the merged one
    }
}

// Resize a block of memory
void* mem_resize(void* ptr, size_t size) {
    if (ptr == NULL) return mem_alloc(size);  // If ptr is NULL, allocate new memory

    Block* block = (Block*)((char*)ptr - sizeof(Block));

    // If the current block is already large enough, return the same block
    if (block->size >= size) {
        return ptr;
    }

    // Check if the next block is free and large enough to accommodate the resize
    if (block->next != NULL && block->next->isFree && block->size + block->next->size + sizeof(Block) >= size) {
        block->size += sizeof(Block) + block->next->size;  // Merge with the next block
        block->next = block->next->next;  // Link to the block after the merged one
        return ptr;
    }

    // Allocate new memory, copy contents, and free the old block
    void* new_block = mem_alloc(size);
    if (new_block != NULL) {
        memcpy(new_block, ptr, block->size);  // Copy old data to new block
        mem_free(ptr);  // Free the old block
    }
    return new_block;
}

// Deinitialize the memory manager and free the memory pool
void mem_deinit() {
    free(memoryPool);
    memoryPool = NULL;
    pool_size = 0;
}
