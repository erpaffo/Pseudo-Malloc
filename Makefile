CC=gcc
CCOPTS=--std=gnu99 -Wall -D_LIST_DEBUG_
AR=ar

OBJS=bit_map.o\
     buddy_allocator.o\
     pseudo_malloc.o

HEADERS=bit_map.h buddy_allocator.h pseudo_malloc.h

LIBS=libbuddy.a

BINS=buddy_allocator_test pseudo_malloc_test

.PHONY: clean all

all: $(LIBS) $(BINS)

%.o: %.c $(HEADERS)
	$(CC) $(CCOPTS) -c -o $@ $<

libbuddy.a: $(OBJS)
	$(AR) -rcs $@ $^
	$(RM) $(OBJS)

buddy_allocator_test: buddy_allocator_test.o $(LIBS)
	$(CC) $(CCOPTS) -o $@ $^ -lm

pseudo_malloc_test: pseudo_malloc_test.o $(LIBS)
	$(CC) $(CCOPTS) -o $@ $^ -lm

clean:
	rm -rf *.o *~ $(LIBS) $(BINS)
