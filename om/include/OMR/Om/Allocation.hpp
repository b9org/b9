/*******************************************************************************
 * Copyright (c) 2000, 2016 IBM Corp. and others
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at http://eclipse.org/legal/epl-2.0
 * or the Apache License, Version 2.0 which accompanies this distribution
 * and is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following Secondary
 * Licenses when the conditions for such availability set forth in the
 * Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
 * version 2 with the GNU Classpath Exception [1] and GNU General Public
 * License, version 2 with the OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] http://openjdk.java.net/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
 *******************************************************************************/

// TODO: Implement the Allocation Initializers

#if !defined(OMR_OM_ALLOCATION_HPP_)
#define OMR_OM_ALLOCATION_HPP_

#include <OMR/Om/Context.inl.hpp>
#include <OMR/Om/Map.hpp>
#include <OMR/Om/Object.hpp>

#include <AllocateInitialization.hpp>

///@file
/// These classes implement basic initialization of objects.
/// Just enough to pay GC tax and walk the objects.

namespace OMR {
namespace Om {

class Initializer {
 public:
  virtual Cell* operator()(Context& cx, Cell* cell) = 0;
};

struct MetaMapInitializer : public Initializer {
  virtual Cell* operator()(Context& cx, Cell* cell) override {
    auto m = reinterpret_cast<MetaMap*>(cell);
    m->baseMap().map(m);  // m describes its own shape.
    m->baseMap().kind(Map::Kind::META_MAP);
    return &m->baseCell();
  }
};

struct EmptyObjectMapInitializer : public Initializer {
 public:
  virtual Cell* operator()(Context& cx, Cell* cell) override {
    auto m = reinterpret_cast<EmptyObjectMap*>(cell);
    m->baseMap().map(cx.globals().metaMap);
    m->baseMap().kind(Map::Kind::EMPTY_OBJECT_MAP);
    return &m->baseCell();
  }
};

struct SlotMapInitializer : public Initializer {
  SlotMapInitializer(Handle<ObjectMap> parent, const SlotDescriptor& desc)
      : parent_(parent), desc_(desc) {}

  virtual Cell* operator()(Context& cx, Cell* cell) override {
    auto m = reinterpret_cast<SlotMap*>(cell);
    m->baseMap().map(cx.globals().metaMap);
    m->baseMap().kind(Map::Kind::SLOT_MAP);
    m->parent(parent_.get());
    m->slotDescriptor(desc_);
    return &m->baseCell();
  }

  Handle<ObjectMap> parent_;
  SlotDescriptor desc_;
};

struct ObjectInitializer : public Initializer {
  virtual Cell* operator()(Context& cx, Cell* cell) override {
    auto o = reinterpret_cast<Object*>(cell);
    o->map(map_);
    o->fixedSlotCount_ = 32;
    return &o->baseCell();
  }

  Handle<ObjectMap> map_;
};

class Allocation : public MM_AllocateInitialization {
 public:
  Cell* initializeObject(Context& cx, Cell* p) { return init_(cx, p); }

  Allocation(Context& cx, Initializer& init, std::size_t size,
             uintptr_t flags = 0)
      : MM_AllocateInitialization(cx.omrGcThread(), 0, size, flags),
        init_(init) {}

 private:
  Initializer& init_;
};

}  // namespace Om
}  // namespace OMR

#endif /* OMR_OM_ALLOCATION_HPP_ */
