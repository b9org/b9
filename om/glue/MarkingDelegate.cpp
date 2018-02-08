/*******************************************************************************
 * Copyright (c) 2017, 2017 IBM Corp. and others
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

#include "MarkingDelegate.hpp"

#include <OMR/Om/ArrayBuffer.hpp>
#include <OMR/Om/Context.inl.hpp>
#include <OMR/Om/Marker.hpp>
#include <OMR/Om/MemoryManager.inl.hpp>
#include <OMR/Om/Object.inl.hpp>
#include <OMR/Om/ObjectMap.inl.hpp>
#include <OMR/Om/TransitionSet.inl.hpp>
#include <OMR/Om/Traverse.hpp>

#include "EnvironmentBase.hpp"
#include "MarkingScheme.hpp"
#include "OMRVMThreadListIterator.hpp"
#include "omr.h"
#include "omrExampleVM.hpp"
#include "omrhashtable.h"

namespace OMR {
namespace Om {

void MarkingDelegate::scanRoots(MM_EnvironmentBase* env) {
  auto& cx = getContext(env);
  auto& manager = cx.manager();
  Marker marker(_markingScheme);
  manager.visit(cx, marker);

  OMR_VMThread* walkThread;
  GC_OMRVMThreadListIterator threadListIterator(env->getOmrVM());
  while ((walkThread = threadListIterator.nextOMRVMThread()) != NULL) {
    if (NULL != walkThread->_savedObject1) {
      _markingScheme->markObject(env, (omrobjectptr_t)walkThread->_savedObject1,
                                 true);
    }
    if (NULL != walkThread->_savedObject2) {
      _markingScheme->markObject(env, (omrobjectptr_t)walkThread->_savedObject2,
                                 true);
    }
  }

  const auto& roots = cx.stackRoots();
  for (const auto& p : roots) {
    std::cout << "NATIVE_STACK: " << p << std::endl;
    _markingScheme->markObject(env, p);
  }

  for (auto& fn : cx.userRoots()) {
    fn(cx, marker);
  }
}

inline void scanMap(Context& cx, Marker& marker, Map* map) {
  switch (map->kind()) {
    case Map::Kind::OBJECT_MAP:
      reinterpret_cast<ObjectMap*>(map)->visit(cx, marker);
      break;
    case Map::Kind::META_MAP:
      reinterpret_cast<MetaMap*>(map)->visit(cx, marker);
      break;
    case Map::Kind::ARRAY_BUFFER_MAP:
      reinterpret_cast<ArrayBufferMap*>(map)->visit(cx, marker);
      break;
    default:
      assert(0);
      break;
  }
}

inline void scanObj(Context& cx, Marker& marker, Object* object) {
  object->visit(cx, marker);
}

inline void scanArrayBuffer(Context& cx, Marker& marker, ArrayBuffer* array) {
  array->visit(cx, marker);
}

uintptr_t MarkingDelegate::scanObject(MM_EnvironmentBase* env,
                                      omrobjectptr_t cell) {
  std::cerr << "Scanning: " << cell << std::endl;

  Context& cx = getContext(env->getOmrVMThread());
  Marker marker(_markingScheme);

  switch (cell->map()->kind()) {
    case Map::Kind::META_MAP:
      scanMap(cx, marker, reinterpret_cast<Map*>(cell));
      break;
    case Map::Kind::OBJECT_MAP:
      scanObj(cx, marker, reinterpret_cast<Object*>(cell));
      break;
    case Map::Kind::ARRAY_BUFFER_MAP:
      scanArrayBuffer(cx, marker, reinterpret_cast<ArrayBuffer*>(cell));
      break;
    default:
      assert(0);
      break;
  }

  return 0;
};

void MarkingDelegate::masterCleanupAfterGC(MM_EnvironmentBase* env) {
#if 0
	OMRPORT_ACCESS_FROM_OMRVM(env->getOmrVM());
	J9HashTableState state;
	ObjectEntry *objEntry = NULL;
	OMR_VM_Example *omrVM = (OMR_VM_Example *)env->getOmrVM()->_language_vm;
	objEntry = (ObjectEntry *)hashTableStartDo(omrVM->objectTable, &state);
	while (objEntry != NULL) {
		if (!_markingScheme->isMarked(objEntry->objPtr)) {
			omrmem_free_memory((void *)objEntry->name);
			objEntry->name = NULL;
			hashTableDoRemove(&state);
		}
		objEntry = (ObjectEntry *)hashTableNextDo(&state);
	}
#endif
}

}  // namespace Om
}  // namespace OMR
