
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
#include "AJStringRef.h"
#include "AJValueRef.h"
#include "Node.h"
#include "NodeList.h"
#include "UnusedParam.h"
#include <wtf/Assertions.h>

static AJValueRef JSNode_appendChild(AJContextRef context, AJObjectRef function, AJObjectRef thisObject, size_t argumentCount, const AJValueRef arguments[], AJValueRef* exception)
{
    UNUSED_PARAM(function);

    /* Example of throwing a type error for invalid values */
    if (!AJValueIsObjectOfClass(context, thisObject, JSNode_class(context))) {
        AJStringRef message = AJStringCreateWithUTF8CString("TypeError: appendChild can only be called on nodes");
        *exception = AJValueMakeString(context, message);
        AJStringRelease(message);
    } else if (argumentCount < 1 || !AJValueIsObjectOfClass(context, arguments[0], JSNode_class(context))) {
        AJStringRef message = AJStringCreateWithUTF8CString("TypeError: first argument to appendChild must be a node");
        *exception = AJValueMakeString(context, message);
        AJStringRelease(message);
    } else {
        Node* node = AJObjectGetPrivate(thisObject);
        Node* child = AJObjectGetPrivate(AJValueToObject(context, arguments[0], NULL));

        Node_appendChild(node, child);
    }

    return AJValueMakeUndefined(context);
}

static AJValueRef JSNode_removeChild(AJContextRef context, AJObjectRef function, AJObjectRef thisObject, size_t argumentCount, const AJValueRef arguments[], AJValueRef* exception)
{
    UNUSED_PARAM(function);

    /* Example of ignoring invalid values */
    if (argumentCount > 0) {
        if (AJValueIsObjectOfClass(context, thisObject, JSNode_class(context))) {
            if (AJValueIsObjectOfClass(context, arguments[0], JSNode_class(context))) {
                Node* node = AJObjectGetPrivate(thisObject);
                Node* child = AJObjectGetPrivate(AJValueToObject(context, arguments[0], exception));
                
                Node_removeChild(node, child);
            }
        }
    }
    
    return AJValueMakeUndefined(context);
}

static AJValueRef JSNode_replaceChild(AJContextRef context, AJObjectRef function, AJObjectRef thisObject, size_t argumentCount, const AJValueRef arguments[], AJValueRef* exception)
{
    UNUSED_PARAM(function);
    
    if (argumentCount > 1) {
        if (AJValueIsObjectOfClass(context, thisObject, JSNode_class(context))) {
            if (AJValueIsObjectOfClass(context, arguments[0], JSNode_class(context))) {
                if (AJValueIsObjectOfClass(context, arguments[1], JSNode_class(context))) {
                    Node* node = AJObjectGetPrivate(thisObject);
                    Node* newChild = AJObjectGetPrivate(AJValueToObject(context, arguments[0], exception));
                    Node* oldChild = AJObjectGetPrivate(AJValueToObject(context, arguments[1], exception));
                    
                    Node_replaceChild(node, newChild, oldChild);
                }
            }
        }
    }
    
    return AJValueMakeUndefined(context);
}

static AJStaticFunction JSNode_staticFunctions[] = {
    { "appendChild", JSNode_appendChild, kAJPropertyAttributeDontDelete },
    { "removeChild", JSNode_removeChild, kAJPropertyAttributeDontDelete },
    { "replaceChild", JSNode_replaceChild, kAJPropertyAttributeDontDelete },
    { 0, 0, 0 }
};

static AJValueRef JSNode_getNodeType(AJContextRef context, AJObjectRef object, AJStringRef propertyName, AJValueRef* exception)
{
    UNUSED_PARAM(propertyName);
    UNUSED_PARAM(exception);

    Node* node = AJObjectGetPrivate(object);
    if (node) {
        AJStringRef nodeType = AJStringCreateWithUTF8CString(node->nodeType);
        AJValueRef value = AJValueMakeString(context, nodeType);
        AJStringRelease(nodeType);
        return value;
    }
    
    return NULL;
}

static AJValueRef JSNode_getChildNodes(AJContextRef context, AJObjectRef thisObject, AJStringRef propertyName, AJValueRef* exception)
{
    UNUSED_PARAM(propertyName);
    UNUSED_PARAM(exception);

    Node* node = AJObjectGetPrivate(thisObject);
    ASSERT(node);
    return JSNodeList_new(context, NodeList_new(node));
}

static AJValueRef JSNode_getFirstChild(AJContextRef context, AJObjectRef object, AJStringRef propertyName, AJValueRef* exception)
{
    UNUSED_PARAM(object);
    UNUSED_PARAM(propertyName);
    UNUSED_PARAM(exception);
    
    return AJValueMakeUndefined(context);
}

static AJStaticValue JSNode_staticValues[] = {
    { "nodeType", JSNode_getNodeType, NULL, kAJPropertyAttributeDontDelete | kAJPropertyAttributeReadOnly },
    { "childNodes", JSNode_getChildNodes, NULL, kAJPropertyAttributeDontDelete | kAJPropertyAttributeReadOnly },
    { "firstChild", JSNode_getFirstChild, NULL, kAJPropertyAttributeDontDelete | kAJPropertyAttributeReadOnly },
    { 0, 0, 0, 0 }
};

static void JSNode_initialize(AJContextRef context, AJObjectRef object)
{
    UNUSED_PARAM(context);

    Node* node = AJObjectGetPrivate(object);
    ASSERT(node);

    Node_ref(node);
}

static void JSNode_finalize(AJObjectRef object)
{
    Node* node = AJObjectGetPrivate(object);
    ASSERT(node);

    Node_deref(node);
}

AJClassRef JSNode_class(AJContextRef context)
{
    UNUSED_PARAM(context);

    static AJClassRef jsClass;
    if (!jsClass) {
        AJClassDefinition definition = kAJClassDefinitionEmpty;
        definition.staticValues = JSNode_staticValues;
        definition.staticFunctions = JSNode_staticFunctions;
        definition.initialize = JSNode_initialize;
        definition.finalize = JSNode_finalize;

        jsClass = AJClassCreate(&definition);
    }
    return jsClass;
}

AJObjectRef JSNode_new(AJContextRef context, Node* node)
{
    return AJObjectMake(context, JSNode_class(context), node);
}

AJObjectRef JSNode_construct(AJContextRef context, AJObjectRef object, size_t argumentCount, const AJValueRef arguments[], AJValueRef* exception)
{
    UNUSED_PARAM(object);
    UNUSED_PARAM(argumentCount);
    UNUSED_PARAM(arguments);
    UNUSED_PARAM(exception);

    return JSNode_new(context, Node_new());
}
