
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
#include "AJContextRef.h"
#include "AJContextRefPrivate.h"

#include "APICast.h"
#include "InitializeThreading.h"
#include "AJCallbackObject.h"
#include "AJClassRef.h"
#include "AJGlobalObject.h"
#include "AJObject.h"
#include <wtf/text/StringHash.h>

#if OS(DARWIN)
#include <mach-o/dyld.h>

static const int32_t webkitFirstVersionWithConcurrentGlobalContexts = 0x2100500; // 528.5.0
#endif

using namespace AJ;

AJContextGroupRef AJContextGroupCreate()
{
    initializeThreading();
    return toRef(AJGlobalData::createContextGroup(ThreadStackTypeSmall).releaseRef());
}

AJContextGroupRef AJContextGroupRetain(AJContextGroupRef group)
{
    toJS(group)->ref();
    return group;
}

void AJContextGroupRelease(AJContextGroupRef group)
{
    toJS(group)->deref();
}

AJGlobalContextRef AJGlobalContextCreate(AJClassRef globalObjectClass)
{
    initializeThreading();
#if OS(DARWIN)
    // When running on Tiger or Leopard, or if the application was linked before AJGlobalContextCreate was changed
    // to use a unique AJGlobalData, we use a shared one for compatibility.
#if !defined(BUILDING_ON_TIGER) && !defined(BUILDING_ON_LEOPARD)
    if (NSVersionOfLinkTimeLibrary("AJCore") <= webkitFirstVersionWithConcurrentGlobalContexts) {
#else
    {
#endif
        AJLock lock(LockForReal);
        return AJGlobalContextCreateInGroup(toRef(&AJGlobalData::sharedInstance()), globalObjectClass);
    }
#endif // OS(DARWIN)

    return AJGlobalContextCreateInGroup(0, globalObjectClass);
}

AJGlobalContextRef AJGlobalContextCreateInGroup(AJContextGroupRef group, AJClassRef globalObjectClass)
{
    initializeThreading();

    AJLock lock(LockForReal);
    RefPtr<AJGlobalData> globalData = group ? PassRefPtr<AJGlobalData>(toJS(group)) : AJGlobalData::createContextGroup(ThreadStackTypeSmall);

    APIEntryShim entryShim(globalData.get(), false);

#if ENABLE(JSC_MULTIPLE_THREADS)
    globalData->makeUsableFromMultipleThreads();
#endif

    if (!globalObjectClass) {
        AJGlobalObject* globalObject = new (globalData.get()) AJGlobalObject;
        return AJGlobalContextRetain(toGlobalRef(globalObject->globalExec()));
    }

    AJGlobalObject* globalObject = new (globalData.get()) AJCallbackObject<AJGlobalObject>(globalObjectClass);
    ExecState* exec = globalObject->globalExec();
    AJValue prototype = globalObjectClass->prototype(exec);
    if (!prototype)
        prototype = jsNull();
    globalObject->resetPrototype(prototype);
    return AJGlobalContextRetain(toGlobalRef(exec));
}

AJGlobalContextRef AJGlobalContextRetain(AJGlobalContextRef ctx)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJGlobalData& globalData = exec->globalData();
    gcProtect(exec->dynamicGlobalObject());
    globalData.ref();
    return ctx;
}

void AJGlobalContextRelease(AJGlobalContextRef ctx)
{
    ExecState* exec = toJS(ctx);
    AJLock lock(exec);

    AJGlobalData& globalData = exec->globalData();
    AJGlobalObject* dgo = exec->dynamicGlobalObject();
    IdentifierTable* savedIdentifierTable = wtfThreadData().setCurrentIdentifierTable(globalData.identifierTable);

    // One reference is held by AJGlobalObject, another added by AJGlobalContextRetain().
    bool releasingContextGroup = globalData.refCount() == 2;
    bool releasingGlobalObject = Heap::heap(dgo)->unprotect(dgo);
    // If this is the last reference to a global data, it should also
    // be the only remaining reference to the global object too!
    ASSERT(!releasingContextGroup || releasingGlobalObject);

    // An API 'AJGlobalContextRef' retains two things - a global object and a
    // global data (or context group, in API terminology).
    // * If this is the last reference to any contexts in the given context group,
    //   call destroy on the heap (the global data is being  freed).
    // * If this was the last reference to the global object, then unprotecting
    //   it may  release a lot of GC memory - run the garbage collector now.
    // * If there are more references remaining the the global object, then do nothing
    //   (specifically that is more protects, which we assume come from other AJGlobalContextRefs).
    if (releasingContextGroup)
        globalData.heap.destroy();
    else if (releasingGlobalObject)
        globalData.heap.collectAllGarbage();

    globalData.deref();

    wtfThreadData().setCurrentIdentifierTable(savedIdentifierTable);
}

AJObjectRef AJContextGetGlobalObject(AJContextRef ctx)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    // It is necessary to call toThisObject to get the wrapper object when used with WebCore.
    return toRef(exec->lexicalGlobalObject()->toThisObject(exec));
}

AJContextGroupRef AJContextGetGroup(AJContextRef ctx)
{
    ExecState* exec = toJS(ctx);
    return toRef(&exec->globalData());
}

AJGlobalContextRef AJContextGetGlobalContext(AJContextRef ctx)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    return toGlobalRef(exec->lexicalGlobalObject()->globalExec());
}
