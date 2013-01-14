
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
 *  Copyright (C) 2009 Torch Mobile, Inc.
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
#include "RegExpConstructor.h"

#include "ArrayPrototype.h"
#include "Error.h"
#include "AJArray.h"
#include "AJFunction.h"
#include "AJString.h"
#include "Lookup.h"
#include "ObjectPrototype.h"
#include "RegExpMatchesArray.h"
#include "RegExpObject.h"
#include "RegExpPrototype.h"
#include "RegExp.h"
#include "RegExpCache.h"

namespace AJ {

static AJValue regExpConstructorInput(ExecState*, AJValue, const Identifier&);
static AJValue regExpConstructorMultiline(ExecState*, AJValue, const Identifier&);
static AJValue regExpConstructorLastMatch(ExecState*, AJValue, const Identifier&);
static AJValue regExpConstructorLastParen(ExecState*, AJValue, const Identifier&);
static AJValue regExpConstructorLeftContext(ExecState*, AJValue, const Identifier&);
static AJValue regExpConstructorRightContext(ExecState*, AJValue, const Identifier&);
static AJValue regExpConstructorDollar1(ExecState*, AJValue, const Identifier&);
static AJValue regExpConstructorDollar2(ExecState*, AJValue, const Identifier&);
static AJValue regExpConstructorDollar3(ExecState*, AJValue, const Identifier&);
static AJValue regExpConstructorDollar4(ExecState*, AJValue, const Identifier&);
static AJValue regExpConstructorDollar5(ExecState*, AJValue, const Identifier&);
static AJValue regExpConstructorDollar6(ExecState*, AJValue, const Identifier&);
static AJValue regExpConstructorDollar7(ExecState*, AJValue, const Identifier&);
static AJValue regExpConstructorDollar8(ExecState*, AJValue, const Identifier&);
static AJValue regExpConstructorDollar9(ExecState*, AJValue, const Identifier&);

static void setRegExpConstructorInput(ExecState*, AJObject*, AJValue);
static void setRegExpConstructorMultiline(ExecState*, AJObject*, AJValue);

} // namespace AJ

#include "RegExpConstructor.lut.h"

