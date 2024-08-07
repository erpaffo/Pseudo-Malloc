#include <stdio.h>
#include <assert.h>
#include "pseudo_malloc.h"
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

void* pseudo_malloc(BuddyAllocator* alloc, int size) {
    if (size < 0) {
        printf("\nMalloc error: Invalid Size (<0)\n");
        return NULL;
    }
    if (size == 0) {
        printf("\nMalloc error: Cannot allocate 0 bytes\n");
        return NULL;
    }

    if (size >= THRESHOLD) { // for large allocations use mmap
        printf("\nAllocation to be done with mmap, size: %d\n", size);
        int memory_size = size + sizeof(int);
        void *p = mmap(NULL, memory_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (p == MAP_FAILED) {
            printf("Malloc error: mmap failed with error: %s", strerror(errno));
            return NULL;
        } else {
            ((int*)p)[0] = memory_size;
            printf("Allocation succeeded: address: %p, size: %d\n", p + sizeof(int), memory_size);
            return (void *)(p + sizeof(int));
        }
    } else { // for small allocations use buddy allocator
        printf("\nAllocation to be done with Buddy Allocator, size: %d", size);
        void* p = BuddyAllocator_malloc(alloc, size);
        if (!p) { // allocation error
            return NULL;
        } else {
            return p;
        }
    }
}

void pseudo_free(BuddyAllocator* alloc, void* ptr) {
    if (!ptr) {
        printf("\nFree error: Memory to be freed is NULL\n");
        return;
    }

    int* original_ptr = (int*)((char*)ptr - sizeof(int));
    int size = *original_ptr;

    if (size >= THRESHOLD) {
        printf("\nFree to be done with munmap\n");
        int ret = munmap((void*)original_ptr, size);
        if (ret != 0) {
            printf("\nFree error: munmap failed\n");
            return;
        }
        printf("\nFree succeeded: Memory block at address %p freed\n", ptr);
    } else {
        printf("\nFree to be done with Buddy Allocator\n");
        BuddyAllocator_free(alloc, ptr);
    }
}
