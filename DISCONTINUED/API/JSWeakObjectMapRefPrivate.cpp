
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
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "JSWeakObjectMapRefPrivate.h"

#include "APICast.h"
#include "APIShims.h"
#include "AJCallbackObject.h"
#include "AJValue.h"
#include "JSWeakObjectMapRefInternal.h"
#include <wtf/HashMap.h>
#include <wtf/RefCounted.h>
#include <wtf/text/StringHash.h>

using namespace ATF;
using namespace AJ;

#ifdef __cplusplus
extern "C" {
#endif

JSWeakObjectMapRef JSWeakObjectMapCreate(AJContextRef context, void* privateData, JSWeakMapDestroyedCallback callback)
{
    ExecState* exec = toJS(context);
    APIEntryShim entryShim(exec);
    RefPtr<OpaqueJSWeakObjectMap> map = OpaqueJSWeakObjectMap::create(privateData, callback);
    exec->lexicalGlobalObject()->registerWeakMap(map.get());
    return map.get();
}

void JSWeakObjectMapSet(AJContextRef ctx, JSWeakObjectMapRef map, void* key, AJObjectRef object)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);
    AJObject* obj = toJS(object);
    if (!obj)
        return;
    ASSERT(obj->inherits(&AJCallbackObject<AJGlobalObject>::info) || obj->inherits(&AJCallbackObject<AJObject>::info));
    map->map().set(key, obj);
}

AJObjectRef JSWeakObjectMapGet(AJContextRef ctx, JSWeakObjectMapRef map, void* key)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);
    return toRef(static_cast<AJObject*>(map->map().get(key)));
}

bool JSWeakObjectMapClear(AJContextRef ctx, JSWeakObjectMapRef map, void* key, AJObjectRef object)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);
    AJObject* obj = toJS(object);
    if (map->map().uncheckedRemove(key, obj))
        return true;
    return false;
}

#ifdef __cplusplus
}
#endif