namespace AJ {

ASSERT_CLASS_FITS_IN_CELL(RegExpConstructor);

const ClassInfo RegExpConstructor::info = { "Function", &InternalFunction::info, 0, ExecState::regExpConstructorTable };

/* Source for RegExpConstructor.lut.h
@begin regExpConstructorTable
    input           regExpConstructorInput          None
    $_              regExpConstructorInput          DontEnum
    multiline       regExpConstructorMultiline      None
    $*              regExpConstructorMultiline      DontEnum
    lastMatch       regExpConstructorLastMatch      DontDelete|ReadOnly
    $&              regExpConstructorLastMatch      DontDelete|ReadOnly|DontEnum
    lastParen       regExpConstructorLastParen      DontDelete|ReadOnly
    $+              regExpConstructorLastParen      DontDelete|ReadOnly|DontEnum
    leftContext     regExpConstructorLeftContext    DontDelete|ReadOnly
    $`              regExpConstructorLeftContext    DontDelete|ReadOnly|DontEnum
    rightContext    regExpConstructorRightContext   DontDelete|ReadOnly
    $'              regExpConstructorRightContext   DontDelete|ReadOnly|DontEnum
    $1              regExpConstructorDollar1        DontDelete|ReadOnly
    $2              regExpConstructorDollar2        DontDelete|ReadOnly
    $3              regExpConstructorDollar3        DontDelete|ReadOnly
    $4              regExpConstructorDollar4        DontDelete|ReadOnly
    $5              regExpConstructorDollar5        DontDelete|ReadOnly
    $6              regExpConstructorDollar6        DontDelete|ReadOnly
    $7              regExpConstructorDollar7        DontDelete|ReadOnly
    $8              regExpConstructorDollar8        DontDelete|ReadOnly
    $9              regExpConstructorDollar9        DontDelete|ReadOnly
@end
*/

RegExpConstructor::RegExpConstructor(ExecState* exec, NonNullPassRefPtr<Structure> structure, RegExpPrototype* regExpPrototype)
    : InternalFunction(&exec->globalData(), structure, Identifier(exec, "RegExp"))
    , d(new RegExpConstructorPrivate)
{
    // ECMA 15.10.5.1 RegExp.prototype
    putDirectWithoutTransition(exec->propertyNames().prototype, regExpPrototype, DontEnum | DontDelete | ReadOnly);

    // no. of arguments for constructor
    putDirectWithoutTransition(exec->propertyNames().length, jsNumber(exec, 2), ReadOnly | DontDelete | DontEnum);
}

RegExpMatchesArray::RegExpMatchesArray(ExecState* exec, RegExpConstructorPrivate* data)
    : AJArray(exec->lexicalGlobalObject()->regExpMatchesArrayStructure(), data->lastNumSubPatterns + 1)
{
    RegExpConstructorPrivate* d = new RegExpConstructorPrivate;
    d->input = data->lastInput;
    d->lastInput = data->lastInput;
    d->lastNumSubPatterns = data->lastNumSubPatterns;
    unsigned offsetVectorSize = (data->lastNumSubPatterns + 1) * 2; // only copying the result part of the vector
    d->lastOvector().resize(offsetVectorSize);
    memcpy(d->lastOvector().data(), data->lastOvector().data(), offsetVectorSize * sizeof(int));
    // d->multiline is not needed, and remains uninitialized

    setSubclassData(d);
}

RegExpMatchesArray::~RegExpMatchesArray()
{
    delete static_cast<RegExpConstructorPrivate*>(subclassData());
}

void RegExpMatchesArray::fillArrayInstance(ExecState* exec)
{
    RegExpConstructorPrivate* d = static_cast<RegExpConstructorPrivate*>(subclassData());
    ASSERT(d);

    unsigned lastNumSubpatterns = d->lastNumSubPatterns;

    for (unsigned i = 0; i <= lastNumSubpatterns; ++i) {
        int start = d->lastOvector()[2 * i];
        if (start >= 0)
            AJArray::put(exec, i, jsSubstring(exec, d->lastInput, start, d->lastOvector()[2 * i + 1] - start));
        else
            AJArray::put(exec, i, jsUndefined());
    }

    PutPropertySlot slot;
    AJArray::put(exec, exec->propertyNames().index, jsNumber(exec, d->lastOvector()[0]), slot);
    AJArray::put(exec, exec->propertyNames().input, jsString(exec, d->input), slot);

    delete d;
    setSubclassData(0);
}

AJObject* RegExpConstructor::arrayOfMatches(ExecState* exec) const
{
    return new (exec) RegExpMatchesArray(exec, d.get());
}

AJValue RegExpConstructor::getBackref(ExecState* exec, unsigned i) const
{
    if (!d->lastOvector().isEmpty() && i <= d->lastNumSubPatterns) {
        int start = d->lastOvector()[2 * i];
        if (start >= 0)
            return jsSubstring(exec, d->lastInput, start, d->lastOvector()[2 * i + 1] - start);
    }
    return jsEmptyString(exec);
}

AJValue RegExpConstructor::getLastParen(ExecState* exec) const
{
    unsigned i = d->lastNumSubPatterns;
    if (i > 0) {
        ASSERT(!d->lastOvector().isEmpty());
        int start = d->lastOvector()[2 * i];
        if (start >= 0)
            return jsSubstring(exec, d->lastInput, start, d->lastOvector()[2 * i + 1] - start);
    }
    return jsEmptyString(exec);
}

AJValue RegExpConstructor::getLeftContext(ExecState* exec) const
{
    if (!d->lastOvector().isEmpty())
        return jsSubstring(exec, d->lastInput, 0, d->lastOvector()[0]);
    return jsEmptyString(exec);
}

AJValue RegExpConstructor::getRightContext(ExecState* exec) const
{
    if (!d->lastOvector().isEmpty())
        return jsSubstring(exec, d->lastInput, d->lastOvector()[1], d->lastInput.size() - d->lastOvector()[1]);
    return jsEmptyString(exec);
}
    
bool RegExpConstructor::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<RegExpConstructor, InternalFunction>(exec, ExecState::regExpConstructorTable(exec), this, propertyName, slot);
}

bool RegExpConstructor::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    return getStaticValueDescriptor<RegExpConstructor, InternalFunction>(exec, ExecState::regExpConstructorTable(exec), this, propertyName, descriptor);
}

AJValue regExpConstructorDollar1(ExecState* exec, AJValue slotBase, const Identifier&)
{
    return asRegExpConstructor(slotBase)->getBackref(exec, 1);
}

AJValue regExpConstructorDollar2(ExecState* exec, AJValue slotBase, const Identifier&)
{
    return asRegExpConstructor(slotBase)->getBackref(exec, 2);
}

AJValue regExpConstructorDollar3(ExecState* exec, AJValue slotBase, const Identifier&)
{
    return asRegExpConstructor(slotBase)->getBackref(exec, 3);
}

AJValue regExpConstructorDollar4(ExecState* exec, AJValue slotBase, const Identifier&)
{
    return asRegExpConstructor(slotBase)->getBackref(exec, 4);
}

AJValue regExpConstructorDollar5(ExecState* exec, AJValue slotBase, const Identifier&)
{
    return asRegExpConstructor(slotBase)->getBackref(exec, 5);
}

