
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
#include "AJValueRef.h"

#include "APICast.h"
#include "APIShims.h"
#include "AJCallbackObject.h"

#include <runtime/AJGlobalObject.h>
#include <runtime/JSONObject.h>
#include <runtime/AJString.h>
#include <runtime/LiteralParser.h>
#include <runtime/Operations.h>
#include <runtime/Protect.h>
#include <runtime/UString.h>
#include <runtime/AJValue.h>

#include <wtf/Assertions.h>
#include <wtf/text/StringHash.h>

#include <algorithm> // for std::min

using namespace AJ;

::AJType AJValueGetType(AJContextRef ctx, AJValueRef value)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJValue jsValue = toJS(exec, value);

    if (jsValue.isUndefined())
        return kAJTypeUndefined;
    if (jsValue.isNull())
        return kAJTypeNull;
    if (jsValue.isBoolean())
        return kAJTypeBoolean;
    if (jsValue.isNumber())
        return kAJTypeNumber;
    if (jsValue.isString())
        return kAJTypeString;
    ASSERT(jsValue.isObject());
    return kAJTypeObject;
}

bool AJValueIsUndefined(AJContextRef ctx, AJValueRef value)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJValue jsValue = toJS(exec, value);
    return jsValue.isUndefined();
}

bool AJValueIsNull(AJContextRef ctx, AJValueRef value)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJValue jsValue = toJS(exec, value);
    return jsValue.isNull();
}

bool AJValueIsBoolean(AJContextRef ctx, AJValueRef value)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJValue jsValue = toJS(exec, value);
    return jsValue.isBoolean();
}

bool AJValueIsNumber(AJContextRef ctx, AJValueRef value)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJValue jsValue = toJS(exec, value);
    return jsValue.isNumber();
}

bool AJValueIsString(AJContextRef ctx, AJValueRef value)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJValue jsValue = toJS(exec, value);
    return jsValue.isString();
}

bool AJValueIsObject(AJContextRef ctx, AJValueRef value)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJValue jsValue = toJS(exec, value);
    return jsValue.isObject();
}

bool AJValueIsObjectOfClass(AJContextRef ctx, AJValueRef value, AJClassRef jsClass)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJValue jsValue = toJS(exec, value);
    
    if (AJObject* o = jsValue.getObject()) {
        if (o->inherits(&AJCallbackObject<AJGlobalObject>::info))
            return static_cast<AJCallbackObject<AJGlobalObject>*>(o)->inherits(jsClass);
        else if (o->inherits(&AJCallbackObject<AJObject>::info))
            return static_cast<AJCallbackObject<AJObject>*>(o)->inherits(jsClass);
    }
    return false;
}

bool AJValueIsEqual(AJContextRef ctx, AJValueRef a, AJValueRef b, AJValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJValue jsA = toJS(exec, a);
    AJValue jsB = toJS(exec, b);

    bool result = AJValue::equal(exec, jsA, jsB); // false if an exception is thrown
    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
    }
    return result;
}

bool AJValueIsStrictEqual(AJContextRef ctx, AJValueRef a, AJValueRef b)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJValue jsA = toJS(exec, a);
    AJValue jsB = toJS(exec, b);

    return AJValue::strictEqual(exec, jsA, jsB);
}

bool AJValueIsInstanceOfConstructor(AJContextRef ctx, AJValueRef value, AJObjectRef constructor, AJValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJValue jsValue = toJS(exec, value);

    AJObject* jsConstructor = toJS(constructor);
    if (!jsConstructor->structure()->typeInfo().implementsHasInstance())
        return false;
    bool result = jsConstructor->hasInstance(exec, jsValue, jsConstructor->get(exec, exec->propertyNames().prototype)); // false if an exception is thrown
    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
    }
    return result;
}

AJValueRef AJValueMakeUndefined(AJContextRef ctx)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    return toRef(exec, jsUndefined());
}

AJValueRef AJValueMakeNull(AJContextRef ctx)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    return toRef(exec, jsNull());
}

AJValueRef AJValueMakeBoolean(AJContextRef ctx, bool value)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    return toRef(exec, jsBoolean(value));
}

AJValueRef AJValueMakeNumber(AJContextRef ctx, double value)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    // Our AJValue representation relies on a standard bit pattern for NaN. NaNs
    // generated internally to AJCore naturally have that representation,
    // but an external NaN might not.
    if (isnan(value))
        value = NaN;

    return toRef(exec, jsNumber(exec, value));
}

AJValueRef AJValueMakeString(AJContextRef ctx, AJStringRef string)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    return toRef(exec, jsString(exec, string->ustring()));
}

AJValueRef AJValueMakeFromJSONString(AJContextRef ctx, AJStringRef string)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);
    LiteralParser parser(exec, string->ustring(), LiteralParser::StrictJSON);
    return toRef(exec, parser.tryLiteralParse());
}

AJStringRef AJValueCreateJSONString(AJContextRef ctx, AJValueRef apiValue, unsigned indent, AJValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);
    AJValue value = toJS(exec, apiValue);
    UString result = JSONStringify(exec, value, indent);
    if (exception)
        *exception = 0;
    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
        return 0;
    }
    return OpaqueAJString::create(result).releaseRef();
}

bool AJValueToBoolean(AJContextRef ctx, AJValueRef value)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJValue jsValue = toJS(exec, value);
    return jsValue.toBoolean(exec);
}

double AJValueToNumber(AJContextRef ctx, AJValueRef value, AJValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJValue jsValue = toJS(exec, value);

    double number = jsValue.toNumber(exec);
    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
        number = NaN;
    }
    return number;
}

AJStringRef AJValueToStringCopy(AJContextRef ctx, AJValueRef value, AJValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJValue jsValue = toJS(exec, value);
    
    RefPtr<OpaqueAJString> stringRef(OpaqueAJString::create(jsValue.toString(exec)));
    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
        stringRef.clear();
    }
    return stringRef.release().releaseRef();
}

AJObjectRef AJValueToObject(AJContextRef ctx, AJValueRef value, AJValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJValue jsValue = toJS(exec, value);
    
    AJObjectRef objectRef = toRef(jsValue.toObject(exec));
    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
        objectRef = 0;
    }
    return objectRef;
}    

void AJValueProtect(AJContextRef ctx, AJValueRef value)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJValue jsValue = toJSForGC(exec, value);
    gcProtect(jsValue);
}

void AJValueUnprotect(AJContextRef ctx, AJValueRef value)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJValue jsValue = toJSForGC(exec, value);
    gcUnprotect(jsValue);
}
