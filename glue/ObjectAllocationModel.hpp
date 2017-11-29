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

#if !defined(OBJECTALLOCATIONMODEL_HPP_)
#define OBJECTALLOCATIONMODEL_HPP_

#include <AllocateInitialization.hpp>
#include <b9/context.inl.hpp>
#include <b9/objects.hpp>

namespace b9 {

class Initializer {
 public:
  virtual Cell* operator()(Cell* cell) = 0;
};

struct MapMapInitializer : public Initializer {
	virtual Cell* operator()(Cell* cell) override {
		return new(cell) MapMap();
	}
};

struct MapInitializer : public Initializer {
  virtual Cell* operator()(Cell* cell) override {
    return new (cell) Map(mapMap, kind);
  }
  MapMap* mapMap;
  MapKind kind;
};

struct EmptyObjectInitializer : public Initializer {
 public:
  virtual Cell* operator()(Cell* cell) override {
    return new (cell) Object(map);
  }
  EmptyObjectMap* map;
};

struct ObjectInitializer : public Initializer {
 public:
  virtual Cell* operator()(Cell* cell) override {
    return new (cell) Object(map);
  }
  ObjectMap* map;
};

class Allocation : public ::MM_AllocateInitialization {
 public:
  Cell* initializeObject(Cell* p) { return init_(p); }

  Allocation(Context& cx, Initializer& init, std::size_t size,
             uintptr_t flags = 0)
      : MM_AllocateInitialization(cx.omrGcThread(), 0, size, flags),
        init_(init) {}

 private:
  Initializer& init_;
};

}  // namespace b9

#endif /* OBJECTALLOCATIONMODEL_HPP_ */
