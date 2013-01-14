
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
 *  Copyright (C) 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
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
#include "StringObject.h"

#include "PropertyNameArray.h"

namespace AJ {

ASSERT_CLASS_FITS_IN_CELL(StringObject);

const ClassInfo StringObject::info = { "String", 0, 0, 0 };

StringObject::StringObject(ExecState* exec, NonNullPassRefPtr<Structure> structure)
    : JSWrapperObject(structure)
{
    setInternalValue(jsEmptyString(exec));
}

StringObject::StringObject(NonNullPassRefPtr<Structure> structure, AJString* string)
    : JSWrapperObject(structure)
{
    setInternalValue(string);
}

StringObject::StringObject(ExecState* exec, NonNullPassRefPtr<Structure> structure, const UString& string)
    : JSWrapperObject(structure)
{
    setInternalValue(jsString(exec, string));
}

bool StringObject::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    if (internalValue()->getStringPropertySlot(exec, propertyName, slot))
        return true;
    return AJObject::getOwnPropertySlot(exec, propertyName, slot);
}
    
bool StringObject::getOwnPropertySlot(ExecState* exec, unsigned propertyName, PropertySlot& slot)
{
    if (internalValue()->getStringPropertySlot(exec, propertyName, slot))
        return true;    
    return AJObject::getOwnPropertySlot(exec, Identifier::from(exec, propertyName), slot);
}

bool StringObject::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    if (internalValue()->getStringPropertyDescriptor(exec, propertyName, descriptor))
        return true;    
    return AJObject::getOwnPropertyDescriptor(exec, propertyName, descriptor);
}

void StringObject::put(ExecState* exec, const Identifier& propertyName, AJValue value, PutPropertySlot& slot)
{
    if (propertyName == exec->propertyNames().length)
        return;
    AJObject::put(exec, propertyName, value, slot);
}

bool StringObject::deleteProperty(ExecState* exec, const Identifier& propertyName)
{
    if (propertyName == exec->propertyNames().length)
        return false;
    bool isStrictUInt32;
    unsigned i = propertyName.toStrictUInt32(&isStrictUInt32);
    if (isStrictUInt32 && internalValue()->canGetIndex(i))
        return false;
    return AJObject::deleteProperty(exec, propertyName);
}

void StringObject::getOwnPropertyNames(ExecState* exec, PropertyNameArray& propertyNames, EnumerationMode mode)
{
    int size = internalValue()->length();
    for (int i = 0; i < size; ++i)
        propertyNames.add(Identifier(exec, UString::from(i)));
    if (mode == IncludeDontEnumProperties)
        propertyNames.add(exec->propertyNames().length);
    return AJObject::getOwnPropertyNames(exec, propertyNames, mode);
}

} // namespace AJ
