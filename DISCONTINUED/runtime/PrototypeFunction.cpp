
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
 *  Copyright (C) 1999-2002 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
 *  Copyright (C) 2007 Cameron Zwarich (cwzwarich@uwaterloo.ca)
 *  Copyright (C) 2007 Maks Orlovich
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
#include "PrototypeFunction.h"

#include "AJGlobalObject.h"
#include <wtf/Assertions.h>

namespace AJ {

ASSERT_CLASS_FITS_IN_CELL(PrototypeFunction);

PrototypeFunction::PrototypeFunction(ExecState* exec, int length, const Identifier& name, NativeFunction function)
    : InternalFunction(&exec->globalData(), exec->lexicalGlobalObject()->prototypeFunctionStructure(), name)
    , m_function(function)
{
    ASSERT_ARG(function, function);
    putDirect(exec->propertyNames().length, jsNumber(exec, length), DontDelete | ReadOnly | DontEnum);
}

PrototypeFunction::PrototypeFunction(ExecState* exec, NonNullPassRefPtr<Structure> prototypeFunctionStructure, int length, const Identifier& name, NativeFunction function)
    : InternalFunction(&exec->globalData(), prototypeFunctionStructure, name)
    , m_function(function)
{
    ASSERT_ARG(function, function);
    putDirect(exec->propertyNames().length, jsNumber(exec, length), DontDelete | ReadOnly | DontEnum);
}
    
CallType PrototypeFunction::getCallData(CallData& callData)
{
    callData.native.function = m_function;
    return CallTypeHost;
}

} // namespace AJ
