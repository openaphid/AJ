
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
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "JSNode.h"
#include "JSNodeList.h"
#include "AJObjectRef.h"
#include "AJValueRef.h"
#include "UnusedParam.h"
#include <wtf/Assertions.h>

static AJValueRef JSNodeList_item(AJContextRef context, AJObjectRef object, AJObjectRef thisObject, size_t argumentCount, const AJValueRef arguments[], AJValueRef* exception)
{
    UNUSED_PARAM(object);

    if (argumentCount > 0) {
        NodeList* nodeList = AJObjectGetPrivate(thisObject);
        ASSERT(nodeList);
        Node* node = NodeList_item(nodeList, (unsigned)AJValueToNumber(context, arguments[0], exception));
        if (node)
            return JSNode_new(context, node);
    }
    
    return AJValueMakeUndefined(context);
}

static AJStaticFunction JSNodeList_staticFunctions[] = {
    { "item", JSNodeList_item, kAJPropertyAttributeDontDelete },
    { 0, 0, 0 }
};

static AJValueRef JSNodeList_length(AJContextRef context, AJObjectRef thisObject, AJStringRef propertyName, AJValueRef* exception)
{
    UNUSED_PARAM(propertyName);
    UNUSED_PARAM(exception);
    
    NodeList* nodeList = AJObjectGetPrivate(thisObject);
    ASSERT(nodeList);
    return AJValueMakeNumber(context, NodeList_length(nodeList));
}

static AJStaticValue JSNodeList_staticValues[] = {
    { "length", JSNodeList_length, NULL, kAJPropertyAttributeReadOnly | kAJPropertyAttributeDontDelete },
    { 0, 0, 0, 0 }
};

static AJValueRef JSNodeList_getProperty(AJContextRef context, AJObjectRef thisObject, AJStringRef propertyName, AJValueRef* exception)
{
    NodeList* nodeList = AJObjectGetPrivate(thisObject);
    ASSERT(nodeList);
    double index = AJValueToNumber(context, AJValueMakeString(context, propertyName), exception);
    unsigned uindex = (unsigned)index;
    if (uindex == index) { /* false for NaN */
        Node* node = NodeList_item(nodeList, uindex);
        if (node)
            return JSNode_new(context, node);
    }
    
    return NULL;
}

static void JSNodeList_initialize(AJContextRef context, AJObjectRef thisObject)
{
    UNUSED_PARAM(context);

    NodeList* nodeList = AJObjectGetPrivate(thisObject);
    ASSERT(nodeList);
    
    NodeList_ref(nodeList);
}

static void JSNodeList_finalize(AJObjectRef thisObject)
{
    NodeList* nodeList = AJObjectGetPrivate(thisObject);
    ASSERT(nodeList);

    NodeList_deref(nodeList);
}

static AJClassRef JSNodeList_class(AJContextRef context)
{
    UNUSED_PARAM(context);

    static AJClassRef jsClass;
    if (!jsClass) {
        AJClassDefinition definition = kAJClassDefinitionEmpty;
        definition.staticValues = JSNodeList_staticValues;
        definition.staticFunctions = JSNodeList_staticFunctions;
        definition.getProperty = JSNodeList_getProperty;
        definition.initialize = JSNodeList_initialize;
        definition.finalize = JSNodeList_finalize;

        jsClass = AJClassCreate(&definition);
    }
    
    return jsClass;
}

AJObjectRef JSNodeList_new(AJContextRef context, NodeList* nodeList)
{
    return AJObjectMake(context, JSNodeList_class(context), nodeList);
}
