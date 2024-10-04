#include <stdlib.h> //behöver den här för malloc etc
#include <stdio.h> //behöver den här för printf
#include <string.h> //behöver den här för memcpy

void mem_init(size_t size);
void* mem_alloc(size_t size);
void mem_free(void* block);
void* mem_resize(void* block, size_t size);
void mem_deinit();

