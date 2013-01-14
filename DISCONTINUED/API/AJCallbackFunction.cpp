
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
 * Copyright (C) 2006, 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "AJCallbackFunction.h"

#include "APIShims.h"
#include "APICast.h"
#include "CodeBlock.h"
#include "AJFunction.h"
#include "FunctionPrototype.h"
#include <runtime/AJGlobalObject.h>
#include <runtime/AJLock.h>
#include <wtf/Vector.h>

namespace AJ {

ASSERT_CLASS_FITS_IN_CELL(AJCallbackFunction);

const ClassInfo AJCallbackFunction::info = { "CallbackFunction", &InternalFunction::info, 0, 0 };

AJCallbackFunction::AJCallbackFunction(ExecState* exec, AJObjectCallAsFunctionCallback callback, const Identifier& name)
    : InternalFunction(&exec->globalData(), exec->lexicalGlobalObject()->callbackFunctionStructure(), name)
    , m_callback(callback)
{
}

AJValue AJCallbackFunction::call(ExecState* exec, AJObject* functionObject, AJValue thisValue, const ArgList& args)
{
    AJContextRef execRef = toRef(exec);
    AJObjectRef functionRef = toRef(functionObject);
    AJObjectRef thisObjRef = toRef(thisValue.toThisObject(exec));

    int argumentCount = static_cast<int>(args.size());
    Vector<AJValueRef, 16> arguments(argumentCount);
    for (int i = 0; i < argumentCount; i++)
        arguments[i] = toRef(exec, args.at(i));

    AJValueRef exception = 0;
    AJValueRef result;
    {
        APICallbackShim callbackShim(exec);
        result = static_cast<AJCallbackFunction*>(functionObject)->m_callback(execRef, functionRef, thisObjRef, argumentCount, arguments.data(), &exception);
    }
    if (exception)
        exec->setException(toJS(exec, exception));

    return toJS(exec, result);
}

CallType AJCallbackFunction::getCallData(CallData& callData)
{
    callData.native.function = call;
    return CallTypeHost;
}

} // namespace AJ
