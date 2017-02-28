
#include "b9.h"

#ifndef B9_HASH_INCL
#define B9_HASH_INCL

#include <cstdlib>

namespace b9 {
namespace om {
class Value {
public:
    static Value *toValue(void *);
    bool isObject();

    /* Implementation */

private:
    uint64_t flags_;
};

// Anything that can be GC'ed
class Cell {
public:
    uint32_t eyeCatcher_;
    uint32_t flags_;
    uint32_t size_;

protected:
    Cell() = default;
};


class Shape;

class Object : public Cell {
public:
    Shape * getShape() { return shape_; }

private:
    Shape *shape_;

};

/* class Symbol; */
typedef uintptr_t Symbol;

class SlotDescriptor {
private:
    Symbol name_;
};

class Shape final : public Object {
public:
    intptr_t getSlotCount ();
    Slot
private:
    SlotDescriptor slots_[0];

};

typedef uint8_t * Address;
typedef uintptr_t Bytes;

class Heap {
public:
    bool init() { return true; }
    Address allocate(Bytes size) {
        return static_cast<Address>(malloc(size));
    }
private:
};

class Allocator {
public:
    static Object *allocate();

private:
};

class MemoryContext {
public:
    MemoryContext() = default;

    bool init(Heap &heap) {
        heap_ = heap;
    }

    Object *allocate() {
    }

    Heap &heap_;

private:
};

class Runtime {
public:
    bool init(Heap &heap) {
        heap_ = heap;

        // Allocate first shape
        baseShape_ = Runtime::createBaseShape(&heap);
    }

private:
    Heap &heap_;
    Shape *baseShape;

    static void createBaseShape(Heap *heap) {
        Bytes size = sizeof(Shape);
        Shape *shape = heap->allocate(size);

        shape->
        return shape;
    }
};

}

} // namespace b9

typedef void* allocated_from_heap;

#define PAD_SIZE 8
typedef union _slots_ {
    struct heap_allocated_* pointer_slots[PAD_SIZE / sizeof(struct heap_allocated_*)];
    int64_t int64_t_slots[PAD_SIZE / sizeof(int64_t)];
    int32_t int32_t_slots[PAD_SIZE / sizeof(int32_t)];
    int8_t int8_t_slots[PAD_SIZE / sizeof(int8_t)];
    char char_slots[PAD_SIZE / sizeof(char)];
} _slots_union_;

struct heap_allocated {
    int32_t  size;
    int32_t  type;  // 0 == no pointers, 1 == pointers
    _slots_union_ d;
};

typedef struct heap_allocated* pHeap;
#define heap_sizeofheader (sizeof (struct heap_allocated ) - sizeof (_slots_union_) )
#define heap_allocate(context, bytesize) malloc(bytesize)

#define USE_C_STRING_AS_KEYS 0
#if USE_C_STRING_AS_KEYS
typedef const char * hashTableKey;
#define charStringToKey(k) k
#define keyToChar(k) k
#else
typedef pHeap hashTableKey;
#define charStringToKey(k) allocateString (k)
#define keyToChar(k) (char*) addressFirstSlot((pHeap)(k))
#endif

pHeap hashTable_allocate(int num_elements);
hashTableKey hashTable_get(ExecutionContext* context, pHeap hashTable, hashTableKey key);
void * hashTable_put(ExecutionContext* context, pHeap hashTable, hashTableKey key, hashTableKey value);

hashTableKey* addressFirstSlot(pHeap hashedTable);
pHeap allocateString(const char* s);

#endif







kkkzzzz
