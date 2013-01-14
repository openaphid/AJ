
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
 * Copyright (C) 2008, 2009 Apple Inc. All Rights Reserved.
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

#include "AJStaticScopeObject.h"

namespace AJ {

ASSERT_CLASS_FITS_IN_CELL(AJStaticScopeObject);

void AJStaticScopeObject::markChildren(MarkStack& markStack)
{
    JSVariableObject::markChildren(markStack);
    markStack.append(d()->registerStore.jsValue());
}

AJObject* AJStaticScopeObject::toThisObject(ExecState* exec) const
{
    return exec->globalThisValue();
}

void AJStaticScopeObject::put(ExecState*, const Identifier& propertyName, AJValue value, PutPropertySlot&)
{
    if (symbolTablePut(propertyName, value))
        return;
    
    ASSERT_NOT_REACHED();
}

void AJStaticScopeObject::putWithAttributes(ExecState*, const Identifier& propertyName, AJValue value, unsigned attributes)
{
    if (symbolTablePutWithAttributes(propertyName, value, attributes))
        return;
    
    ASSERT_NOT_REACHED();
}

bool AJStaticScopeObject::isDynamicScope(bool&) const
{
    return false;
}

AJStaticScopeObject::~AJStaticScopeObject()
{
    ASSERT(d());
    delete d();
}

inline bool AJStaticScopeObject::getOwnPropertySlot(ExecState*, const Identifier& propertyName, PropertySlot& slot)
{
    return symbolTableGet(propertyName, slot);
}

}
