#ifndef __RUBINIUS_HEAP__
#define __RUBINIUS_HEAP__ 1

typedef unsigned long address;

struct heap {
  int size;
  address address;
  address current;
  address last;
  address scan;
  address extended;
};

typedef struct heap* rheap;

rheap heap_new(int size);
int heap_deallocate(rheap h);
int heap_allocate_memory(rheap h);
int heap_allocate_extended(rheap h);
int heap_reset(rheap h);
int heap_contains_p(rheap h, address addr);
int heap_allocated_p(rheap h);
int heap_using_extended_p(rheap h);
address heap_allocate(rheap h, int size);
int heap_enough_space_p(rheap h, int size);
OBJECT heap_copy_object(rheap h, OBJECT obj);
OBJECT heap_next_object(rheap h);
OBJECT heap_fully_scanned_p(rheap h);
OBJECT heap_next_unscanned(rheap h);
int heap_enough_fields_p(rheap h, int fields);

#endif

