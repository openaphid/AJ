
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
 * Copyright (C) 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Kelvin W Sherlock (ksherlock@gmail.com)
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

#include "config.h"
#include "AJObjectRef.h"
#include "AJObjectRefPrivate.h"

#include "APICast.h"
#include "CodeBlock.h"
#include "DateConstructor.h"
#include "ErrorConstructor.h"
#include "FunctionConstructor.h"
#include "Identifier.h"
#include "InitializeThreading.h"
#include "AJArray.h"
#include "AJCallbackConstructor.h"
#include "AJCallbackFunction.h"
#include "AJCallbackObject.h"
#include "AJClassRef.h"
#include "AJFunction.h"
#include "AJGlobalObject.h"
#include "AJObject.h"
#include "AJRetainPtr.h"
#include "AJString.h"
#include "AJValueRef.h"
#include "ObjectPrototype.h"
#include "PropertyNameArray.h"
#include "RegExpConstructor.h"

using namespace AJ;

AJClassRef AJClassCreate(const AJClassDefinition* definition)
{
    initializeThreading();
    RefPtr<OpaqueAJClass> jsClass = (definition->attributes & kAJClassAttributeNoAutomaticPrototype)
        ? OpaqueAJClass::createNoAutomaticPrototype(definition)
        : OpaqueAJClass::create(definition);
    
    return jsClass.release().releaseRef();
}

AJClassRef AJClassRetain(AJClassRef jsClass)
{
    jsClass->ref();
    return jsClass;
}

void AJClassRelease(AJClassRef jsClass)
{
    jsClass->deref();
}

AJObjectRef AJObjectMake(AJContextRef ctx, AJClassRef jsClass, void* data)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    if (!jsClass)
        return toRef(new (exec) AJObject(exec->lexicalGlobalObject()->emptyObjectStructure())); // slightly more efficient

    AJCallbackObject<AJObject>* object = new (exec) AJCallbackObject<AJObject>(exec, exec->lexicalGlobalObject()->callbackObjectStructure(), jsClass, data);
    if (AJObject* prototype = jsClass->prototype(exec))
        object->setPrototype(prototype);

    return toRef(object);
}

AJObjectRef AJObjectMakeFunctionWithCallback(AJContextRef ctx, AJStringRef name, AJObjectCallAsFunctionCallback callAsFunction)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    Identifier nameID = name ? name->identifier(&exec->globalData()) : Identifier(exec, "anonymous");
    
    return toRef(new (exec) AJCallbackFunction(exec, callAsFunction, nameID));
}

AJObjectRef AJObjectMakeConstructor(AJContextRef ctx, AJClassRef jsClass, AJObjectCallAsConstructorCallback callAsConstructor)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJValue jsPrototype = jsClass ? jsClass->prototype(exec) : 0;
    if (!jsPrototype)
        jsPrototype = exec->lexicalGlobalObject()->objectPrototype();

    AJCallbackConstructor* constructor = new (exec) AJCallbackConstructor(exec->lexicalGlobalObject()->callbackConstructorStructure(), jsClass, callAsConstructor);
    constructor->putDirect(exec->propertyNames().prototype, jsPrototype, DontEnum | DontDelete | ReadOnly);
    return toRef(constructor);
}

AJObjectRef AJObjectMakeFunction(AJContextRef ctx, AJStringRef name, unsigned parameterCount, const AJStringRef parameterNames[], AJStringRef body, AJStringRef sourceURL, int startingLineNumber, AJValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    Identifier nameID = name ? name->identifier(&exec->globalData()) : Identifier(exec, "anonymous");
    
    MarkedArgumentBuffer args;
    for (unsigned i = 0; i < parameterCount; i++)
        args.append(jsString(exec, parameterNames[i]->ustring()));
    args.append(jsString(exec, body->ustring()));

    AJObject* result = constructFunction(exec, args, nameID, sourceURL->ustring(), startingLineNumber);
    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
        result = 0;
    }
    return toRef(result);
}

