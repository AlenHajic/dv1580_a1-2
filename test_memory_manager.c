#include "memory_manager.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "common_defs.h"

#include "gitdata.h"

void test_init()
{
    printf_yellow("  Testing mem_init ---> ");
    mem_init(1024);               // Initialize with 1KB of memory
    void *block = mem_alloc(100); // Try allocating to check if init was successful
    my_assert(block != NULL);

    mem_free(block);
    mem_deinit();
    printf_green("[PASS].\n");
}

void test_alloc_and_free()
{
    printf_yellow("  Testing mem_alloc and mem_free ---> ");
    mem_init(1024);
    void *block1 = mem_alloc(100);
    my_assert(block1 != NULL);
    void *block2 = mem_alloc(200);
    my_assert(block2 != NULL);
    mem_free(block1);
    mem_free(block2);
    mem_deinit();
    printf_green("[PASS].\n");
}

void test_zero_alloc_and_free()
{
    printf_yellow("  Testing mem_alloc(0) and mem_free --->");
    mem_init(1024);
    void *block1 = mem_alloc(0);
    my_assert(block1 != NULL);
    void *block2 = mem_alloc(200);
    my_assert(block2 != NULL);
    my_assert(block1 == block2);

    mem_free(block1);
    mem_free(block2);
    mem_deinit();
    printf_green("[PASS].\n");
}

void test_random_blocks()
{
    printf_yellow("  Testing random blocks and mem_free ---> ");
    srand(time(NULL));
    int nBlocks = 1000 + rand() % 10000;
    int blockSize = rand() % 1024;

    int memSize = nBlocks * 1024;

    mem_init(memSize);
    void *blocks[nBlocks];

#ifdef DEBUG
    printf_yellow("  Allocating; %d blocks, total %d bytes, max block size %d bytes\n", nBlocks, memSize, blockSize);
#endif
    for (int k = 0; k < nBlocks; k++)
    {
        blocks[k] = mem_alloc(blockSize);
        my_assert(blocks[k] != NULL);
        blockSize = rand() % 1024;
    }
#ifdef DEBUG
    printf_yellow("  Releasing the blocks.\n");
#endif
    for (int k = 0; k < nBlocks; k++)
    {
        mem_free(blocks[k]);
    }
    mem_free(blocks[0]);

    printf_green("[PASS].\n");
}

void test_resize()
{
    printf_yellow("  Testing mem_resize ---> ");
    mem_init(1024);
    void *block = mem_alloc(100);
    my_assert(block != NULL);
    block = mem_resize(block, 200);
    my_assert(block != NULL);
    mem_free(block);
    mem_deinit();
    printf_green("[PASS].\n");
}

void test_exceed_single_allocation()
{
    printf_yellow("  Testing allocation exceeding pool size ---> ");
    mem_init(1024);                // Initialize with 1KB of memory
    void *block = mem_alloc(2048); // Try allocating more than available
    my_assert(block == NULL);      // Allocation should fail
    mem_deinit();
    printf_green("[PASS].\n");
}

void test_exceed_cumulative_allocation()
{
    printf_yellow("  Testing cumulative allocations exceeding pool size ---> ");
    mem_init(1024); // Initialize with 1KB of memory
    void *block1 = mem_alloc(512);
    my_assert(block1 != NULL);
    void *block2 = mem_alloc(512);
    my_assert(block2 != NULL);
    void *block3 = mem_alloc(100); // This should fail, no space left
    my_assert(block3 == NULL);
    mem_free(block1);
    mem_free(block2);
    mem_deinit();
    printf_green("[PASS].\n");
}

void test_memory_overcommit()
{
    printf_yellow("  Testing memory over-commitment ---> ");
    mem_init(1024); // Initialize with 1KB of memory

    void *block1 = mem_alloc(1020); // Allocate almost all memory
    my_assert(block1 != NULL);
    void *block2 = mem_alloc(10); // Try allocating beyond the limit
    my_assert(block2 == NULL);    // Expect NULL because it exceeds available memory

    mem_free(block1);
    mem_deinit();
    printf_green("[PASS].\n");
}

void test_boundary_condition()
{
    printf_yellow("  Testing boundary conditions ---> ");
    mem_init(1024); // Initialize with 1KB of memory

    void *block = mem_alloc(1024); // Attempt to allocate the exact pool size
    my_assert(block != NULL);
    void *block2 = mem_alloc(1); // This should fail as there is no space left
    my_assert(block2 == NULL);

    mem_free(block);
    mem_deinit();
    printf_green("[PASS].\n");
}

