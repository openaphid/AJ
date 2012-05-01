
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
 * Copyright (C) 2009 Apple Inc. All Rights Reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "AJArrayArray.h"

#include "AJGlobalObject.h"
#include "PropertyNameArray.h"

using namespace ATF;

namespace AJ {

const ClassInfo AJArrayArray::s_defaultInfo = { "ByteArray", 0, 0, 0 };

AJArrayArray::AJArrayArray(ExecState* exec, NonNullPassRefPtr<Structure> structure, ByteArray* storage, const AJ::ClassInfo* classInfo)
    : AJObject(structure)
    , m_storage(storage)
    , m_classInfo(classInfo)
{
    putDirect(exec->globalData().propertyNames->length, jsNumber(exec, m_storage->length()), ReadOnly | DontDelete);
}

#if !ASSERT_DISABLED
AJArrayArray::~AJArrayArray()
{
    ASSERT(vptr() == AJGlobalData::jsByteArrayVPtr);
}
#endif


PassRefPtr<Structure> AJArrayArray::createStructure(AJValue prototype)
{
    PassRefPtr<Structure> result = Structure::create(prototype, TypeInfo(ObjectType, StructureFlags), AnonymousSlotCount);
    return result;
}

bool AJArrayArray::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    bool ok;
    unsigned index = propertyName.toUInt32(&ok, false);
    if (ok && canAccessIndex(index)) {
        slot.setValue(getIndex(exec, index));
        return true;
    }
    return AJObject::getOwnPropertySlot(exec, propertyName, slot);
}

bool AJArrayArray::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    bool ok;
    unsigned index = propertyName.toUInt32(&ok, false);
    if (ok && canAccessIndex(index)) {
        descriptor.setDescriptor(getIndex(exec, index), DontDelete);
        return true;
    }
    return AJObject::getOwnPropertyDescriptor(exec, propertyName, descriptor);
}

bool AJArrayArray::getOwnPropertySlot(ExecState* exec, unsigned propertyName, PropertySlot& slot)
{
    if (canAccessIndex(propertyName)) {
        slot.setValue(getIndex(exec, propertyName));
        return true;
    }
    return AJObject::getOwnPropertySlot(exec, Identifier::from(exec, propertyName), slot);
}

void AJArrayArray::put(ExecState* exec, const Identifier& propertyName, AJValue value, PutPropertySlot& slot)
{
    bool ok;
    unsigned index = propertyName.toUInt32(&ok, false);
    if (ok) {
        setIndex(exec, index, value);
        return;
    }
    AJObject::put(exec, propertyName, value, slot);
}

void AJArrayArray::put(ExecState* exec, unsigned propertyName, AJValue value)
{
    setIndex(exec, propertyName, value);
}

void AJArrayArray::getOwnPropertyNames(ExecState* exec, PropertyNameArray& propertyNames, EnumerationMode mode)
{
    unsigned length = m_storage->length();
    for (unsigned i = 0; i < length; ++i)
        propertyNames.add(Identifier::from(exec, i));
    AJObject::getOwnPropertyNames(exec, propertyNames, mode);
}

}