AJObjectRef AJObjectMakeArray(AJContextRef ctx, size_t argumentCount, const AJValueRef arguments[],  AJValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJObject* result;
    if (argumentCount) {
        MarkedArgumentBuffer argList;
        for (size_t i = 0; i < argumentCount; ++i)
            argList.append(toJS(exec, arguments[i]));

        result = constructArray(exec, argList);
    } else
        result = constructEmptyArray(exec);

    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
        result = 0;
    }

    return toRef(result);
}

AJObjectRef AJObjectMakeDate(AJContextRef ctx, size_t argumentCount, const AJValueRef arguments[],  AJValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    MarkedArgumentBuffer argList;
    for (size_t i = 0; i < argumentCount; ++i)
        argList.append(toJS(exec, arguments[i]));

    AJObject* result = constructDate(exec, argList);
    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
        result = 0;
    }

    return toRef(result);
}

AJObjectRef AJObjectMakeError(AJContextRef ctx, size_t argumentCount, const AJValueRef arguments[],  AJValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    MarkedArgumentBuffer argList;
    for (size_t i = 0; i < argumentCount; ++i)
        argList.append(toJS(exec, arguments[i]));

    AJObject* result = constructError(exec, argList);
    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
        result = 0;
    }

    return toRef(result);
}

AJObjectRef AJObjectMakeRegExp(AJContextRef ctx, size_t argumentCount, const AJValueRef arguments[],  AJValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    MarkedArgumentBuffer argList;
    for (size_t i = 0; i < argumentCount; ++i)
        argList.append(toJS(exec, arguments[i]));

    AJObject* result = constructRegExp(exec, argList);
    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
        result = 0;
    }
    
    return toRef(result);
}

AJValueRef AJObjectGetPrototype(AJContextRef ctx, AJObjectRef object)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJObject* jsObject = toJS(object);
    return toRef(exec, jsObject->prototype());
}

void AJObjectSetPrototype(AJContextRef ctx, AJObjectRef object, AJValueRef value)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJObject* jsObject = toJS(object);
    AJValue jsValue = toJS(exec, value);

    jsObject->setPrototype(jsValue.isObject() ? jsValue : jsNull());
}

bool AJObjectHasProperty(AJContextRef ctx, AJObjectRef object, AJStringRef propertyName)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJObject* jsObject = toJS(object);
    
    return jsObject->hasProperty(exec, propertyName->identifier(&exec->globalData()));
}

AJValueRef AJObjectGetProperty(AJContextRef ctx, AJObjectRef object, AJStringRef propertyName, AJValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJObject* jsObject = toJS(object);

    AJValue jsValue = jsObject->get(exec, propertyName->identifier(&exec->globalData()));
    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
    }
    return toRef(exec, jsValue);
}

void AJObjectSetProperty(AJContextRef ctx, AJObjectRef object, AJStringRef propertyName, AJValueRef value, AJPropertyAttributes attributes, AJValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJObject* jsObject = toJS(object);
    Identifier name(propertyName->identifier(&exec->globalData()));
    AJValue jsValue = toJS(exec, value);

    if (attributes && !jsObject->hasProperty(exec, name))
        jsObject->putWithAttributes(exec, name, jsValue, attributes);
    else {
        PutPropertySlot slot;
        jsObject->put(exec, name, jsValue, slot);
    }

    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
    }
}

AJValueRef AJObjectGetPropertyAtIndex(AJContextRef ctx, AJObjectRef object, unsigned propertyIndex, AJValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJObject* jsObject = toJS(object);

    AJValue jsValue = jsObject->get(exec, propertyIndex);
    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
    }
    return toRef(exec, jsValue);
}


void AJObjectSetPropertyAtIndex(AJContextRef ctx, AJObjectRef object, unsigned propertyIndex, AJValueRef value, AJValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJObject* jsObject = toJS(object);
    AJValue jsValue = toJS(exec, value);
    
    jsObject->put(exec, propertyIndex, jsValue);
    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
    }
}

