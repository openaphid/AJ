
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
 *  Copyright (C) 2008 Apple Inc. All rights reserved.
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
#include "ObjectPrototype.h"

#include "Error.h"
#include "AJFunction.h"
#include "AJString.h"
#include "AJStringBuilder.h"
#include "PrototypeFunction.h"

namespace AJ {

ASSERT_CLASS_FITS_IN_CELL(ObjectPrototype);

static AJValue JSC_HOST_CALL objectProtoFuncValueOf(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL objectProtoFuncHasOwnProperty(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL objectProtoFuncIsPrototypeOf(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL objectProtoFuncDefineGetter(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL objectProtoFuncDefineSetter(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL objectProtoFuncLookupGetter(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL objectProtoFuncLookupSetter(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL objectProtoFuncPropertyIsEnumerable(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL objectProtoFuncToLocaleString(ExecState*, AJObject*, AJValue, const ArgList&);

ObjectPrototype::ObjectPrototype(ExecState* exec, NonNullPassRefPtr<Structure> stucture, Structure* prototypeFunctionStructure)
    : AJObject(stucture)
    , m_hasNoPropertiesWithUInt32Names(true)
{
    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 0, exec->propertyNames().toString, objectProtoFuncToString), DontEnum);
    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 0, exec->propertyNames().toLocaleString, objectProtoFuncToLocaleString), DontEnum);
    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 0, exec->propertyNames().valueOf, objectProtoFuncValueOf), DontEnum);
    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 1, exec->propertyNames().hasOwnProperty, objectProtoFuncHasOwnProperty), DontEnum);
    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 1, exec->propertyNames().propertyIsEnumerable, objectProtoFuncPropertyIsEnumerable), DontEnum);
    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 1, exec->propertyNames().isPrototypeOf, objectProtoFuncIsPrototypeOf), DontEnum);

    // Mozilla extensions
    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 2, exec->propertyNames().__defineGetter__, objectProtoFuncDefineGetter), DontEnum);
    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 2, exec->propertyNames().__defineSetter__, objectProtoFuncDefineSetter), DontEnum);
    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 1, exec->propertyNames().__lookupGetter__, objectProtoFuncLookupGetter), DontEnum);
    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 1, exec->propertyNames().__lookupSetter__, objectProtoFuncLookupSetter), DontEnum);
}

void ObjectPrototype::put(ExecState* exec, const Identifier& propertyName, AJValue value, PutPropertySlot& slot)
{
    AJObject::put(exec, propertyName, value, slot);

    if (m_hasNoPropertiesWithUInt32Names) {
        bool isUInt32;
        propertyName.toStrictUInt32(&isUInt32);
        m_hasNoPropertiesWithUInt32Names = !isUInt32;
    }
}

bool ObjectPrototype::getOwnPropertySlot(ExecState* exec, unsigned propertyName, PropertySlot& slot)
{
    if (m_hasNoPropertiesWithUInt32Names)
        return false;
    return AJObject::getOwnPropertySlot(exec, propertyName, slot);
}

// ------------------------------ Functions --------------------------------

// ECMA 15.2.4.2, 15.2.4.4, 15.2.4.5, 15.2.4.7

AJValue JSC_HOST_CALL objectProtoFuncValueOf(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    return thisValue.toThisObject(exec);
}

AJValue JSC_HOST_CALL objectProtoFuncHasOwnProperty(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    return jsBoolean(thisValue.toThisObject(exec)->hasOwnProperty(exec, Identifier(exec, args.at(0).toString(exec))));
}

AJValue JSC_HOST_CALL objectProtoFuncIsPrototypeOf(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    AJObject* thisObj = thisValue.toThisObject(exec);

    if (!args.at(0).isObject())
        return jsBoolean(false);

    AJValue v = asObject(args.at(0))->prototype();

    while (true) {
        if (!v.isObject())
            return jsBoolean(false);
        if (v == thisObj)
            return jsBoolean(true);
        v = asObject(v)->prototype();
    }
}

AJValue JSC_HOST_CALL objectProtoFuncDefineGetter(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    CallData callData;
    if (args.at(1).getCallData(callData) == CallTypeNone)
        return throwError(exec, SyntaxError, "invalid getter usage");
    thisValue.toThisObject(exec)->defineGetter(exec, Identifier(exec, args.at(0).toString(exec)), asObject(args.at(1)));
    return jsUndefined();
}

AJValue JSC_HOST_CALL objectProtoFuncDefineSetter(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    CallData callData;
    if (args.at(1).getCallData(callData) == CallTypeNone)
        return throwError(exec, SyntaxError, "invalid setter usage");
    thisValue.toThisObject(exec)->defineSetter(exec, Identifier(exec, args.at(0).toString(exec)), asObject(args.at(1)));
    return jsUndefined();
}

AJValue JSC_HOST_CALL objectProtoFuncLookupGetter(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    return thisValue.toThisObject(exec)->lookupGetter(exec, Identifier(exec, args.at(0).toString(exec)));
}

AJValue JSC_HOST_CALL objectProtoFuncLookupSetter(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    return thisValue.toThisObject(exec)->lookupSetter(exec, Identifier(exec, args.at(0).toString(exec)));
}

AJValue JSC_HOST_CALL objectProtoFuncPropertyIsEnumerable(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    return jsBoolean(thisValue.toThisObject(exec)->propertyIsEnumerable(exec, Identifier(exec, args.at(0).toString(exec))));
}

AJValue JSC_HOST_CALL objectProtoFuncToLocaleString(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    return thisValue.toThisAJString(exec);
}

AJValue JSC_HOST_CALL objectProtoFuncToString(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    return jsMakeNontrivialString(exec, "[object ", thisValue.toThisObject(exec)->className(), "]");
}

} // namespace AJ
