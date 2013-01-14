
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
#include "NativeErrorConstructor.h"

#include "ErrorInstance.h"
#include "AJFunction.h"
#include "AJString.h"
#include "NativeErrorPrototype.h"

namespace AJ {

ASSERT_CLASS_FITS_IN_CELL(NativeErrorConstructor);

const ClassInfo NativeErrorConstructor::info = { "Function", &InternalFunction::info, 0, 0 };

NativeErrorConstructor::NativeErrorConstructor(ExecState* exec, NonNullPassRefPtr<Structure> structure, NonNullPassRefPtr<Structure> prototypeStructure, const UString& nameAndMessage)
    : InternalFunction(&exec->globalData(), structure, Identifier(exec, nameAndMessage))
{
    NativeErrorPrototype* prototype = new (exec) NativeErrorPrototype(exec, prototypeStructure, nameAndMessage, this);

    putDirect(exec->propertyNames().length, jsNumber(exec, 1), DontDelete | ReadOnly | DontEnum); // ECMA 15.11.7.5
    putDirect(exec->propertyNames().prototype, prototype, DontDelete | ReadOnly | DontEnum);
    m_errorStructure = ErrorInstance::createStructure(prototype);
}


ErrorInstance* NativeErrorConstructor::construct(ExecState* exec, const ArgList& args)
{
    ErrorInstance* object = new (exec) ErrorInstance(m_errorStructure);
    if (!args.at(0).isUndefined())
        object->putDirect(exec->propertyNames().message, jsString(exec, args.at(0).toString(exec)));
    return object;
}

static AJObject* constructWithNativeErrorConstructor(ExecState* exec, AJObject* constructor, const ArgList& args)
{
    return static_cast<NativeErrorConstructor*>(constructor)->construct(exec, args);
}

ConstructType NativeErrorConstructor::getConstructData(ConstructData& constructData)
{
    constructData.native.function = constructWithNativeErrorConstructor;
    return ConstructTypeHost;
}
    
static AJValue JSC_HOST_CALL callNativeErrorConstructor(ExecState* exec, AJObject* constructor, AJValue, const ArgList& args)
{
    return static_cast<NativeErrorConstructor*>(constructor)->construct(exec, args);
}

CallType NativeErrorConstructor::getCallData(CallData& callData)
{
    callData.native.function = callNativeErrorConstructor;
    return CallTypeHost;
}

} // namespace AJ
