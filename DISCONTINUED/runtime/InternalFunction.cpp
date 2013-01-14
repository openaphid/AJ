
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
 *  Copyright (C) 2004, 2007, 2008 Apple Inc. All rights reserved.
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
#include "InternalFunction.h"

#include "FunctionPrototype.h"
#include "AJString.h"

namespace AJ {

ASSERT_CLASS_FITS_IN_CELL(InternalFunction);

const ClassInfo InternalFunction::info = { "Function", 0, 0, 0 };

const ClassInfo* InternalFunction::classInfo() const
{
    return &info;
}

InternalFunction::InternalFunction(AJGlobalData* globalData, NonNullPassRefPtr<Structure> structure, const Identifier& name)
    : AJObject(structure)
{
    putDirect(globalData->propertyNames->name, jsString(globalData, name.isNull() ? "" : name.ustring()), DontDelete | ReadOnly | DontEnum);
}

const UString& InternalFunction::name(ExecState* exec)
{
    return asString(getDirect(exec->globalData().propertyNames->name))->tryGetValue();
}

const UString InternalFunction::displayName(ExecState* exec)
{
    AJValue displayName = getDirect(exec->globalData().propertyNames->displayName);
    
    if (displayName && isAJString(&exec->globalData(), displayName))
        return asString(displayName)->tryGetValue();
    
    return UString::null();
}

const UString InternalFunction::calculatedDisplayName(ExecState* exec)
{
    const UString explicitName = displayName(exec);
    
    if (!explicitName.isEmpty())
        return explicitName;
    
    return name(exec);
}

} // namespace AJ
