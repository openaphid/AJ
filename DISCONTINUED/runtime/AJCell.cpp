
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
 *  Copyright (C) 2003, 2007, 2008 Apple Inc. All rights reserved.
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
#include "AJCell.h"

#include "AJFunction.h"
#include "AJString.h"
#include "AJObject.h"
#include <wtf/MathExtras.h>

namespace AJ {

#if defined NAN && defined INFINITY

extern const double NaN = NAN;
extern const double Inf = INFINITY;

#else // !(defined NAN && defined INFINITY)

// The trick is to define the NaN and Inf globals with a different type than the declaration.
// This trick works because the mangled name of the globals does not include the type, although
// I'm not sure that's guaranteed. There could be alignment issues with this, since arrays of
// characters don't necessarily need the same alignment doubles do, but for now it seems to work.
// It would be good to figure out a 100% clean way that still avoids code that runs at init time.

// Note, we have to use union to ensure alignment. Otherwise, NaN_Bytes can start anywhere,
// while NaN_double has to be 4-byte aligned for 32-bits.
// With -fstrict-aliasing enabled, unions are the only safe way to do type masquerading.

static const union {
    struct {
        unsigned char NaN_Bytes[8];
        unsigned char Inf_Bytes[8];
    } bytes;
    
    struct {
        double NaN_Double;
        double Inf_Double;
    } doubles;
    
} NaNInf = { {
#if CPU(BIG_ENDIAN)
    { 0x7f, 0xf8, 0, 0, 0, 0, 0, 0 },
    { 0x7f, 0xf0, 0, 0, 0, 0, 0, 0 }
#elif CPU(MIDDLE_ENDIAN)
    { 0, 0, 0xf8, 0x7f, 0, 0, 0, 0 },
    { 0, 0, 0xf0, 0x7f, 0, 0, 0, 0 }
#else
    { 0, 0, 0, 0, 0, 0, 0xf8, 0x7f },
    { 0, 0, 0, 0, 0, 0, 0xf0, 0x7f }
#endif
} } ;

extern const double NaN = NaNInf.doubles.NaN_Double;
extern const double Inf = NaNInf.doubles.Inf_Double;
 
#endif // !(defined NAN && defined INFINITY)

bool AJCell::getUInt32(uint32_t&) const
{
    return false;
}

bool AJCell::getString(ExecState* exec, UString&stringValue) const
{
    if (!isString())
        return false;
    stringValue = static_cast<const AJString*>(this)->value(exec);
    return true;
}

UString AJCell::getString(ExecState* exec) const
{
    return isString() ? static_cast<const AJString*>(this)->value(exec) : UString();
}

AJObject* AJCell::getObject()
{
    return isObject() ? asObject(this) : 0;
}

const AJObject* AJCell::getObject() const
{
    return isObject() ? static_cast<const AJObject*>(this) : 0;
}

CallType AJCell::getCallData(CallData&)
{
    return CallTypeNone;
}

ConstructType AJCell::getConstructData(ConstructData&)
{
    return ConstructTypeNone;
}

bool AJCell::getOwnPropertySlot(ExecState* exec, const Identifier& identifier, PropertySlot& slot)
{
    // This is not a general purpose implementation of getOwnPropertySlot.
    // It should only be called by AJValue::get.
    // It calls getPropertySlot, not getOwnPropertySlot.
    AJObject* object = toObject(exec);
    slot.setBase(object);
    if (!object->getPropertySlot(exec, identifier, slot))
        slot.setUndefined();
    return true;
}

bool AJCell::getOwnPropertySlot(ExecState* exec, unsigned identifier, PropertySlot& slot)
{
    // This is not a general purpose implementation of getOwnPropertySlot.
    // It should only be called by AJValue::get.
    // It calls getPropertySlot, not getOwnPropertySlot.
    AJObject* object = toObject(exec);
    slot.setBase(object);
    if (!object->getPropertySlot(exec, identifier, slot))
        slot.setUndefined();
    return true;
}

void AJCell::put(ExecState* exec, const Identifier& identifier, AJValue value, PutPropertySlot& slot)
{
    toObject(exec)->put(exec, identifier, value, slot);
}

void AJCell::put(ExecState* exec, unsigned identifier, AJValue value)
{
    toObject(exec)->put(exec, identifier, value);
}

bool AJCell::deleteProperty(ExecState* exec, const Identifier& identifier)
{
    return toObject(exec)->deleteProperty(exec, identifier);
}

bool AJCell::deleteProperty(ExecState* exec, unsigned identifier)
{
    return toObject(exec)->deleteProperty(exec, identifier);
}

AJObject* AJCell::toThisObject(ExecState* exec) const
{
    return toObject(exec);
}

const ClassInfo* AJCell::classInfo() const
{
    return 0;
}

AJValue AJCell::getJSNumber()
{
    return AJValue();
}

bool AJCell::isGetterSetter() const
{
    return false;
}

AJValue AJCell::toPrimitive(ExecState*, PreferredPrimitiveType) const
{
    ASSERT_NOT_REACHED();
    return AJValue();
}

bool AJCell::getPrimitiveNumber(ExecState*, double&, AJValue&)
{
    ASSERT_NOT_REACHED();
    return false;
}

bool AJCell::toBoolean(ExecState*) const
{
    ASSERT_NOT_REACHED();
    return false;
}

double AJCell::toNumber(ExecState*) const
{
    ASSERT_NOT_REACHED();
    return 0;
}

UString AJCell::toString(ExecState*) const
{
    ASSERT_NOT_REACHED();
    return UString();
}

AJObject* AJCell::toObject(ExecState*) const
{
    ASSERT_NOT_REACHED();
    return 0;
}

} // namespace AJ
