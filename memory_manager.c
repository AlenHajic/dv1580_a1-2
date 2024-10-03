#include "memory_manager.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//stdio används vid printf etc.
//stdbool används för använding av bool datatypen.
//stdlib används för malloc etc.

// #define MAX_BLOCKS 100 //Maximum number of blocks; 

#define MIN_SIZE 16

typedef struct Block
{
    size_t size;
    int isFree;
} Block;

//typedef gör att vi slipper skriva struct efter avrje nytt block vi skapar etc, enda undantage är i själva structen vid nextBlock. Anledningen
//är att att structen Block inte ännu fått detta allias och compilern inte förstår det föresen efter. Alltså kan vi i fortsättningen skriva 
//block newBlock enbart.

//size_t är det man oftast använder i såna här sammahang fär memory, arrays etc. Detta för att size_t är så pass stor att den kan hålla nästinitll
//vilken storlek vi en ger den.

// void* memory_pool = NULL;
void* memoryPool = NULL;
size_t pool_size;
size_t current_allocated_size = 0;

void mem_init(size_t size)
{
    if(size == 0)
    {
        printf("%s", "Size for initiliazintion was 0");
        exit(1);
    }

    memoryPool = malloc(size);
    pool_size = size;
    if(memoryPool == NULL)
    {
        printf("%s", "Memory pool failed to initialize \n");
        exit(1);
    }

    Block* initialBlock = (Block*)memoryPool;
    initialBlock->size = size - sizeof(Block);
    initialBlock->isFree = 1;

}

void* mem_alloc(size_t size) {
    void* current = memoryPool;

    // Make sure the request size is valid and doesn't exceed the pool size
    if (size <= 0 || current_allocated_size + size + sizeof(Block) > pool_size) {
        return NULL; // Not enough memory
    }
    
    while ((char*)current + sizeof(Block) + size <= (char*)memoryPool + pool_size) {
        Block* block = (Block*)current;

        // Check if block is free and large enough
        if (block->isFree && block->size >= size) {
            // Split block if there's enough space for the next block
            if (block->size >= size + sizeof(Block) + MIN_SIZE) {
                Block* nextBlock = (Block*)((char*)current + size + sizeof(Block));
                nextBlock->size = block->size - size - sizeof(Block);
                nextBlock->isFree = 1;
                block->size = size;
            }
            block->isFree = 0; // Mark block as allocated
            current_allocated_size += size + sizeof(Block); // Update the total allocated size
            printf("Current allocated size: %c, Requested size: %c\n", (char)current_allocated_size, (char)size);

            return (void*)((char*)current + sizeof(Block)); // Return usable memory pointer
        }

        // Move to the next block
        current = (char*)current + sizeof(Block) + block->size;
    }
    return NULL; // No memory block was large enough
}

void mem_free(void* ptr) {
    if (ptr == NULL) return; // Don't free NULL pointers

    Block* block = (Block*)((char*)ptr - sizeof(Block));
    block->isFree = 1; // Mark the block as free
    current_allocated_size -= block->size; // Update cumulative allocated size

    // Optionally, implement coalescing adjacent free blocks here
}

void* mem_resize(void* block, size_t size)
{
    if(block == NULL) return mem_alloc(size);

    Block* header = (Block*)(char*)block - sizeof(block);
    if(header->size >= size) 
    {
        return block;
    }
    else
    {
        Block* nextBlock = (Block*)(char*)block + sizeof(Block) + header->size;
        if((char*)nextBlock < (char*)memoryPool + pool_size && nextBlock->isFree == 1 && header->size + nextBlock->size + sizeof(block) >= size)
        {

            header->size += sizeof(block) + nextBlock->size;
            return block;
        }
        else
        {
            void* new_block = mem_alloc(size);
            if (new_block != NULL) {
                memcpy(new_block, block, header->size);
                mem_free(block);
            }
            return new_block;
        }
    }

}

void mem_deinit()
{
    free(memoryPool);
    memoryPool = NULL;
    pool_size = 0;

}
