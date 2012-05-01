
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
#include "BooleanConstructor.h"

#include "BooleanPrototype.h"
#include "AJGlobalObject.h"

namespace AJ {

ASSERT_CLASS_FITS_IN_CELL(BooleanConstructor);

BooleanConstructor::BooleanConstructor(ExecState* exec, NonNullPassRefPtr<Structure> structure, BooleanPrototype* booleanPrototype)
    : InternalFunction(&exec->globalData(), structure, Identifier(exec, booleanPrototype->classInfo()->className))
{
    putDirectWithoutTransition(exec->propertyNames().prototype, booleanPrototype, DontEnum | DontDelete | ReadOnly);

    // no. of arguments for constructor
    putDirectWithoutTransition(exec->propertyNames().length, jsNumber(exec, 1), ReadOnly | DontDelete | DontEnum);
}

// ECMA 15.6.2
AJObject* constructBoolean(ExecState* exec, const ArgList& args)
{
    BooleanObject* obj = new (exec) BooleanObject(exec->lexicalGlobalObject()->booleanObjectStructure());
    obj->setInternalValue(jsBoolean(args.at(0).toBoolean(exec)));
    return obj;
}

static AJObject* constructWithBooleanConstructor(ExecState* exec, AJObject*, const ArgList& args)
{
    return constructBoolean(exec, args);
}

ConstructType BooleanConstructor::getConstructData(ConstructData& constructData)
{
    constructData.native.function = constructWithBooleanConstructor;
    return ConstructTypeHost;
}

// ECMA 15.6.1
static AJValue JSC_HOST_CALL callBooleanConstructor(ExecState* exec, AJObject*, AJValue, const ArgList& args)
{
    return jsBoolean(args.at(0).toBoolean(exec));
}

CallType BooleanConstructor::getCallData(CallData& callData)
{
    callData.native.function = callBooleanConstructor;
    return CallTypeHost;
}

AJObject* constructBooleanFromImmediateBoolean(ExecState* exec, AJValue immediateBooleanValue)
{
    BooleanObject* obj = new (exec) BooleanObject(exec->lexicalGlobalObject()->booleanObjectStructure());
    obj->setInternalValue(immediateBooleanValue);
    return obj;
}

} // namespace AJ
