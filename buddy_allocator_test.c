#include "buddy_allocator.h"
#include <stdio.h>

#define BUFFER_SIZE 131072 // 128 KB buffer to handle memory
#define BUDDY_LEVELS 19
#define MEMORY_SIZE (1024*1024)
#define MIN_BUCKET_SIZE (MEMORY_SIZE >> BUDDY_LEVELS)

char buffer[BUFFER_SIZE];
char memory[MEMORY_SIZE];

BuddyAllocator alloc;

typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
} TestSummary;

TestSummary summary = {0, 0, 0};

void print_allocation_result(void* ptr, int size, int expected_failure) {
    summary.total_tests++;
    if ((ptr != NULL && !expected_failure) || (ptr == NULL && expected_failure)) {
        summary.passed_tests++;
        printf("[SUCCESS] %s %d bytes %s\n", expected_failure ? "Failed to allocate" : "Allocated", size, ptr ? "" : "(expected failure)");
    } else {
        summary.failed_tests++;
        printf("[ERROR] %s %d bytes at address %p\n", expected_failure ? "Unexpectedly allocated" : "Failed to allocate", size, ptr);
    }
}

void print_free_result(void* ptr, int size) {
    if (ptr != NULL) {
        printf("[INFO] Freed %d bytes from address %p\n", size, ptr);
    } else {
        printf("[WARNING] Attempted to free a NULL pointer\n");
    }
}

void test_small_allocations() {
    printf("\n== Running small allocation tests ==\n");

    void* p1 = BuddyAllocator_malloc(&alloc, 100);
    print_allocation_result(p1, 100, 0);

    void* p2 = BuddyAllocator_malloc(&alloc, 200);
    print_allocation_result(p2, 200, 0);

    void* p3 = BuddyAllocator_malloc(&alloc, 300);
    print_allocation_result(p3, 300, 0);

    BuddyAllocator_free(&alloc, p1);
    print_free_result(p1, 100);

    BuddyAllocator_free(&alloc, p2);
    print_free_result(p2, 200);

    BuddyAllocator_free(&alloc, p3);
    print_free_result(p3, 300);

    printf("== Small allocation tests completed ==\n");
}

void test_large_allocations() {
    printf("\n== Running large allocation tests ==\n");

    void* p1 = BuddyAllocator_malloc(&alloc, 2000);
    print_allocation_result(p1, 2000, 0);

    void* p2 = BuddyAllocator_malloc(&alloc, 3000);
    print_allocation_result(p2, 3000, 0);

    void* p3 = BuddyAllocator_malloc(&alloc, 5000);
    print_allocation_result(p3, 5000, 0);

    BuddyAllocator_free(&alloc, p1);
    print_free_result(p1, 2000);

    BuddyAllocator_free(&alloc, p2);
    print_free_result(p2, 3000);

    BuddyAllocator_free(&alloc, p3);
    print_free_result(p3, 5000);

    printf("== Large allocation tests completed ==\n");
}

void test_edge_cases() {
    printf("\n== Running edge case tests ==\n");

    void* p1 = BuddyAllocator_malloc(&alloc, 0);
    print_allocation_result(p1, 0, 1); // Expected to fail

    void* p2 = BuddyAllocator_malloc(&alloc, -100);
    print_allocation_result(p2, -100, 1); // Expected to fail

    printf("== Edge case tests completed ==\n");
}

void test_combined_allocations() {
    printf("\n== Running combined allocation tests ==\n");

    void* p1 = BuddyAllocator_malloc(&alloc, 100);    // Small
    print_allocation_result(p1, 100, 0);

    void* p2 = BuddyAllocator_malloc(&alloc, 2000);   // Large
    print_allocation_result(p2, 2000, 0);

    void* p3 = BuddyAllocator_malloc(&alloc, 300);    // Small
    print_allocation_result(p3, 300, 0);

    void* p4 = BuddyAllocator_malloc(&alloc, 5000);   // Large
    print_allocation_result(p4, 5000, 0);

    BuddyAllocator_free(&alloc, p1);
    print_free_result(p1, 100);

    BuddyAllocator_free(&alloc, p2);
    print_free_result(p2, 2000);

    BuddyAllocator_free(&alloc, p3);
    print_free_result(p3, 300);

    BuddyAllocator_free(&alloc, p4);
    print_free_result(p4, 5000);

    printf("== Combined allocation tests completed ==\n");
}

void print_final_summary() {
    printf("\n========== TEST SUMMARY ==========\n");
    printf("Total tests run: %d\n", summary.total_tests);
    printf("Passed tests: %d\n", summary.passed_tests);
    printf("Failed tests: %d\n", summary.failed_tests);
    printf("==================================\n");
}

int main(int argc, char** argv) {
    printf("Initializing Buddy Allocator... ");
    if (BuddyAllocator_init(&alloc, BUDDY_LEVELS, memory, MEMORY_SIZE, buffer, BUFFER_SIZE, MIN_BUCKET_SIZE) != 0) {
        printf("[FATAL] Failed to initialize Buddy Allocator\n");
        return -1;
    }
    printf("DONE\n");

    // Run tests
    test_small_allocations();
    test_large_allocations();
    test_edge_cases();
    test_combined_allocations();

    // Print final results
    print_final_summary();

    return 0;
}