void test_exact_fit_reuse()
{
    printf_yellow("  Testing exact fit reuse ---> ");
    mem_init(1024); // Initialize with 1KB of memory

    void *block1 = mem_alloc(500);
    mem_free(block1);
    void *block2 = mem_alloc(500); // Reuse the exact space freed
    my_assert(block1 == block2);   // Should be the same address if reused properly

    mem_free(block2);
    mem_deinit();
    printf_green("[PASS].\n");
}

void test_frequent_small_allocations()
{
    printf_yellow("  Testing frequent small allocations ---> ");
    mem_init(1024); // Initialize with 1KB of memory

    const int num_allocations = 50;
    void *blocks[num_allocations];

    for (int i = 0; i < num_allocations; i++)
    {
        blocks[i] = mem_alloc(10); // Small allocations
        my_assert(blocks[i] != NULL);
    }

    for (int i = 0; i < num_allocations; i++)
    {
        mem_free(blocks[i]);
    }

    mem_deinit();
    printf_green("[PASS].\n");
}

void test_memory_reuse()
{
    printf_yellow("  Testing memory reuse ---> ");
    mem_init(1024);

    void *block1 = mem_alloc(256);
    void *block2 = mem_alloc(256);
    mem_free(block1);
    void *block3 = mem_alloc(128); // This should ideally reuse the space from block1
    my_assert(block3 == block1);   // Check if the same memory is reused

    mem_free(block2);
    mem_free(block3);
    mem_deinit();
    printf_green("[PASS].\n");
}

void test_block_merging()
{
    printf_yellow("  Testing block merging ---> ");
    mem_init(1024);

    void *block1 = mem_alloc(200);
    void *block2 = mem_alloc(200);
    void *block3 = mem_alloc(200);
    mem_free(block1);
    mem_free(block3);
    mem_free(block2); // Freeing block2 should trigger merging with block1 and block3

    void *block4 = mem_alloc(600); // Should fit into the merged block
    my_assert(block4 != NULL);

    mem_free(block4);
    mem_deinit();
    printf_green("[PASS].\n");
}

void test_non_contiguous_allocation_failure()
{
    printf_yellow("  Testing non-contiguous allocation failure ---> ");
    mem_init(800); // Initialize with 800 bytes of memory

    // Allocate several blocks to fragment the memory
    void *block1 = mem_alloc(250);
    void *block2 = mem_alloc(250);
    void *block3 = mem_alloc(250);
    mem_free(block1); // Free the first block
    mem_free(block3); // Free the third block, leaving non-contiguous free slots

    // Attempt to allocate a block larger than any single free block but smaller than the total free space
    void *block4 = mem_alloc(500);
    my_assert(block4 == NULL); // This allocation should fail due to lack of contiguous space

    mem_free(block2); // Cleanup
    mem_deinit();
    printf_green("[PASS].\n");
}

void test_contiguous_allocation_success()
{
    printf_yellow("  Testing contiguous allocation success ---> ");
    mem_init(1024); // Initialize with 1KB of memory

    // Allocate and then free a block to create a sufficiently large contiguous free block
    void *block1 = mem_alloc(256);
    void *block2 = mem_alloc(256);
    void *block3 = mem_alloc(512);
    mem_free(block1); // Free block1 and block2 to create a contiguous free space
    mem_free(block2); // now block1 and block2 are contiguous

    // Try to allocate a block that fits into the freed space
    void *block4 = mem_alloc(500);
    my_assert(block2 != NULL); // This allocation should succeed

    mem_free(block3);
    mem_free(block4);
    mem_deinit();
    printf_green("[PASS].\n");
}

void test_double_free()
{
    printf_yellow("  Testing double deallocation ---> ");
    mem_init(1024); // Initialize with 1KB of memory

    void *block = mem_alloc(100); // Allocate a block of 100 bytes
    my_assert(block != NULL);     // Ensure the block was allocated

    mem_free(block); // Free the block for the first time
    mem_free(block); // Attempt to free the block a second time

    printf_green("[PASS].\n");
    mem_deinit(); // Cleanup memory
}

void test_memory_fragmentation()
{
    printf_yellow("  Testing memory fragmentation handling ---> ");
    mem_init(1024); // Initialize with 1024 bytes

    void *block1 = mem_alloc(200);
    void *block2 = mem_alloc(300);
    void *block3 = mem_alloc(500);
    mem_free(block1);              // Free first block
    mem_free(block3);              // Free third block, leaving a fragmented hole before and after block2
    void *block4 = mem_alloc(500); // Should fit into the space of block
    assert(block4 != NULL);

    mem_free(block2);
    mem_free(block4);
    mem_deinit();
    printf_green("[PASS].\n");
}


  
void test_edge_case_allocations()
{
    printf_yellow("  Testing edge case allocations ---> ");
    mem_init(1024); // Initialize with 1024 bytes

    void *block0 = mem_alloc(0); // Edge case: zero allocation
    // assert(block0 != NULL);      // Depending on handling, this could also be NULL

    void *block1 = mem_alloc(1024); // Exactly remaining
    assert(block1 != NULL);

    void *block2 = mem_alloc(1); // Attempt to allocate with no space left
    assert(block2 == NULL);

    mem_free(block0);
    mem_free(block1);
    mem_deinit();
    printf_green("[PASS].\n");
}

