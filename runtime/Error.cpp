
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
 *  Copyright (C) 2003, 2004, 2005, 2006, 2008 Apple Inc. All rights reserved.
 *  Copyright (C) 2007 Eric Seidel (eric@webkit.org)
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
#include "Error.h"

#include "ConstructData.h"
#include "ErrorConstructor.h"
#include "AJFunction.h"
#include "AJGlobalObject.h"
#include "AJObject.h"
#include "AJString.h"
#include "NativeErrorConstructor.h"

namespace AJ {

const char* expressionBeginOffsetPropertyName = "expressionBeginOffset";
const char* expressionCaretOffsetPropertyName = "expressionCaretOffset";
const char* expressionEndOffsetPropertyName = "expressionEndOffset";

AJObject* Error::create(ExecState* exec, ErrorType type, const UString& message, int lineNumber, intptr_t sourceID, const UString& sourceURL)
{
    AJObject* constructor;
    const char* name;
    switch (type) {
        case EvalError:
            constructor = exec->lexicalGlobalObject()->evalErrorConstructor();
            name = "Evaluation error";
            break;
        case RangeError:
            constructor = exec->lexicalGlobalObject()->rangeErrorConstructor();
            name = "Range error";
            break;
        case ReferenceError:
            constructor = exec->lexicalGlobalObject()->referenceErrorConstructor();
            name = "Reference error";
            break;
        case SyntaxError:
            constructor = exec->lexicalGlobalObject()->syntaxErrorConstructor();
            name = "Syntax error";
            break;
        case TypeError:
            constructor = exec->lexicalGlobalObject()->typeErrorConstructor();
            name = "Type error";
            break;
        case URIError:
            constructor = exec->lexicalGlobalObject()->URIErrorConstructor();
            name = "URI error";
            break;
        default:
            constructor = exec->lexicalGlobalObject()->errorConstructor();
            name = "Error";
            break;
    }

    MarkedArgumentBuffer args;
    if (message.isEmpty())
        args.append(jsString(exec, name));
    else
        args.append(jsString(exec, message));
    ConstructData constructData;
    ConstructType constructType = constructor->getConstructData(constructData);
    AJObject* error = construct(exec, constructor, constructType, constructData, args);

    if (lineNumber != -1)
        error->putWithAttributes(exec, Identifier(exec, "line"), jsNumber(exec, lineNumber), ReadOnly | DontDelete);
    if (sourceID != -1)
        error->putWithAttributes(exec, Identifier(exec, "sourceId"), jsNumber(exec, sourceID), ReadOnly | DontDelete);
    if (!sourceURL.isNull())
        error->putWithAttributes(exec, Identifier(exec, "sourceURL"), jsString(exec, sourceURL), ReadOnly | DontDelete);

    return error;
}

AJObject* Error::create(ExecState* exec, ErrorType type, const char* message)
{
    return create(exec, type, message, -1, -1, UString());
}

AJObject* throwError(ExecState* exec, AJObject* error)
{
    exec->setException(error);
    return error;
}

AJObject* throwError(ExecState* exec, ErrorType type)
{
    AJObject* error = Error::create(exec, type, UString(), -1, -1, UString());
    exec->setException(error);
    return error;
}

AJObject* throwError(ExecState* exec, ErrorType type, const UString& message)
{
    AJObject* error = Error::create(exec, type, message, -1, -1, UString());
    exec->setException(error);
    return error;
}

AJObject* throwError(ExecState* exec, ErrorType type, const char* message)
{
    AJObject* error = Error::create(exec, type, message, -1, -1, UString());
    exec->setException(error);
    return error;
}

AJObject* throwError(ExecState* exec, ErrorType type, const UString& message, int line, intptr_t sourceID, const UString& sourceURL)
{
    AJObject* error = Error::create(exec, type, message, line, sourceID, sourceURL);
    exec->setException(error);
    return error;
}

} // namespace AJ
