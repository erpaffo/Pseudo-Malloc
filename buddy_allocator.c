#include <stdio.h>
#include <assert.h>
#include <math.h> // for floor and log2
#include "buddy_allocator.h"

///////////////////////////////////////////////////////////
// these are trivial helpers to support you in case you want
// to do a bitmap implementation

// level of node i
int levelIdx(size_t idx){
  return (int)floor(log2(idx+1));
};

// index of the buddy of node i
int buddyIdx(int idx){
  if (idx == 0) // root
    return 0;
  if (idx % 2) // if odd
    return idx - 1;
  return idx + 1; // if even
}

// parent of the node idx
int parentIdx(int idx){
  return (int)(idx - 1) / 2; // always considering 0 as root
}

// idx of 1st node of a level i
int firstIdx(int level){
  return (1 << level) - 1; 
}

// offset of node idx in its level
int startIdx(int idx){
    return (idx - (firstIdx(levelIdx(idx))));
}
///////////////////////////////////////////////////////////

int BuddyAllocator_init(BuddyAllocator* alloc,
                         int num_levels,
                         char* memory,
                         int memory_size,
                         char* bitmap_buffer,
                         int bitmap_buffer_size,
                         int min_bucket_size){
                        
    // buffer checks
    if (!memory){
      printf("Error: Memory pointer provided is NULL\n");
      return -1;
    } 
    if (!bitmap_buffer){
      printf("Error: Bitmap buffer pointer provided is NULL\n");
      return -1;
    }       

    // size checks
    if (memory_size <= 0){
      printf("Error: Memory size must be > 0\n");
      return -1;
    } 
    if (bitmap_buffer_size <= 0){
      printf("Error: Bitmap buffer size must be > 0\n");
      return -1;
    }              

    // level checks
    if (num_levels >= MAX_LEVELS){
      printf("Error: Number of levels exceeds the maximum (%d)\n", MAX_LEVELS);
      return -1;
    }

    // consistency check of min_bucket_size with memory size and number of levels
    if (min_bucket_size != memory_size >> num_levels){
      printf("Error: Invalid min_bucket_size\n");
      return -1;
    }

    // if the memory is not a power of 2, the largest possible portion of available memory (which is a power of 2) will be used
    if (log2(memory_size) != floor(log2(memory_size))){
        memory_size = min_bucket_size << num_levels;
    }

    alloc->num_levels = num_levels;
    alloc->memory = memory;
    alloc->memory_size = memory_size;
    alloc->min_bucket_size = min_bucket_size;
    
    // check the size of the bitmap and available memory to allocate it
    int num_bits = (1 << (num_levels + 1)) - 1; // number of bits will be (2 * 2^num_levels ) - 1 because level 0 is always counted
    if (bitmap_buffer_size < BitMap_getBytes(num_bits)){
      printf("Error: Insufficient memory provided for Bitmap: requires %d bytes\n", BitMap_getBytes(num_bits));
      return -1;
    }
    // initialization
    BitMap_init(&(alloc->bitmap), num_bits, (uint8_t*)bitmap_buffer);
    printf("Buddy Allocator Created\nLevels: %d\nMemory Size: %d\nNumber of bits in the bitmap: %d\nBitmap size: %d\nMinimum Bucket Size: %d\n", num_levels, memory_size, num_bits, BitMap_getBytes(num_bits), min_bucket_size);
    return 0;
}

// find a free buddy to return to malloc, also inserting the block index in the bitmap
// and size in the block to return (for operation)
void* BuddyAllocator_getBuddy(BuddyAllocator* alloc, int level, int size){
  int bitmap_idx = -1;
  if (level == 0){ // root
    int bit = BitMap_bit(&alloc->bitmap, firstIdx(level));
    if (bit == 0) bitmap_idx = 0;
    // if not free, bitmap_idx remains -1, indicating no free blocks found
  }
  else{
    int i = firstIdx(level); // iterate over the entire level to find a buddy
    while (i < firstIdx(level + 1)){ // until the beginning of the next level
      int bit = BitMap_bit(&alloc->bitmap, i); 
      if (bit == 0){
         bitmap_idx = i; 
         break;
      }
      i++;
    }
  }
  if (bitmap_idx == -1){ // if no free blocks found
    return NULL;
  }
  else{ // block found
    // update the bitmap setting to 1 the ancestors and children of the taken block
    update_child(&alloc->bitmap, bitmap_idx, 1); // both functions set the bit indicating the index
    update_parent(&alloc->bitmap, bitmap_idx, 1); // of the taken block to 1 (being recursive): no need to do it here

    int block_size = alloc->min_bucket_size << (alloc->num_levels - level); // block_size = bucket_size * 2 ^num_level - level
                                                                            // because levels are counted top-down,
                                                                            // the size is reversed

    char *ret = alloc->memory + ((startIdx(bitmap_idx) + 1) * block_size); // the address to return is calculated by adding to the start
                                                                          // of the memory the offset of the index in its level * block size
    
    // save the bitmap index in the block
    ((int*)ret)[0] = bitmap_idx;
    ((int*)ret)[1] = size; // save the size for checking whether to deallocate the block with munmap or free from the buddy allocator
    return (void *)(ret + 2 * sizeof(int)); // + size of the block address in bitmap and block size (original)
  }
}

