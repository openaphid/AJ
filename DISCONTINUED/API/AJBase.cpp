
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
 * Copyright (C) 2006, 2007 Apple Inc. All rights reserved.
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
#include "AJBase.h"
#include "AJBasePrivate.h"

#include "APICast.h"
#include "APIShims.h"
#include "Completion.h"
#include "OpaqueAJString.h"
#include "SourceCode.h"
#include <interpreter/CallFrame.h>
#include <runtime/InitializeThreading.h>
#include <runtime/Completion.h>
#include <runtime/AJGlobalObject.h>
#include <runtime/AJLock.h>
#include <runtime/AJObject.h>
#include <wtf/text/StringHash.h>

using namespace AJ;

AJValueRef AJEvaluateScript(AJContextRef ctx, AJStringRef script, AJObjectRef thisObject, AJStringRef sourceURL, int startingLineNumber, AJValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJObject* jsThisObject = toJS(thisObject);

    // evaluate sets "this" to the global object if it is NULL
    AJGlobalObject* globalObject = exec->dynamicGlobalObject();
    SourceCode source = makeSource(script->ustring(), sourceURL->ustring(), startingLineNumber);
    Completion completion = evaluate(globalObject->globalExec(), globalObject->globalScopeChain(), source, jsThisObject);

    if (completion.complType() == Throw) {
        if (exception)
            *exception = toRef(exec, completion.value());
        return 0;
    }

    if (completion.value())
        return toRef(exec, completion.value());
    
    // happens, for example, when the only statement is an empty (';') statement
    return toRef(exec, jsUndefined());
}

bool AJCheckScriptSyntax(AJContextRef ctx, AJStringRef script, AJStringRef sourceURL, int startingLineNumber, AJValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    SourceCode source = makeSource(script->ustring(), sourceURL->ustring(), startingLineNumber);
    Completion completion = checkSyntax(exec->dynamicGlobalObject()->globalExec(), source);
    if (completion.complType() == Throw) {
        if (exception)
            *exception = toRef(exec, completion.value());
        return false;
    }
    
    return true;
}

void AJGarbageCollect(AJContextRef ctx)
{
    // We used to recommend passing NULL as an argument here, which caused the only heap to be collected.
    // As there is no longer a shared heap, the previously recommended usage became a no-op (but the GC
    // will happen when the context group is destroyed).
    // Because the function argument was originally ignored, some clients may pass their released context here,
    // in which case there is a risk of crashing if another thread performs GC on the same heap in between.
    if (!ctx)
        return;

    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec, false);

    AJGlobalData& globalData = exec->globalData();
    if (!globalData.heap.isBusy())
        globalData.heap.collectAllGarbage();

    // FIXME: Perhaps we should trigger a second mark and sweep
    // once the garbage collector is done if this is called when
    // the collector is busy.
}

void JSReportExtraMemoryCost(AJContextRef ctx, size_t size)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);
    exec->globalData().heap.reportExtraMemoryCost(size);
}