int main(int argc, char *argv[])
{
#ifdef VERSION
    printf("Build Version; %s \n", VERSION);
#endif
    printf("Git Version; %s/%s \n", git_date, git_sha);

    if (argc < 2)
    {
        printf("Usage: %s <test function>\n", argv[0]);
        printf("Available test functions:\n");
        printf("Basic Operations:\n");
        printf(" 1. test_init - Initialize memory system\n");
        printf(" 2. test_alloc_and_free - Test basic allocation and deallocation\n");
        printf(" 3. test_resize - Test resizing allocated memory\n");

        printf("\nStress and Edge Cases:\n");
        printf(" 4. test_exceed_single_allocation - Test allocation beyond total memory\n");
        printf(" 5. test_exceed_cumulative_allocation - Test cumulative allocations exceeding total memory\n");
        printf(" 6. test_memory_overcommit - Test memory over-commitment\n");
        printf(" 7. test_boundary_condition - Test boundary conditions\n");
        printf(" 8. test_exact_fit_reuse - Test reuse of exact fit memory\n");
        printf(" 9. test_double_free - Test handling of double free operations\n");
        printf(" 10. test_memory_fragmentation - Test handling of memory fragmentation\n");
        printf(" 11. test_edge_case_allocations - Test allocations at edge conditions\n");

        printf("\nAdvanced Memory Management:\n");
        printf(" 12. test_frequent_small_allocations - Test frequent small allocations\n");
        printf(" 13. test_memory_reuse - Test reuse of freed memory\n");
        printf(" 14. test_block_merging - Test merging of adjacent free blocks\n");
        printf(" 15. test_non_contiguous_allocation_failure - Ensure failure when no contiguous block fits\n");
        printf(" 16. test_contiguous_allocation_success - Ensure success when a contiguous block fits\n");

	
	printf("\nVarious tests: \n");
	printf(" 17. test_zero_alloc_and_free - Ensure that we can allocate 0 bytes, and it does not fail.\n");
	printf(" 18. test_random_blocks - Test that we can allocate a random size, and random amounts of blocks [1000,10000]. \n\n");
        printf(" 0. Run all tests\n");
        return 1;
    }

    switch (atoi(argv[1]))
    {
    case -1:
        printf("No tests will be executed.\n");
        break;
    case 0:
        // Running all tests
        printf("Testing Basic Operations:\n");
        test_init();
        test_alloc_and_free();
        test_resize();

        printf("\nTesting Stress and Edge Cases:\n");
        test_exceed_single_allocation();
        test_exceed_cumulative_allocation();
        test_memory_overcommit();
        test_boundary_condition();
        test_exact_fit_reuse();
        test_double_free();
        test_memory_fragmentation();
        test_edge_case_allocations();

        printf("\nTesting Advanced Memory Management:\n");
        test_frequent_small_allocations();
        test_memory_reuse();
        test_block_merging();
        test_non_contiguous_allocation_failure();
        test_contiguous_allocation_success();

        printf("\nVarious other tests:\n");
        test_zero_alloc_and_free();
        test_random_blocks();
        break;
    case 1:
        test_init();
        break;
    case 2:
        test_alloc_and_free();
        break;
    case 3:
        test_resize();
        break;
    case 4:
        test_exceed_single_allocation();
        break;
    case 5:
        test_exceed_cumulative_allocation();
        break;
    case 6:
        test_memory_overcommit();
        break;
    case 7:
        test_boundary_condition();
        break;
    case 8:
        test_exact_fit_reuse();
        break;
    case 9:
        test_double_free();
        break;
    case 10:
        test_memory_fragmentation();
        break;
    case 11:
        test_edge_case_allocations();
        break;
    case 12:
        test_frequent_small_allocations();
        break;
    case 13:
        test_memory_reuse();
        break;
    case 14:
        test_block_merging();
        break;
    case 15:
        test_non_contiguous_allocation_failure();
        break;
    case 16:
        test_contiguous_allocation_success();
        break;
    case 17:
        test_zero_alloc_and_free();
        break;
    case 18:
        test_random_blocks();
        break;
    default:
        printf("Invalid test function\n");
        break;
    }
    return 0;
}