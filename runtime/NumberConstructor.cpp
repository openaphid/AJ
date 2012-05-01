
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
 *  Copyright (C) 1999-2000,2003 Harri Porten (porten@kde.org)
 *  Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 *
 */

#include "config.h"
#include "NumberConstructor.h"

#include "Lookup.h"
#include "NumberObject.h"
#include "NumberPrototype.h"

namespace AJ {

ASSERT_CLASS_FITS_IN_CELL(NumberConstructor);

static AJValue numberConstructorNaNValue(ExecState*, AJValue, const Identifier&);
static AJValue numberConstructorNegInfinity(ExecState*, AJValue, const Identifier&);
static AJValue numberConstructorPosInfinity(ExecState*, AJValue, const Identifier&);
static AJValue numberConstructorMaxValue(ExecState*, AJValue, const Identifier&);
static AJValue numberConstructorMinValue(ExecState*, AJValue, const Identifier&);

} // namespace AJ

#include "NumberConstructor.lut.h"

namespace AJ {

const ClassInfo NumberConstructor::info = { "Function", &InternalFunction::info, 0, ExecState::numberTable };

/* Source for NumberConstructor.lut.h
@begin numberTable
   NaN                   numberConstructorNaNValue       DontEnum|DontDelete|ReadOnly
   NEGATIVE_INFINITY     numberConstructorNegInfinity    DontEnum|DontDelete|ReadOnly
   POSITIVE_INFINITY     numberConstructorPosInfinity    DontEnum|DontDelete|ReadOnly
   MAX_VALUE             numberConstructorMaxValue       DontEnum|DontDelete|ReadOnly
   MIN_VALUE             numberConstructorMinValue       DontEnum|DontDelete|ReadOnly
@end
*/

NumberConstructor::NumberConstructor(ExecState* exec, NonNullPassRefPtr<Structure> structure, NumberPrototype* numberPrototype)
    : InternalFunction(&exec->globalData(), structure, Identifier(exec, numberPrototype->info.className))
{
    // Number.Prototype
    putDirectWithoutTransition(exec->propertyNames().prototype, numberPrototype, DontEnum | DontDelete | ReadOnly);

    // no. of arguments for constructor
    putDirectWithoutTransition(exec->propertyNames().length, jsNumber(exec, 1), ReadOnly | DontEnum | DontDelete);
}

bool NumberConstructor::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<NumberConstructor, InternalFunction>(exec, ExecState::numberTable(exec), this, propertyName, slot);
}

bool NumberConstructor::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    return getStaticValueDescriptor<NumberConstructor, InternalFunction>(exec, ExecState::numberTable(exec), this, propertyName, descriptor);
}

static AJValue numberConstructorNaNValue(ExecState* exec, AJValue, const Identifier&)
{
    return jsNaN(exec);
}

static AJValue numberConstructorNegInfinity(ExecState* exec, AJValue, const Identifier&)
{
    return jsNumber(exec, -Inf);
}

static AJValue numberConstructorPosInfinity(ExecState* exec, AJValue, const Identifier&)
{
    return jsNumber(exec, Inf);
}

static AJValue numberConstructorMaxValue(ExecState* exec, AJValue, const Identifier&)
{
    return jsNumber(exec, 1.7976931348623157E+308);
}

static AJValue numberConstructorMinValue(ExecState* exec, AJValue, const Identifier&)
{
    return jsNumber(exec, 5E-324);
}

// ECMA 15.7.1
static AJObject* constructWithNumberConstructor(ExecState* exec, AJObject*, const ArgList& args)
{
    NumberObject* object = new (exec) NumberObject(exec->lexicalGlobalObject()->numberObjectStructure());
    double n = args.isEmpty() ? 0 : args.at(0).toNumber(exec);
    object->setInternalValue(jsNumber(exec, n));
    return object;
}

ConstructType NumberConstructor::getConstructData(ConstructData& constructData)
{
    constructData.native.function = constructWithNumberConstructor;
    return ConstructTypeHost;
}

// ECMA 15.7.2
static AJValue JSC_HOST_CALL callNumberConstructor(ExecState* exec, AJObject*, AJValue, const ArgList& args)
{
    return jsNumber(exec, args.isEmpty() ? 0 : args.at(0).toNumber(exec));
}

CallType NumberConstructor::getCallData(CallData& callData)
{
    callData.native.function = callNumberConstructor;
    return CallTypeHost;
}

} // namespace AJ
