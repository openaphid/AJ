
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
 * Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
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
#include "Operations.h"

#include "Error.h"
#include "AJObject.h"
#include "AJString.h"
#include <math.h>
#include <stdio.h>
#include <wtf/MathExtras.h>

namespace AJ {

bool AJValue::equalSlowCase(ExecState* exec, AJValue v1, AJValue v2)
{
    return equalSlowCaseInline(exec, v1, v2);
}

bool AJValue::strictEqualSlowCase(ExecState* exec, AJValue v1, AJValue v2)
{
    return strictEqualSlowCaseInline(exec, v1, v2);
}

NEVER_INLINE AJValue jsAddSlowCase(CallFrame* callFrame, AJValue v1, AJValue v2)
{
    // exception for the Date exception in defaultValue()
    AJValue p1 = v1.toPrimitive(callFrame);
    AJValue p2 = v2.toPrimitive(callFrame);

    if (p1.isString()) {
        return p2.isString()
            ? jsString(callFrame, asString(p1), asString(p2))
            : jsString(callFrame, asString(p1), p2.toString(callFrame));
    }
    if (p2.isString())
        return jsString(callFrame, p1.toString(callFrame), asString(p2));

    return jsNumber(callFrame, p1.toNumber(callFrame) + p2.toNumber(callFrame));
}

AJValue jsTypeStringForValue(CallFrame* callFrame, AJValue v)
{
    if (v.isUndefined())
        return jsNontrivialString(callFrame, "undefined");
    if (v.isBoolean())
        return jsNontrivialString(callFrame, "boolean");
    if (v.isNumber())
        return jsNontrivialString(callFrame, "number");
    if (v.isString())
        return jsNontrivialString(callFrame, "string");
    if (v.isObject()) {
        // Return "undefined" for objects that should be treated
        // as null when doing comparisons.
        if (asObject(v)->structure()->typeInfo().masqueradesAsUndefined())
            return jsNontrivialString(callFrame, "undefined");
        CallData callData;
        if (asObject(v)->getCallData(callData) != CallTypeNone)
            return jsNontrivialString(callFrame, "function");
    }
    return jsNontrivialString(callFrame, "object");
}

bool jsIsObjectType(AJValue v)
{
    if (!v.isCell())
        return v.isNull();

    AJType type = asCell(v)->structure()->typeInfo().type();
    if (type == NumberType || type == StringType)
        return false;
    if (type == ObjectType) {
        if (asObject(v)->structure()->typeInfo().masqueradesAsUndefined())
            return false;
        CallData callData;
        if (asObject(v)->getCallData(callData) != CallTypeNone)
            return false;
    }
    return true;
}

bool jsIsFunctionType(AJValue v)
{
    if (v.isObject()) {
        CallData callData;
        if (asObject(v)->getCallData(callData) != CallTypeNone)
            return true;
    }
    return false;
}

} // namespace AJ
