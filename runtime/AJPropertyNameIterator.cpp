
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
 * Copyright (C) 2008, 2009 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "AJPropertyNameIterator.h"

#include "AJGlobalObject.h"

namespace AJ {

ASSERT_CLASS_FITS_IN_CELL(AJPropertyNameIterator);

inline AJPropertyNameIterator::AJPropertyNameIterator(ExecState* exec, PropertyNameArrayData* propertyNameArrayData, size_t numCacheableSlots)
    : AJCell(exec->globalData().propertyNameIteratorStructure.get())
    , m_cachedStructure(0)
    , m_numCacheableSlots(numCacheableSlots)
    , m_jsStringsSize(propertyNameArrayData->propertyNameVector().size())
    , m_jsStrings(new AJValue[m_jsStringsSize])
{
    PropertyNameArrayData::PropertyNameVector& propertyNameVector = propertyNameArrayData->propertyNameVector();
    for (size_t i = 0; i < m_jsStringsSize; ++i)
        m_jsStrings[i] = jsOwnedString(exec, propertyNameVector[i].ustring());
}

AJPropertyNameIterator::~AJPropertyNameIterator()
{
    if (m_cachedStructure)
        m_cachedStructure->clearEnumerationCache(this);
}

AJPropertyNameIterator* AJPropertyNameIterator::create(ExecState* exec, AJObject* o)
{
    ASSERT(!o->structure()->enumerationCache() ||
            o->structure()->enumerationCache()->cachedStructure() != o->structure() ||
            o->structure()->enumerationCache()->cachedPrototypeChain() != o->structure()->prototypeChain(exec));

    PropertyNameArray propertyNames(exec);
    o->getPropertyNames(exec, propertyNames);
    size_t numCacheableSlots = 0;
    if (!o->structure()->hasNonEnumerableProperties() && !o->structure()->hasAnonymousSlots() &&
        !o->structure()->hasGetterSetterProperties() && !o->structure()->isUncacheableDictionary() &&
        !o->structure()->typeInfo().overridesGetPropertyNames())
        numCacheableSlots = o->structure()->propertyStorageSize();

    AJPropertyNameIterator* jsPropertyNameIterator = new (exec) AJPropertyNameIterator(exec, propertyNames.data(), numCacheableSlots);

    if (o->structure()->isDictionary())
        return jsPropertyNameIterator;

    if (o->structure()->typeInfo().overridesGetPropertyNames())
        return jsPropertyNameIterator;
    
    size_t count = normalizePrototypeChain(exec, o);
    StructureChain* structureChain = o->structure()->prototypeChain(exec);
    RefPtr<Structure>* structure = structureChain->head();
    for (size_t i = 0; i < count; ++i) {
        if (structure[i]->typeInfo().overridesGetPropertyNames())
            return jsPropertyNameIterator;
    }

    jsPropertyNameIterator->setCachedPrototypeChain(structureChain);
    jsPropertyNameIterator->setCachedStructure(o->structure());
    o->structure()->setEnumerationCache(jsPropertyNameIterator);
    return jsPropertyNameIterator;
}

AJValue AJPropertyNameIterator::get(ExecState* exec, AJObject* base, size_t i)
{
    AJValue& identifier = m_jsStrings[i];
    if (m_cachedStructure == base->structure() && m_cachedPrototypeChain == base->structure()->prototypeChain(exec))
        return identifier;

    if (!base->hasProperty(exec, Identifier(exec, asString(identifier)->value(exec))))
        return AJValue();
    return identifier;
}

void AJPropertyNameIterator::markChildren(MarkStack& markStack)
{
    markStack.appendValues(m_jsStrings.get(), m_jsStringsSize, MayContainNullValues);
}

} // namespace AJ