bool AJObjectDeleteProperty(AJContextRef ctx, AJObjectRef object, AJStringRef propertyName, AJValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJObject* jsObject = toJS(object);

    bool result = jsObject->deleteProperty(exec, propertyName->identifier(&exec->globalData()));
    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
    }
    return result;
}

void* AJObjectGetPrivate(AJObjectRef object)
{
    AJObject* jsObject = toJS(object);
    
    if (jsObject->inherits(&AJCallbackObject<AJGlobalObject>::info))
        return static_cast<AJCallbackObject<AJGlobalObject>*>(jsObject)->getPrivate();
    else if (jsObject->inherits(&AJCallbackObject<AJObject>::info))
        return static_cast<AJCallbackObject<AJObject>*>(jsObject)->getPrivate();
    
    return 0;
}

bool AJObjectSetPrivate(AJObjectRef object, void* data)
{
    AJObject* jsObject = toJS(object);
    
    if (jsObject->inherits(&AJCallbackObject<AJGlobalObject>::info)) {
        static_cast<AJCallbackObject<AJGlobalObject>*>(jsObject)->setPrivate(data);
        return true;
    } else if (jsObject->inherits(&AJCallbackObject<AJObject>::info)) {
        static_cast<AJCallbackObject<AJObject>*>(jsObject)->setPrivate(data);
        return true;
    }
        
    return false;
}

AJValueRef AJObjectGetPrivateProperty(AJContextRef ctx, AJObjectRef object, AJStringRef propertyName)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);
    AJObject* jsObject = toJS(object);
    AJValue result;
    Identifier name(propertyName->identifier(&exec->globalData()));
    if (jsObject->inherits(&AJCallbackObject<AJGlobalObject>::info))
        result = static_cast<AJCallbackObject<AJGlobalObject>*>(jsObject)->getPrivateProperty(name);
    else if (jsObject->inherits(&AJCallbackObject<AJObject>::info))
        result = static_cast<AJCallbackObject<AJObject>*>(jsObject)->getPrivateProperty(name);
    return toRef(exec, result);
}

bool AJObjectSetPrivateProperty(AJContextRef ctx, AJObjectRef object, AJStringRef propertyName, AJValueRef value)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);
    AJObject* jsObject = toJS(object);
    AJValue jsValue = toJS(exec, value);
    Identifier name(propertyName->identifier(&exec->globalData()));
    if (jsObject->inherits(&AJCallbackObject<AJGlobalObject>::info)) {
        static_cast<AJCallbackObject<AJGlobalObject>*>(jsObject)->setPrivateProperty(name, jsValue);
        return true;
    }
    if (jsObject->inherits(&AJCallbackObject<AJObject>::info)) {
        static_cast<AJCallbackObject<AJObject>*>(jsObject)->setPrivateProperty(name, jsValue);
        return true;
    }
    return false;
}

bool AJObjectDeletePrivateProperty(AJContextRef ctx, AJObjectRef object, AJStringRef propertyName)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);
    AJObject* jsObject = toJS(object);
    Identifier name(propertyName->identifier(&exec->globalData()));
    if (jsObject->inherits(&AJCallbackObject<AJGlobalObject>::info)) {
        static_cast<AJCallbackObject<AJGlobalObject>*>(jsObject)->deletePrivateProperty(name);
        return true;
    }
    if (jsObject->inherits(&AJCallbackObject<AJObject>::info)) {
        static_cast<AJCallbackObject<AJObject>*>(jsObject)->deletePrivateProperty(name);
        return true;
    }
    return false;
}

bool AJObjectIsFunction(AJContextRef, AJObjectRef object)
{
    CallData callData;
    return toJS(object)->getCallData(callData) != CallTypeNone;
}

