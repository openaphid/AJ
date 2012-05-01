
/*
Copyright 2012 Aphid Mobile

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
 
   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/
/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003, 2008 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "config.h"
#include "ErrorPrototype.h"

#include "AJFunction.h"
#include "AJString.h"
#include "AJStringBuilder.h"
#include "ObjectPrototype.h"
#include "PrototypeFunction.h"
#include "UString.h"

namespace AJ {

ASSERT_CLASS_FITS_IN_CELL(ErrorPrototype);

static AJValue JSC_HOST_CALL errorProtoFuncToString(ExecState*, AJObject*, AJValue, const ArgList&);

// ECMA 15.9.4
ErrorPrototype::ErrorPrototype(ExecState* exec, NonNullPassRefPtr<Structure> structure, Structure* prototypeFunctionStructure)
    : ErrorInstance(structure)
{
    // The constructor will be added later in ErrorConstructor's constructor

    putDirectWithoutTransition(exec->propertyNames().name, jsNontrivialString(exec, "Error"), DontEnum);
    putDirectWithoutTransition(exec->propertyNames().message, jsNontrivialString(exec, "Unknown error"), DontEnum);

    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 0, exec->propertyNames().toString, errorProtoFuncToString), DontEnum);
}

AJValue JSC_HOST_CALL errorProtoFuncToString(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    AJObject* thisObj = thisValue.toThisObject(exec);
    AJValue name = thisObj->get(exec, exec->propertyNames().name);
    AJValue message = thisObj->get(exec, exec->propertyNames().message);

    // Mozilla-compatible format.

    if (!name.isUndefined()) {
        if (!message.isUndefined())
            return jsMakeNontrivialString(exec, name.toString(exec), ": ", message.toString(exec));
        return jsNontrivialString(exec, name.toString(exec));
    }
    if (!message.isUndefined())
        return jsMakeNontrivialString(exec, "Error: ", message.toString(exec));
    return jsNontrivialString(exec, "Error");
}

} // namespace AJ
