
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
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003, 2007, 2008 Apple Inc. All Rights Reserved.
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
#include "RegExpObject.h"

#include "Error.h"
#include "AJArray.h"
#include "AJGlobalObject.h"
#include "AJString.h"
#include "Lookup.h"
#include "RegExpConstructor.h"
#include "RegExpPrototype.h"

namespace AJ {

static AJValue regExpObjectGlobal(ExecState*, AJValue, const Identifier&);
static AJValue regExpObjectIgnoreCase(ExecState*, AJValue, const Identifier&);
static AJValue regExpObjectMultiline(ExecState*, AJValue, const Identifier&);
static AJValue regExpObjectSource(ExecState*, AJValue, const Identifier&);
static AJValue regExpObjectLastIndex(ExecState*, AJValue, const Identifier&);
static void setRegExpObjectLastIndex(ExecState*, AJObject*, AJValue);

} // namespace AJ

#include "RegExpObject.lut.h"

namespace AJ {

ASSERT_CLASS_FITS_IN_CELL(RegExpObject);

const ClassInfo RegExpObject::info = { "RegExp", 0, 0, ExecState::regExpTable };

/* Source for RegExpObject.lut.h
@begin regExpTable
    global        regExpObjectGlobal       DontDelete|ReadOnly|DontEnum
    ignoreCase    regExpObjectIgnoreCase   DontDelete|ReadOnly|DontEnum
    multiline     regExpObjectMultiline    DontDelete|ReadOnly|DontEnum
    source        regExpObjectSource       DontDelete|ReadOnly|DontEnum
    lastIndex     regExpObjectLastIndex    DontDelete|DontEnum
@end
*/

RegExpObject::RegExpObject(NonNullPassRefPtr<Structure> structure, NonNullPassRefPtr<RegExp> regExp)
    : AJObject(structure)
    , d(new RegExpObjectData(regExp, 0))
{
}

RegExpObject::~RegExpObject()
{
}

bool RegExpObject::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<RegExpObject, AJObject>(exec, ExecState::regExpTable(exec), this, propertyName, slot);
}

bool RegExpObject::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    return getStaticValueDescriptor<RegExpObject, AJObject>(exec, ExecState::regExpTable(exec), this, propertyName, descriptor);
}

AJValue regExpObjectGlobal(ExecState*, AJValue slotBase, const Identifier&)
{
    return jsBoolean(asRegExpObject(slotBase)->regExp()->global());
}

AJValue regExpObjectIgnoreCase(ExecState*, AJValue slotBase, const Identifier&)
{
    return jsBoolean(asRegExpObject(slotBase)->regExp()->ignoreCase());
}
 
AJValue regExpObjectMultiline(ExecState*, AJValue slotBase, const Identifier&)
{            
    return jsBoolean(asRegExpObject(slotBase)->regExp()->multiline());
}

AJValue regExpObjectSource(ExecState* exec, AJValue slotBase, const Identifier&)
{
    return jsString(exec, asRegExpObject(slotBase)->regExp()->pattern());
}

AJValue regExpObjectLastIndex(ExecState* exec, AJValue slotBase, const Identifier&)
{
    return jsNumber(exec, asRegExpObject(slotBase)->lastIndex());
}

void RegExpObject::put(ExecState* exec, const Identifier& propertyName, AJValue value, PutPropertySlot& slot)
{
    lookupPut<RegExpObject, AJObject>(exec, propertyName, value, ExecState::regExpTable(exec), this, slot);
}

void setRegExpObjectLastIndex(ExecState* exec, AJObject* baseObject, AJValue value)
{
    asRegExpObject(baseObject)->setLastIndex(value.toInteger(exec));
}

AJValue RegExpObject::test(ExecState* exec, const ArgList& args)
{
    return jsBoolean(match(exec, args));
}

AJValue RegExpObject::exec(ExecState* exec, const ArgList& args)
{
    if (match(exec, args))
        return exec->lexicalGlobalObject()->regExpConstructor()->arrayOfMatches(exec);
    return jsNull();
}

static AJValue JSC_HOST_CALL callRegExpObject(ExecState* exec, AJObject* function, AJValue, const ArgList& args)
{
    return asRegExpObject(function)->exec(exec, args);
}

CallType RegExpObject::getCallData(CallData& callData)
{
    callData.native.function = callRegExpObject;
    return CallTypeHost;
}

// Shared implementation used by test and exec.
bool RegExpObject::match(ExecState* exec, const ArgList& args)
{
    RegExpConstructor* regExpConstructor = exec->lexicalGlobalObject()->regExpConstructor();

    UString input = args.isEmpty() ? regExpConstructor->input() : args.at(0).toString(exec);
    if (input.isNull()) {
        throwError(exec, GeneralError, makeString("No input to ", toString(exec), "."));
        return false;
    }

    if (!regExp()->global()) {
        int position;
        int length;
        regExpConstructor->performMatch(d->regExp.get(), input, 0, position, length);
        return position >= 0;
    }

    if (d->lastIndex < 0 || d->lastIndex > input.size()) {
        d->lastIndex = 0;
        return false;
    }

    int position;
    int length = 0;
    regExpConstructor->performMatch(d->regExp.get(), input, static_cast<int>(d->lastIndex), position, length);
    if (position < 0) {
        d->lastIndex = 0;
        return false;
    }

    d->lastIndex = position + length;
    return true;
}

} // namespace AJ