AJValueRef AJObjectCallAsFunction(AJContextRef ctx, AJObjectRef object, AJObjectRef thisObject, size_t argumentCount, const AJValueRef arguments[], AJValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJObject* jsObject = toJS(object);
    AJObject* jsThisObject = toJS(thisObject);

    if (!jsThisObject)
        jsThisObject = exec->globalThisValue();

    MarkedArgumentBuffer argList;
    for (size_t i = 0; i < argumentCount; i++)
        argList.append(toJS(exec, arguments[i]));

    CallData callData;
    CallType callType = jsObject->getCallData(callData);
    if (callType == CallTypeNone)
        return 0;

    AJValueRef result = toRef(exec, call(exec, jsObject, callType, callData, jsThisObject, argList));
    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
        result = 0;
    }
    return result;
}

bool AJObjectIsConstructor(AJContextRef, AJObjectRef object)
{
    AJObject* jsObject = toJS(object);
    ConstructData constructData;
    return jsObject->getConstructData(constructData) != ConstructTypeNone;
}

AJObjectRef AJObjectCallAsConstructor(AJContextRef ctx, AJObjectRef object, size_t argumentCount, const AJValueRef arguments[], AJValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJObject* jsObject = toJS(object);

    ConstructData constructData;
    ConstructType constructType = jsObject->getConstructData(constructData);
    if (constructType == ConstructTypeNone)
        return 0;

    MarkedArgumentBuffer argList;
    for (size_t i = 0; i < argumentCount; i++)
        argList.append(toJS(exec, arguments[i]));
    AJObjectRef result = toRef(construct(exec, jsObject, constructType, constructData, argList));
    if (exec->hadException()) {
        if (exception)
            *exception = toRef(exec, exec->exception());
        exec->clearException();
        result = 0;
    }
    return result;
}

struct OpaqueAJPropertyNameArray : FastAllocBase {
    OpaqueAJPropertyNameArray(AJGlobalData* globalData)
        : refCount(0)
        , globalData(globalData)
    {
    }
    
    unsigned refCount;
    AJGlobalData* globalData;
    Vector<AJRetainPtr<AJStringRef> > array;
};

AJPropertyNameArrayRef AJObjectCopyPropertyNames(AJContextRef ctx, AJObjectRef object)
{
    AJObject* jsObject = toJS(object);
    ExecState* exec = toJS(ctx);
    APIEntryShim entryShim(exec);

    AJGlobalData* globalData = &exec->globalData();

    AJPropertyNameArrayRef propertyNames = new OpaqueAJPropertyNameArray(globalData);
    PropertyNameArray array(globalData);
    jsObject->getPropertyNames(exec, array);

    size_t size = array.size();
    propertyNames->array.reserveInitialCapacity(size);
    for (size_t i = 0; i < size; ++i)
        propertyNames->array.append(AJRetainPtr<AJStringRef>(Adopt, OpaqueAJString::create(array[i].ustring()).releaseRef()));
    
    return AJPropertyNameArrayRetain(propertyNames);
}

AJPropertyNameArrayRef AJPropertyNameArrayRetain(AJPropertyNameArrayRef array)
{
    ++array->refCount;
    return array;
}

void AJPropertyNameArrayRelease(AJPropertyNameArrayRef array)
{
    if (--array->refCount == 0) {
        APIEntryShim entryShim(array->globalData, false);
        delete array;
    }
}

size_t AJPropertyNameArrayGetCount(AJPropertyNameArrayRef array)
{
    return array->array.size();
}

AJStringRef AJPropertyNameArrayGetNameAtIndex(AJPropertyNameArrayRef array, size_t index)
{
    return array->array[static_cast<unsigned>(index)].get();
}

void AJPropertyNameAccumulatorAddName(AJPropertyNameAccumulatorRef array, AJStringRef propertyName)
{
    PropertyNameArray* propertyNames = toJS(array);
    APIEntryShim entryShim(propertyNames->globalData());
    propertyNames->add(propertyName->identifier(propertyNames->globalData()));
}
