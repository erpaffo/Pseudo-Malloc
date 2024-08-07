#include "buddy_allocator.h"
#include <stdio.h>

#define BUFFER_SIZE 131072 // 128 KB buffer to handle memory
#define BUDDY_LEVELS 19
#define MEMORY_SIZE (1024*1024)
#define MIN_BUCKET_SIZE (MEMORY_SIZE >> BUDDY_LEVELS)

char buffer[BUFFER_SIZE];
char memory[MEMORY_SIZE];

BuddyAllocator alloc;

void test_small_allocations() {
    printf("\nRunning small allocation tests...\n");

    // Allocate small memory blocks
    void* p1 = BuddyAllocator_malloc(&alloc, 100);
    void* p2 = BuddyAllocator_malloc(&alloc, 200);
    void* p3 = BuddyAllocator_malloc(&alloc, 300);

    // Verify allocations
    if (p1 != NULL) {
        printf("Allocated 100 bytes at %p\n", p1);
    } else {
        printf("Failed to allocate 100 bytes\n");
    }

    if (p2 != NULL) {
        printf("Allocated 200 bytes at %p\n", p2);
    } else {
        printf("Failed to allocate 200 bytes\n");
    }

    if (p3 != NULL) {
        printf("Allocated 300 bytes at %p\n", p3);
    } else {
        printf("Failed to allocate 300 bytes\n");
    }

    // Free allocated memory
    BuddyAllocator_free(&alloc, p1);
    BuddyAllocator_free(&alloc, p2);
    BuddyAllocator_free(&alloc, p3);

    printf("Small allocation tests completed.\n");
}

void test_large_allocations() {
    printf("\nRunning large allocation tests...\n");

    // Allocate large memory blocks
    void* p1 = BuddyAllocator_malloc(&alloc, 2000);
    void* p2 = BuddyAllocator_malloc(&alloc, 3000);
    void* p3 = BuddyAllocator_malloc(&alloc, 5000);

    // Verify allocations
    if (p1 != NULL) {
        printf("Allocated 2000 bytes at %p\n", p1);
    } else {
        printf("Failed to allocate 2000 bytes\n");
    }

    if (p2 != NULL) {
        printf("Allocated 3000 bytes at %p\n", p2);
    } else {
        printf("Failed to allocate 3000 bytes\n");
    }

    if (p3 != NULL) {
        printf("Allocated 5000 bytes at %p\n", p3);
    } else {
        printf("Failed to allocate 5000 bytes\n");
    }

    // Free allocated memory
    BuddyAllocator_free(&alloc, p1);
    BuddyAllocator_free(&alloc, p2);
    BuddyAllocator_free(&alloc, p3);

    printf("Large allocation tests completed.\n");
}

void test_edge_cases() {
    printf("\nRunning edge case tests...\n");

    // Allocate zero bytes (should fail)
    void* p1 = BuddyAllocator_malloc(&alloc, 0);
    if (p1 == NULL) {
        printf("Correctly failed to allocate 0 bytes\n");
    } else {
        printf("Unexpectedly allocated 0 bytes at %p\n", p1);
        BuddyAllocator_free(&alloc, p1);
    }

    // Allocate negative bytes (should fail)
    void* p2 = BuddyAllocator_malloc(&alloc, -100);
    if (p2 == NULL) {
        printf("Correctly failed to allocate -100 bytes\n");
    } else {
        printf("Unexpectedly allocated -100 bytes at %p\n", p2);
        BuddyAllocator_free(&alloc, p2);
    }

    printf("Edge case tests completed.\n");
}

void test_combined_allocations() {
    printf("\nRunning combined allocation tests...\n");

    // Allocate a mix of small and large memory blocks
    void* p1 = BuddyAllocator_malloc(&alloc, 100);    // Small
    void* p2 = BuddyAllocator_malloc(&alloc, 2000);   // Large
    void* p3 = BuddyAllocator_malloc(&alloc, 300);    // Small
    void* p4 = BuddyAllocator_malloc(&alloc, 5000);   // Large

    // Verify allocations
    if (p1 != NULL) {
        printf("Allocated 100 bytes at %p\n", p1);
    } else {
        printf("Failed to allocate 100 bytes\n");
    }

    if (p2 != NULL) {
        printf("Allocated 2000 bytes at %p\n", p2);
    } else {
        printf("Failed to allocate 2000 bytes\n");
    }

    if (p3 != NULL) {
        printf("Allocated 300 bytes at %p\n", p3);
    } else {
        printf("Failed to allocate 300 bytes\n");
    }

    if (p4 != NULL) {
        printf("Allocated 5000 bytes at %p\n", p4);
    } else {
        printf("Failed to allocate 5000 bytes\n");
    }

    // Free allocated memory
    BuddyAllocator_free(&alloc, p1);
    BuddyAllocator_free(&alloc, p2);
    BuddyAllocator_free(&alloc, p3);
    BuddyAllocator_free(&alloc, p4);

    printf("Combined allocation tests completed.\n");
}

int main(int argc, char** argv) {
    // Initialize the buddy allocator
    printf("init... ");
    if (BuddyAllocator_init(&alloc, BUDDY_LEVELS, memory, MEMORY_SIZE, buffer, BUFFER_SIZE, MIN_BUCKET_SIZE) != 0) {
        printf("Failed to initialize Buddy Allocator\n");
        return -1;
    }
    printf("DONE\n");

    // Run tests
    test_small_allocations();
    test_large_allocations();
    test_edge_cases();
    test_combined_allocations();

    return 0;
}
