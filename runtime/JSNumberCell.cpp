
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
#include "JSNumberCell.h"

#if USE(JSVALUE32)

#include "NumberObject.h"
#include "UString.h"

namespace AJ {

AJValue JSNumberCell::toPrimitive(ExecState*, PreferredPrimitiveType) const
{
    return const_cast<JSNumberCell*>(this);
}

bool JSNumberCell::getPrimitiveNumber(ExecState*, double& number, AJValue& value)
{
    number = m_value;
    value = this;
    return true;
}

bool JSNumberCell::toBoolean(ExecState*) const
{
    return m_value < 0.0 || m_value > 0.0; // false for NaN
}

double JSNumberCell::toNumber(ExecState*) const
{
  return m_value;
}

UString JSNumberCell::toString(ExecState*) const
{
    return UString::from(m_value);
}

AJObject* JSNumberCell::toObject(ExecState* exec) const
{
    return constructNumber(exec, const_cast<JSNumberCell*>(this));
}

AJObject* JSNumberCell::toThisObject(ExecState* exec) const
{
    return constructNumber(exec, const_cast<JSNumberCell*>(this));
}

bool JSNumberCell::getUInt32(uint32_t& uint32) const
{
    uint32 = static_cast<uint32_t>(m_value);
    return uint32 == m_value;
}

AJValue JSNumberCell::getJSNumber()
{
    return this;
}

AJValue jsNumberCell(ExecState* exec, double d)
{
    return new (exec) JSNumberCell(exec, d);
}

AJValue jsNumberCell(AJGlobalData* globalData, double d)
{
    return new (globalData) JSNumberCell(globalData, d);
}

} // namespace AJ

#else // USE(JSVALUE32)

// Keep our exported symbols lists happy.
namespace AJ {

AJValue jsNumberCell(ExecState*, double);

AJValue jsNumberCell(ExecState*, double)
{
    ASSERT_NOT_REACHED();
    return AJValue();
}

} // namespace AJ

#endif // USE(JSVALUE32)
