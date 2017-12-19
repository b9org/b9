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

#include <OMR/Om/Context.inl.hpp>
#include <OMR/Om/Marker.hpp>
#include <OMR/Om/MemoryManager.inl.hpp>
#include <OMR/Om/Object.inl.hpp>
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
  manager.visitRoots(cx, marker);

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
    std::cout << "found root: " << p << std::endl;
    _markingScheme->markObject(env, p, true);
  }

  for (auto& fn : cx.userRoots()) {
    fn(cx, marker);
  }
}

uintptr_t MarkingDelegate::scanObject(MM_EnvironmentBase* env,
                                      omrobjectptr_t cell) {
  auto map = cell->map();
  _markingScheme->inlineMarkObjectNoCheck(env, map);

  if (map->kind() != MapKind::SLOT_MAP) {
    // The cell is a leaf-object, like a map or empty object.
    return 0;
  }

  auto slotMap = reinterpret_cast<SlotMap*>(map);
  auto object = reinterpret_cast<Object*>(cell);

  for (std::size_t i = 0; i < slotMap->index(); i++) {
    Value value = object->getAt(i);
    if (value.isPtr()) {
      _markingScheme->inlineMarkObjectNoCheck(env, map);
    }
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
