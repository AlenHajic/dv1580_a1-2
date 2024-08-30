#include "memory_manager.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "common_defs.h"

void test_init()
{
    printf_yellow("Testing mem_init...\n");
    mem_init(1024);               // Initialize with 1KB of memory
    void *block = mem_alloc(100); // Try allocating to check if init was successful
    my_assert(block != NULL);
    mem_free(block);
    mem_deinit();
    printf_green("mem_init passed.\n");
}

void test_alloc_and_free()
{
    printf_yellow("Testing mem_alloc and mem_free...\n");
    mem_init(1024);
    void *block1 = mem_alloc(100);
    my_assert(block1 != NULL);
    void *block2 = mem_alloc(200);
    my_assert(block2 != NULL);
    mem_free(block1);
    mem_free(block2);
    mem_deinit();
    printf_green("mem_alloc and mem_free passed.\n");
}

void test_resize()
{
    printf_yellow("Testing mem_resize...\n");
    mem_init(1024);
    void *block = mem_alloc(100);
    my_assert(block != NULL);
    block = mem_resize(block, 200);
    my_assert(block != NULL);
    mem_free(block);
    mem_deinit();
    printf_green("mem_resize passed.\n");
}

void test_exceed_single_allocation()
{
    printf_yellow("Testing allocation exceeding pool size...\n");
    mem_init(1024);                // Initialize with 1KB of memory
    void *block = mem_alloc(2048); // Try allocating more than available
    my_assert(block == NULL);      // Allocation should fail
    mem_deinit();
    printf_green("Single allocation exceeding pool size passed.\n");
}

void test_exceed_cumulative_allocation()
{
    printf_yellow("Testing cumulative allocations exceeding pool size...\n");
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
    printf_green("Cumulative allocations exceeding pool size passed.\n");
}

void test_double_free()
{
    printf_yellow("Testing double deallocation...\n");
    mem_init(1024); // Initialize with 1KB of memory

    void *block = mem_alloc(100); // Allocate a block of 100 bytes
    my_assert(block != NULL);     // Ensure the block was allocated

    mem_free(block); // Free the block for the first time
    mem_free(block); // Attempt to free the block a second time

    printf_green("Double deallocation test passed (if no crash and handled gracefully).\n");
    mem_deinit(); // Cleanup memory
}

void test_memory_fragmentation()
{
    printf_yellow("Testing memory fragmentation handling...\n");
    mem_init(1024); // Initialize with 2048 bytes

    void *block1 = mem_alloc(200);
    void *block2 = mem_alloc(300);
    void *block3 = mem_alloc(500);
    mem_free(block1);              // Free first block
    mem_free(block3);              // Free third block, leaving a fragmented hole before and after block2
    void *block4 = mem_alloc(500); // Should fit into the space of block1
    assert(block4 != NULL);

    mem_free(block2);
    mem_free(block4);
    mem_deinit();
    printf_green("Memory fragmentation test passed.\n");
}

void test_edge_case_allocations()
{
    printf_yellow("Testing edge case allocations...\n");
    mem_init(1024); // Initialize with 1024 bytes

    void *block0 = mem_alloc(0); // Edge case: zero allocation
    assert(block0 != NULL);      // Depending on handling, this could also be NULL

    void *block1 = mem_alloc(1024); // Exactly remaining
    assert(block1 != NULL);

    void *block2 = mem_alloc(1); // Attempt to allocate with no space left
    assert(block2 == NULL);

    mem_free(block0);
    mem_free(block1);
    mem_deinit();
    printf_green("Edge case allocations test passed.\n");
}

int main()
{
    test_init();
    test_alloc_and_free();
    test_resize();
    test_exceed_single_allocation();
    test_exceed_cumulative_allocation();
    test_double_free();
    test_memory_fragmentation();
    test_edge_case_allocations();
    printf_green("All tests passed successfully!\n");
    return 0;
}
