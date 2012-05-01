
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
 *  Copyright (C) 2008 Apple Inc. All rights reserved.
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
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
#include "Debugger.h"

#include "CollectorHeapIterator.h"
#include "Error.h"
#include "Interpreter.h"
#include "AJFunction.h"
#include "AJGlobalObject.h"
#include "Parser.h"
#include "Protect.h"

namespace AJ {

Debugger::~Debugger()
{
    HashSet<AJGlobalObject*>::iterator end = m_globalObjects.end();
    for (HashSet<AJGlobalObject*>::iterator it = m_globalObjects.begin(); it != end; ++it)
        (*it)->setDebugger(0);
}

void Debugger::attach(AJGlobalObject* globalObject)
{
    ASSERT(!globalObject->debugger());
    globalObject->setDebugger(this);
    m_globalObjects.add(globalObject);
}

void Debugger::detach(AJGlobalObject* globalObject)
{
    ASSERT(m_globalObjects.contains(globalObject));
    m_globalObjects.remove(globalObject);
    globalObject->setDebugger(0);
}

void Debugger::recompileAllAJFunctions(AJGlobalData* globalData)
{
    // If AJ is running, it's not safe to recompile, since we'll end
    // up throwing away code that is live on the stack.
    ASSERT(!globalData->dynamicGlobalObject);
    if (globalData->dynamicGlobalObject)
        return;

    typedef HashSet<FunctionExecutable*> FunctionExecutableSet;
    typedef HashMap<SourceProvider*, ExecState*> SourceProviderMap;

    FunctionExecutableSet functionExecutables;
    SourceProviderMap sourceProviders;

    LiveObjectIterator it = globalData->heap.primaryHeapBegin();
    LiveObjectIterator heapEnd = globalData->heap.primaryHeapEnd();
    for ( ; it != heapEnd; ++it) {
        if (!(*it)->inherits(&AJFunction::info))
            continue;

        AJFunction* function = asFunction(*it);
        if (function->executable()->isHostFunction())
            continue;

        FunctionExecutable* executable = function->jsExecutable();

        // Check if the function is already in the set - if so,
        // we've already retranslated it, nothing to do here.
        if (!functionExecutables.add(executable).second)
            continue;

        ExecState* exec = function->scope().globalObject()->AJGlobalObject::globalExec();
        executable->recompile();
        if (function->scope().globalObject()->debugger() == this)
            sourceProviders.add(executable->source().provider(), exec);
    }

    // Call sourceParsed() after reparsing all functions because it will execute
    // AJ in the inspector.
    SourceProviderMap::const_iterator end = sourceProviders.end();
    for (SourceProviderMap::const_iterator iter = sourceProviders.begin(); iter != end; ++iter)
        sourceParsed(iter->second, SourceCode(iter->first), -1, UString());
}

AJValue evaluateInGlobalCallFrame(const UString& script, AJValue& exception, AJGlobalObject* globalObject)
{
    CallFrame* globalCallFrame = globalObject->globalExec();

    RefPtr<EvalExecutable> eval = EvalExecutable::create(globalCallFrame, makeSource(script));
    AJObject* error = eval->compile(globalCallFrame, globalCallFrame->scopeChain());
    if (error)
        return error;

    return globalObject->globalData()->interpreter->execute(eval.get(), globalCallFrame, globalObject, globalCallFrame->scopeChain(), &exception);
}

} // namespace AJ