void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size){

  // size checks
  if (size < 0){
    printf("\nMalloc error: Invalid Size (<0)\n");
    return NULL;
  }
  if (size == 0) {
    printf("\nMalloc error: Cannot allocate 0 bytes\n");
    return NULL;
  }

  // add space to save the block address in the bitmap and its original size
  // to check in pseudo_free whether to deallocate with buddy_free or munmap
  int org_size = size; // save the original size
  size += 2 * sizeof(int); // overhead (8 bytes)

  // check available space
  if (size > alloc->memory_size){
    printf("\nMalloc error: Requested memory larger than total available memory\n");
    return NULL;
  }

  // determine the level of the page
  int mem_size = (1 << alloc->num_levels) * alloc->min_bucket_size;
  // log2(mem_size): n bits to represent the whole memory
  // log2(size): n bits to represent the requested chunk
  // bits_mem_size - bits_size = depth of the chunk = level
  int level = floor(log2(mem_size / size));
  // if the level is too small, pad it to max
  if (level > alloc->num_levels){ level = alloc->num_levels; }

  printf("\nRequested: %d bytes (+ 8 bytes overhead), required %d bytes, at level %d\n", org_size, alloc->min_bucket_size << (alloc->num_levels - level), level);

  // find a free block in the bitmap
  void* address = BuddyAllocator_getBuddy(alloc, level, org_size);
  if (address == NULL){
    printf("Malloc error: no free memory block available\n");
  }
  else{
    printf("Allocation succeeded: address %p\n", address);
    return address;
  }
  return NULL;
}

void BuddyAllocator_releaseBuddy(BuddyAllocator* alloc, int bit, void* mem){
  // check for double free
  if (BitMap_bit(&alloc->bitmap, bit) == 0){
     printf("\nFree error: Memory block at index: %p, already freed (double free).\n", mem);
    return;
  }
  // update the children's bit to 0 recursively
  update_child(&alloc->bitmap, bit, 0);
  // update the parent's bit to 0 and try to merge, all recursively
  merge(&alloc->bitmap, bit);
  printf("\nFree succeeded: Memory block at index %p freed\n", mem);
}

void BuddyAllocator_free(BuddyAllocator* alloc, void* mem){
  if (!mem){
    printf("\nFree error: Memory to be freed is NULL\n");
    return;
  }
  // retrieve the buddy bit in the system, having saved it in memory
  int* p = (int*) mem;
  p--; // to skip the size saved in the block
  int idx = *(--p); // bitmap index of the block
  BuddyAllocator_releaseBuddy(alloc, idx, mem);
}

// when a block is freed, check if its buddy is free, and if so
// merge, i.e., free the parent block of the buddies.
void merge(BitMap *bitmap, int bit){
    if (bit == 0) return; // root
  
    int value = BitMap_bit(bitmap, bit);
    // sanity check
    if (value == 1){ 
      printf("\n Fatal Error in bitmap (merge on bit 1)\n");
      return;
    }
    // find the buddy index and see if it is free or not
    int buddy = buddyIdx(bit);
    value = BitMap_bit(bitmap, buddy);
    if (value == 1){ // if not free do nothing
      return;
    }
    else { // otherwise set the parent's bit to 0 merging the children, all recursively
      int parent = parentIdx(bit);
      BitMap_setBit(bitmap, parent, 0);
      merge(bitmap, parent); // upward recursion
    }
}

// set the bit itself and its parent bit in the bitmap to the given value (1 or 0)
void update_parent(BitMap *bitmap, int bit, int value) {
  BitMap_setBit(bitmap,  bit, value);
  if ( bit > 0) { // stop at the root
    update_parent(bitmap, parentIdx(bit), value); // upward recursion in the tree
  }
}

void update_child(BitMap *bitmap, int  bit, int value) {
  if ( bit < bitmap->num_bits) { // stop when exceeding the bitmap limit
    BitMap_setBit(bitmap,  bit, value);
    // binary recursion on the children downwards
    update_child(bitmap,  bit * 2 + 1, value);  // left
    update_child(bitmap,  bit * 2 + 2, value);  // right
  }
}
