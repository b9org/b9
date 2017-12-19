/*******************************************************************************
 * Copyright (c) 2014, 2016 IBM Corp. and others
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

#if !defined(OMR_OM_OBJECTDESCRIPTION_H_)
#define OMR_OM_OBJECTDESCRIPTION_H_

#include "omr.h"
#include "omrcomp.h"

namespace OMR {
namespace Om {

class Cell;

}  // namespace Om
}  // namespace OMR

/**
 * Object token definitions to be used by OMR components.
 */
typedef OMR::Om::Cell* languageobjectptr_t;
typedef OMR::Om::Cell* omrobjectptr_t;
typedef OMR::Om::Cell* omrarrayptr_t;

#if defined(OMR_GC_COMPRESSED_POINTERS)
typedef uint32_t fomrobject_t;
typedef uint32_t fomrarray_t;
#else
typedef uintptr_t fomrobject_t;
typedef uintptr_t fomrarray_t;
#endif

#endif /* OMR_OM_OBJECTDESCRIPTION_H_ */
