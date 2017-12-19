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

#if !defined(OMR_OM_ALLOCATION_HPP_)
#define OMR_OM_ALLOCATION_HPP_

#include <OMR/Om/Context.inl.hpp>
#include <OMR/Om/Map.hpp>
#include <OMR/Om/Object.hpp>

#include <AllocateInitialization.hpp>

namespace OMR {
namespace Om {

class Initializer {
 public:
  virtual Cell* operator()(Context& cx, Cell* cell) = 0;
};

struct MetaMapInitializer : public Initializer {
  virtual Cell* operator()(Context& cx, Cell* cell) override {
    return new (cell) MetaMap();
  }
};

/// Note: Must be allocated AFTER the MetaMap
struct EmptyObjectMapInitializer : public Initializer {
 public:
  virtual Cell* operator()(Context& cx, Cell* cell) override {
    auto map = cx.globals().metaMap;
    return new (cell) EmptyObjectMap(map);
  }
};

/// Note: Must be allocated AFTER the MetaMap.
struct SlotMapInitializer : public Initializer {
 public:
  SlotMapInitializer(Context& cx, Map* parent, Id slotId)
      : parent_(cx, parent), slotId_(slotId) {}

  virtual Cell* operator()(Context& cx, Cell* cell) override {
    if (parent_->kind() == MapKind::EMPTY_OBJECT_MAP) {
      auto parent = parent_.get<EmptyObjectMap>();
      return new (cell) SlotMap(parent, slotId_);
    } else {
      assert(parent_->kind() == MapKind::SLOT_MAP);
      auto parent = parent_.get<SlotMap>();
      return new (cell) SlotMap(parent, slotId_);
    }
  }

  RootRef<Map> parent_;
  Id slotId_;
};

struct EmptyObjectInitializer : public Initializer {
 public:
  virtual Cell* operator()(Context& cx, Cell* cell) override {
    auto map = cx.globals().emptyObjectMap;
    return new (cell) Object(map);
  }
};

struct ObjectInitializer : public Initializer {
 public:
  ObjectInitializer(Context& cx, Object* base) : base_(cx, base) {}

  virtual Cell* operator()(Context& cx, Cell* cell) override {
    return new (cell) Object(*base_.get());
  }

 private:
  RootRef<Object> base_;
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
