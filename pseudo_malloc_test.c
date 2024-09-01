#include "pseudo_malloc.h"
#include <stdio.h>
#include <stdbool.h>

#define BUFFER_SIZE 131072
#define BUDDY_LEVELS 19
#define MEMORY_SIZE (1024*1024)
#define MIN_BUCKET_SIZE (MEMORY_SIZE >> BUDDY_LEVELS)
#define THRESHOLD 1024  // 1/4 della dimensione della pagina (4096 / 4)

char buffer[BUFFER_SIZE]; // 128 KB buffer to handle memory
char memory[MEMORY_SIZE];

BuddyAllocator buddy_allocator;

typedef struct {
    int total_tests;
    int passed_tests;
} TestResult;

TestResult test_result = {0, 0};

void print_test_result(bool passed, const char* description) {
    test_result.total_tests++;
    if (passed) {
        test_result.passed_tests++;
        printf("[SUCCESS] %s\n", description);
    } else {
        printf("[ERROR] %s\n", description);
    }
}

void test_small_allocations() {
    printf("\n== Running small allocation tests ==\n");

    // Test allocazioni < 1024 byte (dovrebbero usare il Buddy Allocator)
    void* p1 = pseudo_malloc(&buddy_allocator, 100);
    print_test_result(p1 != NULL, "Allocate 100 bytes with Buddy Allocator");

    void* p2 = pseudo_malloc(&buddy_allocator, 512);
    print_test_result(p2 != NULL, "Allocate 512 bytes with Buddy Allocator");

    void* p3 = pseudo_malloc(&buddy_allocator, 1023);
    print_test_result(p3 != NULL, "Allocate 1023 bytes with Buddy Allocator");

    // Liberazione della memoria
    pseudo_free(&buddy_allocator, p1);
    pseudo_free(&buddy_allocator, p2);
    pseudo_free(&buddy_allocator, p3);

    printf("== Small allocation tests completed ==\n");
}

void test_threshold_allocation() {
    printf("\n== Running threshold allocation tests ==\n");

    // Test allocazione esattamente 1024 byte (dovrebbe usare mmap)
    void* p1 = pseudo_malloc(&buddy_allocator, 1024);
    print_test_result(p1 != NULL, "Allocate 1024 bytes with mmap");

    // Liberazione della memoria
    pseudo_free(&buddy_allocator, p1);

    printf("== Threshold allocation tests completed ==\n");
}

void test_large_allocations() {
    printf("\n== Running large allocation tests ==\n");

    // Test allocazioni > 1024 byte (dovrebbero usare mmap)
    void* p1 = pseudo_malloc(&buddy_allocator, 2000);
    print_test_result(p1 != NULL, "Allocate 2000 bytes with mmap");

    void* p2 = pseudo_malloc(&buddy_allocator, 5000);
    print_test_result(p2 != NULL, "Allocate 5000 bytes with mmap");

    void* p3 = pseudo_malloc(&buddy_allocator, 10000);
    print_test_result(p3 != NULL, "Allocate 10000 bytes with mmap");

    // Liberazione della memoria
    pseudo_free(&buddy_allocator, p1);
    pseudo_free(&buddy_allocator, p2);
    pseudo_free(&buddy_allocator, p3);

    printf("== Large allocation tests completed ==\n");
}

void test_edge_cases() {
    printf("\n== Running edge case tests ==\n");

    // Allocazione di 0 byte (deve fallire)
    void* p1 = pseudo_malloc(&buddy_allocator, 0);
    print_test_result(p1 == NULL, "Correctly failed to allocate 0 bytes");

    // Allocazione di dimensioni negative (deve fallire)
    void* p2 = pseudo_malloc(&buddy_allocator, -100);
    print_test_result(p2 == NULL, "Correctly failed to allocate -100 bytes");

    printf("== Edge case tests completed ==\n");
}

void test_combined_allocations() {
    printf("\n== Running combined allocation tests ==\n");

    // Test combinato di allocazioni piccole e grandi
    void* p1 = pseudo_malloc(&buddy_allocator, 100);    // Buddy Allocator
    print_test_result(p1 != NULL, "Allocate 100 bytes with Buddy Allocator");

    void* p2 = pseudo_malloc(&buddy_allocator, 2000);   // mmap
    print_test_result(p2 != NULL, "Allocate 2000 bytes with mmap");

    void* p3 = pseudo_malloc(&buddy_allocator, 512);    // Buddy Allocator
    print_test_result(p3 != NULL, "Allocate 512 bytes with Buddy Allocator");

    void* p4 = pseudo_malloc(&buddy_allocator, 5000);   // mmap
    print_test_result(p4 != NULL, "Allocate 5000 bytes with mmap");

    // Liberazione della memoria
    pseudo_free(&buddy_allocator, p1);
    pseudo_free(&buddy_allocator, p2);
    pseudo_free(&buddy_allocator, p3);
    pseudo_free(&buddy_allocator, p4);

    printf("== Combined allocation tests completed ==\n");
}

void print_final_results() {
    printf("\n========== TEST RESULTS ==========\n");
    printf("Total tests run: %d\n", test_result.total_tests);
    printf("Passed tests: %d\n", test_result.passed_tests);
    printf("Failed tests: %d\n", test_result.total_tests - test_result.passed_tests);
    printf("==================================\n");
}

int main(int argc, char** argv) {
    printf("Initializing Buddy Allocator... ");
    if (BuddyAllocator_init(&buddy_allocator, BUDDY_LEVELS, memory, MEMORY_SIZE, buffer, BUFFER_SIZE, MIN_BUCKET_SIZE) != 0) {
        printf("Failed to initialize Buddy Allocator\n");
        return -1;
    }
    printf("DONE\n");

    // Run tests
    test_small_allocations();
    test_threshold_allocation();
    test_large_allocations();
    test_edge_cases();
    test_combined_allocations();

    // Print final results
    print_final_results();

    return 0;
}
