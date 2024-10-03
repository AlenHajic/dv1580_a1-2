#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//stdio används vid printf etc.
//stdbool används för använding av bool datatypen.
//stdlib används för malloc etc.

// #define MAX_BLOCKS 100 //Maximum number of blocks; 

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

void* mem_alloc(size_t size)
{
    void* current = memoryPool;

    while(current < ((char*)memoryPool + pool_size)) //char eftersom memoryPoolen är en void och när vi adderar pool_size flyttar vi pointern med ett antal bites vileks inte går då den är en void, därför convertar vi den till en char.
    {
        Block* block = (Block*)current; //Skapar ett nytt block.
        if(block->isFree == 1 && block->size >= size) //Kollar om blocket är ledigt + kollar om det blocket vi skapade innan är tillräkligt stort för att hålla vår size.
        {
            if(block->size >= size + sizeof(block) + 1) //Kollar om vi kan splitta
            {
                Block* nextBlock = (Block*)((char*)current + size + sizeof(Block)); //Skapar blocket efter att vi splittat och lägger starten av det efter "main" blocket + ger det sin egen header/metadata
                nextBlock->size = block->size - size - sizeof(Block); //Ger sizen till blocket med det överblivna minnet
                nextBlock->isFree = 1; //Markerar att detta block är free
                block->size = size; //Lägger "main" till den sizen vi ville allocatea
            }
            block->isFree = 0; //markera "main" blocket som ledigt
            return (void*)((char*)current + sizeof(Block)); //vi uppdaterar pointern av våran current för att accounta för starten av våran user memory (starten av första blocket).
        }
        current = (char*)current + sizeof(Block) + block->size; //här flyttar vi current till slutet av det använda blocket.
    }
    return NULL; //indicates no memory block was large enough.
}

void mem_free(void* block)
{
    if(block == NULL) return;

    Block* header = (Block*)(char*)block - sizeof(block);
    header->isFree = 1;
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