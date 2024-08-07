#pragma once
#include "buddy_allocator.h"

#define THRESHOLD 1024 // 1/4 of page size (4096 / 4)

void* pseudo_malloc(BuddyAllocator* alloc, int size);
void pseudo_free(BuddyAllocator* alloc, void* ptr);
