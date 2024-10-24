#include "memory_manager.h"

#define MAX_BLOCKS 1000 //Definear en limit för max antal blocks vi kan ha i våran array
#define MIN_SIZE 16 //Definerar en limit för min size som ett block kan ha, ser till att de kan hålla strukten och inte blir för små vid splittring    

//En strukt för att hålla koll på ett blokcs metadata
typedef struct {
    size_t size;   
    int isFree;    
} BlockMeta; //Ser till att vi inte behöver skriva struct efter varje BlockMeta struct vi skapar

void* memoryPool = NULL; //En pionter till hela minnes poolen 
BlockMeta blockMetaArray[MAX_BLOCKS]; //En array för metadatan av varje block i poolen, max limit av 100 block
size_t pool_size = 0; //Hela sizen av minnes poolen
size_t blockCount = 0; //En counter för antal block i poolen

//En funktion för att initialisera minnes poolen
void mem_init(size_t size) {
    memoryPool = malloc(size); //Allokerar minne av önskad size och returnerar en pointer till starten
    pool_size = size;
    if (!memoryPool) //Om poolen inte har något minne så kan vi inte köra någon annan funktion och avslutar därför programmet 
    {
        printf("Failed to initialize memory pool.\n");
        exit(1); //1an är ett sätt att visa på att något error uppstod 
    }
    
    blockMetaArray[0].size = size; //Skapar ett block som täcker hela minnes poolen
    blockMetaArray[0].isFree = 1; //Markera att blocket är free för allokering
    blockCount = 1;
    printf("Memory pool initialized with size: %zu\n", size);
}

//Funktion för att allokera ett block med önskad strolek
void* mem_alloc(size_t size) {
    size_t offset = 0;
    for (size_t i = 0; i < blockCount; ++i) { //Loopar igenom arrayen för att se vilka block som finns att allokera till eller inte
        if (blockMetaArray[i].isFree && blockMetaArray[i].size >= size)
        {
            size_t remainingSize = blockMetaArray[i].size - size; //Tar reda på hur mycket av storleken som blir över när vi allokerar så mcyket minne som ösnakdes
            if (remainingSize >= MIN_SIZE + sizeof(BlockMeta)) { //Kollar om resternade minne är tillräcklgit stor att splittas, minne under MIN_SIZE går dock till spillo, skulle kunna implementera ett system för att ändå splitta och mergea 
                blockMetaArray[i].size = size;
                blockMetaArray[i].isFree = 0; //Inte free

                blockMetaArray[blockCount].size = remainingSize; //Skapar ett nytt block av kvarstående minne (Spitting)
                blockMetaArray[blockCount].isFree = 1;
                blockCount++;
            } 
            else
            {
                blockMetaArray[i].isFree = 0;
            }
            // printf("Allocating memory at block %zu, size: %zu\n", i, size);
            return (char*)memoryPool + offset;
        }
        offset += blockMetaArray[i].size;
    }
    printf("Error: No suitable block found for size %zu\n", size);
    return NULL;  
}

//Används för att fria minne i använda block men även merga fria block som ligger intill varandra
void mem_free(void* ptr) {
    if (ptr == NULL) return;

    size_t offset = 0;
    for (size_t i = 0; i < blockCount; ++i) {
        // Kolla om pekaren matchar startadressen för något block
        if ((char*)memoryPool + offset == (char*)ptr) {
            printf("Freeing memory at block %zu\n", i);
            blockMetaArray[i].isFree = 1;

            // Kolla och merga block till höger om möjligt
            if (i + 1 < blockCount && blockMetaArray[i + 1].isFree) {
                blockMetaArray[i].size += blockMetaArray[i + 1].size;
                for (size_t j = i + 1; j < blockCount - 1; ++j) {
                    blockMetaArray[j] = blockMetaArray[j + 1];
                }
                blockCount--;
            }

            // Kolla och merga block till vänster om möjligt
            if (i > 0 && blockMetaArray[i - 1].isFree) {
                blockMetaArray[i - 1].size += blockMetaArray[i].size;
                for (size_t j = i; j < blockCount - 1; ++j) {
                    blockMetaArray[j] = blockMetaArray[j + 1];
                }
                blockCount--;
            }
            return;
        }
        // Uppdatera offset med storleken på det nuvarande blocket
        offset += blockMetaArray[i].size;
    }

    printf("Error: Pointer not found in memory pool.\n");
}


//Resize:a ett block
void* mem_resize(void* ptr, size_t newSize) {
    if (ptr == NULL) {
        return mem_alloc(newSize);
    }

    size_t offset = 0;
    for (size_t i = 0; i < blockCount; ++i) {
        // Hitta blocket som matchar pekaren
        if ((char*)memoryPool + offset == (char*)ptr) {
            BlockMeta* block = &blockMetaArray[i];

            if (block->size >= newSize) {
                return ptr;
            }

            // Kolla om blocket till höger kan användas för att expandera
            if (i + 1 < blockCount && blockMetaArray[i + 1].isFree) {
                BlockMeta* nextBlock = &blockMetaArray[i + 1];
                if (block->size + nextBlock->size >= newSize) {
                    block->size += nextBlock->size;
                    blockMetaArray[i + 1].isFree = 0;
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
        offset += blockMetaArray[i].size;
    }

    printf("Error: Pointer not found in memory pool for resize.\n");
    return NULL;
}


//Friar allt minne från poolen
void mem_deinit() 
{
    free(memoryPool); //Friar det allokerade minnet från poolen
    memoryPool = NULL; //Lägger den som NULL
    pool_size = 0; //Reset:ar pool_size
    blockCount = 0; //Reset:ar blockCount
    printf("Memory pool deinitialized.\n");
}
