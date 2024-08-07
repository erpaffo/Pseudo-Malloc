# Pseudo-Malloc

## Overview
This is a project for the "Operating Systems" course at La Sapienza University. This is the project track:
```text 
This is a malloc replacement
The system relies on mmap for the physical allocation of memory, but handles the requests in
2 ways:
- for small requests (< 1/4 of the page size) it uses a buddy allocator.
    Clearly, such a buddy allocator can manage at most page-size bytes
    For simplicity use a single buddy allocator, implemented with a bitmap
    that manages 1 MB of memory for these small allocations.

- for large request (>=1/4 of the page size) uses a mmap.
```

To execute the code run in the shell these commands:
```shell
make
./buddy_allocator_test
./pseudo_malloc_test
```