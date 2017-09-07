
#ifndef B9OBJECT_HPP_
#define B9OBJECT_HPP_

#include "base9.hpp"

#include <stdlib.h>

namespace b9 {
namespace om {

#define OM_DEBUG_ASSERT(x) assert(x)
#define smi 0b0
#define ho 0b1
#define fail 0b11

class Address {
 public:
  Address(uint8_t *address) : address_(address) {}

  template <typename Type>
  // constexpr Type to();

 private:
  uint8_t *address_;
};

// template <typename Type>
// constexpr Type Address::to() {
//   return reinterpret_cast<Type>(address_);
// }

class Value {
 public:
  static Value *toValue(void *);
  bool isObject();

  /* Implementation */

 private:
  uint64_t flags_;
};

// Cell
// Anything that can be GC'ed

class Cell {
 public:
  uint32_t eyeCatcher_ = eyeCatcher;
  uint32_t flags_;
  uint32_t size_;

  static const uint32_t eyeCatcher = 0x8d8d8d8d;

 protected:
  Cell() = default;
};

// Allocator API
class MemoryContext;

class MallocAllocator {
 public:
  template <typename Type>
  static Type allocate(MemoryContext *context) {
    return static_cast<Type>(malloc(sizeof(Type)));
  }
  template <typename Type>
  static Type allocate(MemoryContext *context, size_t size) {
    return static_cast<Type>(size);
  }

 private:
  MallocAllocator() = delete;
};

using DefaultAllocator = MallocAllocator;

// Object

class Shape;

class ObjectHeader {
 public:
  Cell cell_;
  Shape *shape_;
};

class Object {
 public:
  template <typename Allocator = DefaultAllocator>
  Object(MemoryContext *memoryContext, Shape *shape) {
    objectHeader_ = Allocator::allocate(memoryContext, getSize());
    objectHeader_->shape_ = shape;
    objectHeader_->cell_.flags_ = 0x0;
    objectHeader_->cell_.size_ = getSize();
  }

  ~Object() {
    // Do nothing
  }

  Shape *getShape() { return objectHeader_->shape_; }
  size_t getSize() { return objectHeader_->cell_.size_; }

 private:
  ObjectHeader *objectHeader_;
};

/* class Symbol; */
typedef uintptr_t Symbol;

class SlotDescriptor {
 private:
  Symbol name_;
};

typedef uint64_t Slot;

class Shape final : public Object {
 public:
  intptr_t getSlotCount();
  Slot slot_;

 private:
  SlotDescriptor slots_[0];
};

typedef uintptr_t Bytes;

class Heap {
 public:
  bool init() { return true; }
  // Address allocate(Bytes size) {
  //    return static_cast<Address>(malloc(size));
  //}
 private:
};

class MemoryContext {
 public:
  MemoryContext() = default;

  bool init(Heap &heap) {
    heap_ = heap;
    return true;
  }

  Object *allocate() { return nullptr; }

  Heap &heap_;

 private:
};

class Runtime {
 public:
  bool init(Heap &heap) {
    heap_ = heap;

    // Allocate first shape
    baseShape_ = Runtime::createBaseShape(&heap);

    return true;
  }

 private:
  Heap &heap_;
  Shape *baseShape_;

  static Shape *createBaseShape(Heap *heap) {
    Bytes size = sizeof(Shape);
    Shape *shape;  // = heap->allocate(size);

    // shape->
    return shape;
  }
};
}

}  // namespace b9

#endif  // B9OBJECT_HPP_
