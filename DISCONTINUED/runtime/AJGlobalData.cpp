
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
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "AJGlobalData.h"

#include "ArgList.h"
#include "Collector.h"
#include "CollectorHeapIterator.h"
#include "CommonIdentifiers.h"
#include "FunctionConstructor.h"
#include "GetterSetter.h"
#include "Interpreter.h"
#include "JSActivation.h"
#include "AJAPIValueWrapper.h"
#include "AJArray.h"
#include "AJArrayArray.h"
#include "AJClassRef.h"
#include "AJFunction.h"
#include "AJLock.h"
#include "JSNotAnObject.h"
#include "AJPropertyNameIterator.h"
#include "AJStaticScopeObject.h"
#include "Lexer.h"
#include "Lookup.h"
#include "Nodes.h"
#include "Parser.h"
#include "RegExpCache.h"
#include <wtf/ATFThreadData.h>

#if ENABLE(JSC_MULTIPLE_THREADS)
#include <wtf/Threading.h>
#endif

#if PLATFORM(MAC)
#include "ProfilerServer.h"
#include <CoreFoundation/CoreFoundation.h>
#endif

using namespace ATF;

namespace AJ {

extern JSC_CONST_HASHTABLE HashTable arrayTable;
extern JSC_CONST_HASHTABLE HashTable jsonTable;
extern JSC_CONST_HASHTABLE HashTable dateTable;
extern JSC_CONST_HASHTABLE HashTable mathTable;
extern JSC_CONST_HASHTABLE HashTable numberTable;
extern JSC_CONST_HASHTABLE HashTable regExpTable;
extern JSC_CONST_HASHTABLE HashTable regExpConstructorTable;
extern JSC_CONST_HASHTABLE HashTable stringTable;

void* AJGlobalData::jsArrayVPtr;
void* AJGlobalData::jsByteArrayVPtr;
void* AJGlobalData::jsStringVPtr;
void* AJGlobalData::jsFunctionVPtr;

void AJGlobalData::storeVPtrs()
{
    CollectorCell cell;
    void* storage = &cell;

    COMPILE_ASSERT(sizeof(AJArray) <= sizeof(CollectorCell), sizeof_AJArray_must_be_less_than_CollectorCell);
    AJCell* jsArray = new (storage) AJArray(AJArray::createStructure(jsNull()));
    AJGlobalData::jsArrayVPtr = jsArray->vptr();
    jsArray->~AJCell();

    COMPILE_ASSERT(sizeof(AJArrayArray) <= sizeof(CollectorCell), sizeof_AJArrayArray_must_be_less_than_CollectorCell);
    AJCell* jsByteArray = new (storage) AJArrayArray(AJArrayArray::VPtrStealingHack);
    AJGlobalData::jsByteArrayVPtr = jsByteArray->vptr();
    jsByteArray->~AJCell();

    COMPILE_ASSERT(sizeof(AJString) <= sizeof(CollectorCell), sizeof_AJString_must_be_less_than_CollectorCell);
    AJCell* jsString = new (storage) AJString(AJString::VPtrStealingHack);
    AJGlobalData::jsStringVPtr = jsString->vptr();
    jsString->~AJCell();

    COMPILE_ASSERT(sizeof(AJFunction) <= sizeof(CollectorCell), sizeof_AJFunction_must_be_less_than_CollectorCell);
    AJCell* jsFunction = new (storage) AJFunction(AJFunction::createStructure(jsNull()));
    AJGlobalData::jsFunctionVPtr = jsFunction->vptr();
    jsFunction->~AJCell();
}

AJGlobalData::AJGlobalData(GlobalDataType globalDataType, ThreadStackType threadStackType)
    : globalDataType(globalDataType)
    , clientData(0)
    , arrayTable(fastNew<HashTable>(AJ::arrayTable))
    , dateTable(fastNew<HashTable>(AJ::dateTable))
    , jsonTable(fastNew<HashTable>(AJ::jsonTable))
    , mathTable(fastNew<HashTable>(AJ::mathTable))
    , numberTable(fastNew<HashTable>(AJ::numberTable))
    , regExpTable(fastNew<HashTable>(AJ::regExpTable))
    , regExpConstructorTable(fastNew<HashTable>(AJ::regExpConstructorTable))
    , stringTable(fastNew<HashTable>(AJ::stringTable))
    , activationStructure(JSActivation::createStructure(jsNull()))
    , interruptedExecutionErrorStructure(AJObject::createStructure(jsNull()))
    , terminatedExecutionErrorStructure(AJObject::createStructure(jsNull()))
    , staticScopeStructure(AJStaticScopeObject::createStructure(jsNull()))
    , stringStructure(AJString::createStructure(jsNull()))
    , notAnObjectErrorStubStructure(JSNotAnObjectErrorStub::createStructure(jsNull()))
    , notAnObjectStructure(JSNotAnObject::createStructure(jsNull()))
    , propertyNameIteratorStructure(AJPropertyNameIterator::createStructure(jsNull()))
    , getterSetterStructure(GetterSetter::createStructure(jsNull()))
    , apiWrapperStructure(AJAPIValueWrapper::createStructure(jsNull()))
    , dummyMarkableCellStructure(AJCell::createDummyStructure())
#if USE(JSVALUE32)
    , numberStructure(JSNumberCell::createStructure(jsNull()))
#endif
    , identifierTable(globalDataType == Default ? wtfThreadData().currentIdentifierTable() : createIdentifierTable())
    , propertyNames(new CommonIdentifiers(this))
    , emptyList(new MarkedArgumentBuffer)
    , lexer(new Lexer(this))
    , parser(new Parser)
    , interpreter(new Interpreter)
    , heap(this)
    , initializingLazyNumericCompareFunction(false)
    , head(0)
    , dynamicGlobalObject(0)
    , functionCodeBlockBeingReparsed(0)
    , firstStringifierToMark(0)
    , markStack(jsArrayVPtr)
    , cachedUTCOffset(NaN)
    , maxReentryDepth(threadStackType == ThreadStackTypeSmall ? MaxSmallThreadReentryDepth : MaxLargeThreadReentryDepth)
    , m_regExpCache(new RegExpCache(this))
#ifndef NDEBUG
    , exclusiveThread(0)
#endif
{
#if PLATFORM(MAC)
    startProfilerServerIfNeeded();
#endif
#if ENABLE(JIT) && ENABLE(INTERPRETER)
#if PLATFORM(CF)
    CFStringRef canUseJITKey = CFStringCreateWithCString(0 , "AJCoreUseJIT", kCFStringEncodingMacRoman);
    CFBooleanRef canUseJIT = (CFBooleanRef)CFPreferencesCopyAppValue(canUseJITKey, kCFPreferencesCurrentApplication);
    if (canUseJIT) {
        m_canUseJIT = kCFBooleanTrue == canUseJIT;
        CFRelease(canUseJIT);
    } else
        m_canUseJIT = !getenv("AJCoreUseJIT");
    CFRelease(canUseJITKey);
#elif OS(UNIX)
    m_canUseJIT = !getenv("AJCoreUseJIT");
#else
    m_canUseJIT = true;
#endif
#endif
#if ENABLE(JIT)
#if ENABLE(INTERPRETER)
    if (m_canUseJIT)
        m_canUseJIT = executableAllocator.isValid();
#endif
    jitStubs.set(new JITThunks(this));
#endif
}

AJGlobalData::~AJGlobalData()
{
    // By the time this is destroyed, heap.destroy() must already have been called.

    delete interpreter;
#ifndef NDEBUG
    // Zeroing out to make the behavior more predictable when someone attempts to use a deleted instance.
    interpreter = 0;
#endif

    arrayTable->deleteTable();
    dateTable->deleteTable();
    jsonTable->deleteTable();
    mathTable->deleteTable();
    numberTable->deleteTable();
    regExpTable->deleteTable();
    regExpConstructorTable->deleteTable();
    stringTable->deleteTable();

    fastDelete(const_cast<HashTable*>(arrayTable));
    fastDelete(const_cast<HashTable*>(dateTable));
    fastDelete(const_cast<HashTable*>(jsonTable));
    fastDelete(const_cast<HashTable*>(mathTable));
    fastDelete(const_cast<HashTable*>(numberTable));
    fastDelete(const_cast<HashTable*>(regExpTable));
    fastDelete(const_cast<HashTable*>(regExpConstructorTable));
    fastDelete(const_cast<HashTable*>(stringTable));

    delete parser;
    delete lexer;

    deleteAllValues(opaqueAJClassData);

    delete emptyList;

    delete propertyNames;
    if (globalDataType != Default)
        deleteIdentifierTable(identifierTable);

    delete clientData;
    delete m_regExpCache;
}

PassRefPtr<AJGlobalData> AJGlobalData::createContextGroup(ThreadStackType type)
{
    return adoptRef(new AJGlobalData(APIContextGroup, type));
}

PassRefPtr<AJGlobalData> AJGlobalData::create(ThreadStackType type)
{
    return adoptRef(new AJGlobalData(Default, type));
}

PassRefPtr<AJGlobalData> AJGlobalData::createLeaked(ThreadStackType type)
{
    Structure::startIgnoringLeaks();
    RefPtr<AJGlobalData> data = create(type);
    Structure::stopIgnoringLeaks();
    return data.release();
}

bool AJGlobalData::sharedInstanceExists()
{
    return sharedInstanceInternal();
}

AJGlobalData& AJGlobalData::sharedInstance()
{
    AJGlobalData*& instance = sharedInstanceInternal();
    if (!instance) {
        instance = new AJGlobalData(APIShared, ThreadStackTypeSmall);
#if ENABLE(JSC_MULTIPLE_THREADS)
        instance->makeUsableFromMultipleThreads();
#endif
    }
    return *instance;
}

AJGlobalData*& AJGlobalData::sharedInstanceInternal()
{
    ASSERT(AJLock::currentThreadIsHoldingLock());
    static AJGlobalData* sharedInstance;
    return sharedInstance;
}

// FIXME: We can also detect forms like v1 < v2 ? -1 : 0, reverse comparison, etc.
const Vector<Instruction>& AJGlobalData::numericCompareFunction(ExecState* exec)
{
    if (!lazyNumericCompareFunction.size() && !initializingLazyNumericCompareFunction) {
        initializingLazyNumericCompareFunction = true;
        RefPtr<FunctionExecutable> function = FunctionExecutable::fromGlobalCode(Identifier(exec, "numericCompare"), exec, 0, makeSource(UString("(function (v1, v2) { return v1 - v2; })")), 0, 0);
        lazyNumericCompareFunction = function->bytecode(exec, exec->scopeChain()).instructions();
        initializingLazyNumericCompareFunction = false;
    }

    return lazyNumericCompareFunction;
}

AJGlobalData::ClientData::~ClientData()
{
}

void AJGlobalData::resetDateCache()
{
    cachedUTCOffset = NaN;
    dstOffsetCache.reset();
    cachedDateString = UString();
    dateInstanceCache.reset();
}

void AJGlobalData::startSampling()
{
    interpreter->startSampling();
}

void AJGlobalData::stopSampling()
{
    interpreter->stopSampling();
}

void AJGlobalData::dumpSampleData(ExecState* exec)
{
    interpreter->dumpSampleData(exec);
}

void AJGlobalData::recompileAllAJFunctions()
{
    // If AJ is running, it's not safe to recompile, since we'll end
    // up throwing away code that is live on the stack.
    ASSERT(!dynamicGlobalObject);

    LiveObjectIterator it = heap.primaryHeapBegin();
    LiveObjectIterator heapEnd = heap.primaryHeapEnd();
    for ( ; it != heapEnd; ++it) {
        if ((*it)->inherits(&AJFunction::info)) {
            AJFunction* function = asFunction(*it);
            if (!function->executable()->isHostFunction())
                function->jsExecutable()->recompile();
        }
    }
}

} // namespace AJ