AJValue regExpConstructorDollar6(ExecState* exec, AJValue slotBase, const Identifier&)
{
    return asRegExpConstructor(slotBase)->getBackref(exec, 6);
}

AJValue regExpConstructorDollar7(ExecState* exec, AJValue slotBase, const Identifier&)
{
    return asRegExpConstructor(slotBase)->getBackref(exec, 7);
}

AJValue regExpConstructorDollar8(ExecState* exec, AJValue slotBase, const Identifier&)
{
    return asRegExpConstructor(slotBase)->getBackref(exec, 8);
}

AJValue regExpConstructorDollar9(ExecState* exec, AJValue slotBase, const Identifier&)
{
    return asRegExpConstructor(slotBase)->getBackref(exec, 9);
}

AJValue regExpConstructorInput(ExecState* exec, AJValue slotBase, const Identifier&)
{
    return jsString(exec, asRegExpConstructor(slotBase)->input());
}

AJValue regExpConstructorMultiline(ExecState*, AJValue slotBase, const Identifier&)
{
    return jsBoolean(asRegExpConstructor(slotBase)->multiline());
}

AJValue regExpConstructorLastMatch(ExecState* exec, AJValue slotBase, const Identifier&)
{
    return asRegExpConstructor(slotBase)->getBackref(exec, 0);
}

AJValue regExpConstructorLastParen(ExecState* exec, AJValue slotBase, const Identifier&)
{
    return asRegExpConstructor(slotBase)->getLastParen(exec);
}

AJValue regExpConstructorLeftContext(ExecState* exec, AJValue slotBase, const Identifier&)
{
    return asRegExpConstructor(slotBase)->getLeftContext(exec);
}

AJValue regExpConstructorRightContext(ExecState* exec, AJValue slotBase, const Identifier&)
{
    return asRegExpConstructor(slotBase)->getRightContext(exec);
}

void RegExpConstructor::put(ExecState* exec, const Identifier& propertyName, AJValue value, PutPropertySlot& slot)
{
    lookupPut<RegExpConstructor, InternalFunction>(exec, propertyName, value, ExecState::regExpConstructorTable(exec), this, slot);
}

void setRegExpConstructorInput(ExecState* exec, AJObject* baseObject, AJValue value)
{
    asRegExpConstructor(baseObject)->setInput(value.toString(exec));
}

void setRegExpConstructorMultiline(ExecState* exec, AJObject* baseObject, AJValue value)
{
    asRegExpConstructor(baseObject)->setMultiline(value.toBoolean(exec));
}
  
// ECMA 15.10.4
AJObject* constructRegExp(ExecState* exec, const ArgList& args)
{
    AJValue arg0 = args.at(0);
    AJValue arg1 = args.at(1);

    if (arg0.inherits(&RegExpObject::info)) {
        if (!arg1.isUndefined())
            return throwError(exec, TypeError, "Cannot supply flags when constructing one RegExp from another.");
        return asObject(arg0);
    }

    UString pattern = arg0.isUndefined() ? UString("") : arg0.toString(exec);
    UString flags = arg1.isUndefined() ? UString("") : arg1.toString(exec);

    RefPtr<RegExp> regExp = exec->globalData().regExpCache()->lookupOrCreate(pattern, flags);
    if (!regExp->isValid())
        return throwError(exec, SyntaxError, makeString("Invalid regular expression: ", regExp->errorMessage()));
    return new (exec) RegExpObject(exec->lexicalGlobalObject()->regExpStructure(), regExp.release());
}

static AJObject* constructWithRegExpConstructor(ExecState* exec, AJObject*, const ArgList& args)
{
    return constructRegExp(exec, args);
}

ConstructType RegExpConstructor::getConstructData(ConstructData& constructData)
{
    constructData.native.function = constructWithRegExpConstructor;
    return ConstructTypeHost;
}

// ECMA 15.10.3
static AJValue JSC_HOST_CALL callRegExpConstructor(ExecState* exec, AJObject*, AJValue, const ArgList& args)
{
    return constructRegExp(exec, args);
}

CallType RegExpConstructor::getCallData(CallData& callData)
{
    callData.native.function = callRegExpConstructor;
    return CallTypeHost;
}

void RegExpConstructor::setInput(const UString& input)
{
    d->input = input;
}

const UString& RegExpConstructor::input() const
{
    // Can detect a distinct initial state that is invisible to AJ, by checking for null
    // state (since jsString turns null strings to empty strings).
    return d->input;
}

void RegExpConstructor::setMultiline(bool multiline)
{
    d->multiline = multiline;
}

bool RegExpConstructor::multiline() const
{
    return d->multiline;
}

} // namespace AJ
