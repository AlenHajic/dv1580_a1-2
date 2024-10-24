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
            return (char*)memoryPool + (i * MIN_SIZE);
        }
    }
    printf("Error: No suitable block found for size %zu\n", size);
    return NULL;  
}

//Används för att fria minne i använda block men även merga fria block som ligger intill varandra
void mem_free(void* ptr) {
    if (ptr == NULL) return;  //Kollar om det minne vi vill fria är ledigt eller inte

    size_t blockIndex = ((char*)ptr - (char*)memoryPool) / MIN_SIZE; //Ptr:n är starten på det block vi vill fria och om vi tar det
                                                                     //- starten av hela memory poolen så får vi skillnaden i distans (offseten) och sedan delar vi med hur stort ett block minst kan vara och frå vilken plats i arrayen blocket ligger.
                                                                     //Nämn att detta inte kommer funka i alla fall eftersom det kan vara olika storlek på blocks och det inte alltid är 16 bytes stora vilket sakapr problem.
                                                                     //Man får leta efter blockets startadress i en forloop där man kör memorypool + offset och för varje nytt block ökar ofset:en med sizen av vårt curennt block.
                                                                     //tills man hittar att ptr == blockStart och då kan markera den som free.
    printf("Freeing memory at block %zu\n", blockIndex);

    blockMetaArray[blockIndex].isFree = 1; //Markerar vårt block som free från den delen där vi är.

    //Merge:a 
    if (blockIndex + 1 < blockCount && blockMetaArray[blockIndex + 1].isFree) //Kollar så att blocket efter faktiskt finns och om det är free att allokera till etc.
    {
        blockMetaArray[blockIndex].size += blockMetaArray[blockIndex + 1].size; //Mergar sizen av blocket vi friade och det efter

    //Startar från det block vi mergade med och flyttar alla block höger om den ett steg åt vänster
        for (size_t i = blockIndex + 1; i < blockCount - 1; ++i) 
        {
            blockMetaArray[i] = blockMetaArray[i + 1];
        }
        blockCount--; //Minskar met ett block eftersom vi fyllde igen vårt gap
    }

    //Kollar om blocket till vänster är free för att merge:a
    if (blockIndex > 0 && blockMetaArray[blockIndex - 1].isFree)
    {
        blockMetaArray[blockIndex - 1].size += blockMetaArray[blockIndex].size;

        for (size_t i = blockIndex; i < blockCount - 1; ++i)
        {
            blockMetaArray[i] = blockMetaArray[i + 1];
        }
        blockCount--;
    }
}

//Resize:a ett block
void* mem_resize(void* ptr, size_t newSize) {
    //Om det inte finns ett block att resize:a så skapr vi ett nytt block med önskad size.
    if (ptr == NULL) 
    {
        return mem_alloc(newSize);
    }

    size_t blockIndex = ((char*)ptr - (char*)memoryPool) / MIN_SIZE; //Ändra här till en forloop som går igenom arrayen och kollr om blockStart == ptr (blockStart = memoryPool + offset och ofset börjar på 0 men ökar fter varje
                                                                     //iteradtion med sizen av vårt nuvarande block).
    BlockMeta* block = &blockMetaArray[blockIndex];

    //Om dem är lika stora eller vårt size redan är större så behöver vi inte resizea, behöver inte kolla om new_size är mindre etersom vi ine kan a bort size
    if (block->size >= newSize)  //Notera att vi inte hanterar det fall då vill shrinka blocket. Det man  får göra är att se hur mcyket minne som blir över och kolla om det går att splitta för att sedan skapa ett nytt block med remainignsize.
    {
        return ptr;
    }

    //Använder den här för att expandera vårt block åt höger om möjligt ifall vårt nuvarande block är för litet för new_size, notera att vi hade kunnat kolla samam sak åt vänster
    if (blockIndex + 1 < blockCount && blockMetaArray[blockIndex + 1].isFree) //Kolla om vi är på sista blcoket eller om det ligger ett ledigt block åt höger
    {
        BlockMeta* nextBlock = &blockMetaArray[blockIndex + 1]; //Skapar en pointer till blcoket åt höger för att kunan kolla dess metadata så att vi inte måstte skriva blockMetaArray[blockIndex + 1]

        if (block->size + nextBlock->size >= newSize) 
        {
            block->size += nextBlock->size; //Komibenrar/mergar vårt block + nästa blocks size
            nextBlock->isFree = 0; //Markera nästa block som free
            return ptr; //Returnerar en ptr till vårt "nya" blcok
        }
    }

    //Om det inte går att resize:a på något sätt så skapr vi ett nytt block.
    void* new_block = mem_alloc(newSize); //Skapar det nya blocket.
    if (new_block != NULL) //Kollar gick det att allokera?
    {
        memcpy(new_block, ptr, block->size); //Kopierar in datan från ptr(vårt block vi försökte resize:a) till vårt nya block och skriver hur många bytes som ska kopieras.
        mem_free(ptr); //Tar bort det gamla blocket
    }
    return new_block; //returnerar det gamla blocket.
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
