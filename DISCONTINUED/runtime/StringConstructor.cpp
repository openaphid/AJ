
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
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
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
#include "StringConstructor.h"

#include "AJFunction.h"
#include "AJGlobalObject.h"
#include "PrototypeFunction.h"
#include "StringPrototype.h"

namespace AJ {

static NEVER_INLINE AJValue stringFromCharCodeSlowCase(ExecState* exec, const ArgList& args)
{
    unsigned length = args.size();
    UChar* buf;
    PassRefPtr<UStringImpl> impl = UStringImpl::createUninitialized(length, buf);
    for (unsigned i = 0; i < length; ++i)
        buf[i] = static_cast<UChar>(args.at(i).toUInt32(exec));
    return jsString(exec, impl);
}

static AJValue JSC_HOST_CALL stringFromCharCode(ExecState* exec, AJObject*, AJValue, const ArgList& args)
{
    if (LIKELY(args.size() == 1))
        return jsSingleCharacterString(exec, args.at(0).toUInt32(exec));
    return stringFromCharCodeSlowCase(exec, args);
}

ASSERT_CLASS_FITS_IN_CELL(StringConstructor);

StringConstructor::StringConstructor(ExecState* exec, NonNullPassRefPtr<Structure> structure, Structure* prototypeFunctionStructure, StringPrototype* stringPrototype)
    : InternalFunction(&exec->globalData(), structure, Identifier(exec, stringPrototype->classInfo()->className))
{
    // ECMA 15.5.3.1 String.prototype
    putDirectWithoutTransition(exec->propertyNames().prototype, stringPrototype, ReadOnly | DontEnum | DontDelete);

    // ECMA 15.5.3.2 fromCharCode()
#if ENABLE(JIT) && ENABLE(JIT_OPTIMIZE_NATIVE_CALL)
    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 1, exec->propertyNames().fromCharCode, exec->globalData().getThunk(fromCharCodeThunkGenerator), stringFromCharCode), DontEnum);
#else
    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 1, exec->propertyNames().fromCharCode, stringFromCharCode), DontEnum);
#endif
    // no. of arguments for constructor
    putDirectWithoutTransition(exec->propertyNames().length, jsNumber(exec, 1), ReadOnly | DontEnum | DontDelete);
}

// ECMA 15.5.2
static AJObject* constructWithStringConstructor(ExecState* exec, AJObject*, const ArgList& args)
{
    if (args.isEmpty())
        return new (exec) StringObject(exec, exec->lexicalGlobalObject()->stringObjectStructure());
    return new (exec) StringObject(exec, exec->lexicalGlobalObject()->stringObjectStructure(), args.at(0).toString(exec));
}

ConstructType StringConstructor::getConstructData(ConstructData& constructData)
{
    constructData.native.function = constructWithStringConstructor;
    return ConstructTypeHost;
}

// ECMA 15.5.1
static AJValue JSC_HOST_CALL callStringConstructor(ExecState* exec, AJObject*, AJValue, const ArgList& args)
{
    if (args.isEmpty())
        return jsEmptyString(exec);
    return jsString(exec, args.at(0).toString(exec));
}

CallType StringConstructor::getCallData(CallData& callData)
{
    callData.native.function = callStringConstructor;
    return CallTypeHost;
}

} // namespace AJ
