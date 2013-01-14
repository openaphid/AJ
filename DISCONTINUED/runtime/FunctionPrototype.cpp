
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
 *  Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
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
#include "FunctionPrototype.h"

#include "Arguments.h"
#include "AJArray.h"
#include "AJFunction.h"
#include "AJString.h"
#include "AJStringBuilder.h"
#include "Interpreter.h"
#include "Lexer.h"
#include "PrototypeFunction.h"

namespace AJ {

ASSERT_CLASS_FITS_IN_CELL(FunctionPrototype);

static AJValue JSC_HOST_CALL functionProtoFuncToString(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL functionProtoFuncApply(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL functionProtoFuncCall(ExecState*, AJObject*, AJValue, const ArgList&);

FunctionPrototype::FunctionPrototype(ExecState* exec, NonNullPassRefPtr<Structure> structure)
    : InternalFunction(&exec->globalData(), structure, exec->propertyNames().nullIdentifier)
{
    putDirectWithoutTransition(exec->propertyNames().length, jsNumber(exec, 0), DontDelete | ReadOnly | DontEnum);
}

void FunctionPrototype::addFunctionProperties(ExecState* exec, Structure* prototypeFunctionStructure, NativeFunctionWrapper** callFunction, NativeFunctionWrapper** applyFunction)
{
    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 0, exec->propertyNames().toString, functionProtoFuncToString), DontEnum);
    *applyFunction = new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 2, exec->propertyNames().apply, functionProtoFuncApply);
    putDirectFunctionWithoutTransition(exec, *applyFunction, DontEnum);
    *callFunction = new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 1, exec->propertyNames().call, functionProtoFuncCall);
    putDirectFunctionWithoutTransition(exec, *callFunction, DontEnum);
}

static AJValue JSC_HOST_CALL callFunctionPrototype(ExecState*, AJObject*, AJValue, const ArgList&)
{
    return jsUndefined();
}

// ECMA 15.3.4
CallType FunctionPrototype::getCallData(CallData& callData)
{
    callData.native.function = callFunctionPrototype;
    return CallTypeHost;
}

// Functions

// Compatibility hack for the Optimost AJ library. (See <rdar://problem/6595040>.)
static inline void insertSemicolonIfNeeded(UString& functionBody)
{
    ASSERT(functionBody[0] == '{');
    ASSERT(functionBody[functionBody.size() - 1] == '}');

    for (size_t i = functionBody.size() - 2; i > 0; --i) {
        UChar ch = functionBody[i];
        if (!Lexer::isWhiteSpace(ch) && !Lexer::isLineTerminator(ch)) {
            if (ch != ';' && ch != '}')
                functionBody = makeString(functionBody.substr(0, i + 1), ";", functionBody.substr(i + 1, functionBody.size() - (i + 1)));
            return;
        }
    }
}

AJValue JSC_HOST_CALL functionProtoFuncToString(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (thisValue.inherits(&AJFunction::info)) {
        AJFunction* function = asFunction(thisValue);
        if (!function->isHostFunction()) {
            FunctionExecutable* executable = function->jsExecutable();
            UString sourceString = executable->source().toString();
            insertSemicolonIfNeeded(sourceString);
            return jsMakeNontrivialString(exec, "function ", function->name(exec), "(", executable->paramString(), ") ", sourceString);
        }
    }

    if (thisValue.inherits(&InternalFunction::info)) {
        InternalFunction* function = asInternalFunction(thisValue);
        return jsMakeNontrivialString(exec, "function ", function->name(exec), "() {\n    [native code]\n}");
    }

    return throwError(exec, TypeError);
}

AJValue JSC_HOST_CALL functionProtoFuncApply(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    CallData callData;
    CallType callType = thisValue.getCallData(callData);
    if (callType == CallTypeNone)
        return throwError(exec, TypeError);

    AJValue array = args.at(1);

    MarkedArgumentBuffer applyArgs;
    if (!array.isUndefinedOrNull()) {
        if (!array.isObject())
            return throwError(exec, TypeError);
        if (asObject(array)->classInfo() == &Arguments::info)
            asArguments(array)->fillArgList(exec, applyArgs);
        else if (isAJArray(&exec->globalData(), array))
            asArray(array)->fillArgList(exec, applyArgs);
        else if (asObject(array)->inherits(&AJArray::info)) {
            unsigned length = asArray(array)->get(exec, exec->propertyNames().length).toUInt32(exec);
            for (unsigned i = 0; i < length; ++i)
                applyArgs.append(asArray(array)->get(exec, i));
        } else
            return throwError(exec, TypeError);
    }

    return call(exec, thisValue, callType, callData, args.at(0), applyArgs);
}

AJValue JSC_HOST_CALL functionProtoFuncCall(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    CallData callData;
    CallType callType = thisValue.getCallData(callData);
    if (callType == CallTypeNone)
        return throwError(exec, TypeError);

    ArgList callArgs;
    args.getSlice(1, callArgs);
    return call(exec, thisValue, callType, callData, args.at(0), callArgs);
}

} // namespace AJ
