
#ifndef B9_HASH_INCL
#define B9_HASH_INCL

#include <cassert>
#include <cstdlib>

#include "base9.hpp"

namespace b9 {

typedef void *allocated_from_heap;

#define PAD_SIZE 8
typedef union _slots_ {
  struct heap_allocated_
      *pointer_slots[PAD_SIZE / sizeof(struct heap_allocated_ *)];
  int64_t int64_t_slots[PAD_SIZE / sizeof(int64_t)];
  int32_t int32_t_slots[PAD_SIZE / sizeof(int32_t)];
  int8_t int8_t_slots[PAD_SIZE / sizeof(int8_t)];
  char char_slots[PAD_SIZE / sizeof(char)];
} _slots_union_;

struct heap_allocated {
  int32_t size;
  int32_t type;  // 0 == no pointers, 1 == pointers
  _slots_union_ d;
};

typedef struct heap_allocated *pHeap;
#define heap_sizeofheader \
  (sizeof(struct heap_allocated) - sizeof(_slots_union_))
#define heap_allocate(context, bytesize) malloc(bytesize)

#define USE_C_STRING_AS_KEYS 0
#if USE_C_STRING_AS_KEYS
typedef const char *hashTableKey;
#define charStringToKey(k) k
#define keyToChar(k) k
#else
typedef pHeap hashTableKey;
#define charStringToKey(k) allocateString(k)
#define keyToChar(k) (char *)addressFirstSlot((pHeap)(k))
#endif

pHeap hashTable_allocate(int num_elements);
hashTableKey hashTable_get(ExecutionContext *context, pHeap hashTable,
                           hashTableKey key);
void *hashTable_put(ExecutionContext *context, pHeap hashTable,
                    hashTableKey key, hashTableKey value);

hashTableKey *addressFirstSlot(pHeap hashedTable);
pHeap allocateString(const char *s);

} // namespace b9

#endif
