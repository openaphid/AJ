
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
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003, 2007 Apple Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "Completion.h"

#include "CallFrame.h"
#include "AJGlobalObject.h"
#include "AJLock.h"
#include "Interpreter.h"
#include "Parser.h"
#include "Debugger.h"
#include "ATFThreadData.h"
#include <stdio.h>

namespace AJ {

Completion checkSyntax(ExecState* exec, const SourceCode& source)
{
    AJLock lock(exec);
    ASSERT(exec->globalData().identifierTable == wtfThreadData().currentIdentifierTable());

    RefPtr<ProgramExecutable> program = ProgramExecutable::create(exec, source);
    AJObject* error = program->checkSyntax(exec);
    if (error)
        return Completion(Throw, error);

    return Completion(Normal);
}

Completion evaluate(ExecState* exec, ScopeChain& scopeChain, const SourceCode& source, AJValue thisValue)
{
    AJLock lock(exec);
    ASSERT(exec->globalData().identifierTable == wtfThreadData().currentIdentifierTable());

    RefPtr<ProgramExecutable> program = ProgramExecutable::create(exec, source);
    AJObject* error = program->compile(exec, scopeChain.node());
    if (error)
        return Completion(Throw, error);

    AJObject* thisObj = (!thisValue || thisValue.isUndefinedOrNull()) ? exec->dynamicGlobalObject() : thisValue.toObject(exec);

    AJValue exception;
    AJValue result = exec->interpreter()->execute(program.get(), exec, scopeChain.node(), thisObj, &exception);

    if (exception) {
        ComplType exceptionType = Throw;
        if (exception.isObject())
            exceptionType = asObject(exception)->exceptionType();
        return Completion(exceptionType, exception);
    }
    return Completion(Normal, result);
}

} // namespace AJ
