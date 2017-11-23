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

#include <b9/objects.hpp>

#include "AllocateInitialization.hpp"
// #include "ObjectModel.hpp"

namespace b9 {

class ObjectInitializer : public ::MM_AllocateInitialization {
 public:
  Cell *initializeObject(MM_EnvironmentBase *env, b9::Cell *cell) {
    return new (cell) b9::Cell(map_);
  }

  ObjectInitializer(MM_EnvironmentBase *env, b9::Map *map, uintptr_t size,
                    uintptr_t flags = 0)
      : MM_AllocateInitialization(env, 0 /*< category, unused */, size, flags),
        map_(map) {}

 private:
  Map *map_;
};

}  // namespace b9

#endif /* OBJECTALLOCATIONMODEL_HPP_ */

