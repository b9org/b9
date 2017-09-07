
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <time.h>
#include <unistd.h>

#include "base9.hpp"
#include "b9hash.hpp"

namespace b9 {

static unsigned long djb2hash(const char *str) {
  unsigned long hash = 5381;
  int c;
  while ((c = *str++)) hash = ((hash << 5) + hash) + c;
  return hash;
}

hashTableKey *addressFirstSlot(pHeap hashedTable) {
  return (hashTableKey *)&hashedTable->d.pointer_slots[0];
}

bool hashTableKeyEquals(hashTableKey k1, hashTableKey k2) {
  return !strcmp(keyToChar(k1), keyToChar(k2));
}

pHeap hashTable_allocate(int num_elements) {
  int grow = num_elements < 1024 ? 32 : 256;
  int allocateSize = (num_elements + grow) * sizeof(pHeap);
  pHeap newht = (pHeap)heap_allocate(context, allocateSize);
  memset(newht, 0, allocateSize);
  newht->size = allocateSize;
  newht->type = 1;
  return newht;
}

pHeap allocateString(const char *s) {
  int stringSize = strlen(s) + 1; /* +1 for NULL terminator */
  int allocSize = stringSize + heap_sizeofheader;
  pHeap heapString = (pHeap)heap_allocate(context, allocSize);
  heapString->size = allocSize;
  heapString->type = 0;
  char *dest = (char *)addressFirstSlot(heapString);
  memcpy(dest, s, stringSize);
  dest[stringSize] = 0; /* Null terminate the string */
  return heapString;
}

hashTableKey hashTable_get(ExecutionContext *context, pHeap hashTable,
                           hashTableKey key) {
  hashTableKey *kv_start = addressFirstSlot(hashTable);
  hashTableKey *kv_end = (hashTableKey *)(((char *)kv_start) + hashTable->size);
  int num_pairs = (kv_end - kv_start) / 2;
  int start = djb2hash(keyToChar(key)) % num_pairs;
  hashTableKey *kv_hashed_start = kv_start + (start * 2);
  for (kv_start = kv_hashed_start; kv_start < kv_end; kv_start += 2) {
    if (kv_start[0] == nullptr) {
      return nullptr;
    }
    if (hashTableKeyEquals(kv_start[0], key)) {
      return kv_start[1];
    }
  }
  for (kv_start = addressFirstSlot(hashTable); kv_start < kv_hashed_start;
       kv_start += 2) {
    if (kv_start[0] == nullptr) {
      return nullptr;
    }
    if (hashTableKeyEquals(kv_start[0], key)) {
      return kv_start[1];
    }
  }
  return nullptr;
}

void *hashTable_put(ExecutionContext *context, pHeap hashTable,
                    hashTableKey key, hashTableKey value) {
  hashTableKey *kv_start = addressFirstSlot(hashTable);
  hashTableKey *kv_end = (hashTableKey *)(((char *)kv_start) + hashTable->size);
  int num_elements = (kv_end - kv_start);
  int num_pairs = num_elements / 2;
  int start = djb2hash(keyToChar(key)) % num_pairs;
  hashTableKey *kv_hashed_start = kv_start + (start * 2);

  for (kv_start = kv_hashed_start; kv_start < kv_end; kv_start += 2) {
    if (kv_start[0] == nullptr || hashTableKeyEquals(kv_start[0], key)) {
      kv_start[0] = key;
      kv_start[1] = value;
      return hashTable;
    }
  }
  for (kv_start = addressFirstSlot(hashTable); kv_start < kv_hashed_start;
       kv_start += 2) {
    if (kv_start[0] == nullptr || hashTableKeyEquals(kv_start[0], key)) {
      kv_start[0] = key;
      kv_start[1] = value;
      return hashTable;
    }
  }
  pHeap newht = hashTable_allocate(num_elements);
  for (kv_start = addressFirstSlot(hashTable); kv_start < kv_end;
       kv_start += 2) {
    hashTable_put(context, newht, kv_start[0], kv_start[1]);
  }
  hashTable_put(context, newht, key, value);
  return newht;
}

} // namespace b9
