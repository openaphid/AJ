
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
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2008 Apple Inc. All rights reserved.
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
#include "ObjectConstructor.h"

#include "Error.h"
#include "AJFunction.h"
#include "AJArray.h"
#include "AJGlobalObject.h"
#include "ObjectPrototype.h"
#include "PropertyDescriptor.h"
#include "PropertyNameArray.h"
#include "PrototypeFunction.h"

namespace AJ {

ASSERT_CLASS_FITS_IN_CELL(ObjectConstructor);

static AJValue JSC_HOST_CALL objectConstructorGetPrototypeOf(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL objectConstructorGetOwnPropertyDescriptor(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL objectConstructorGetOwnPropertyNames(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL objectConstructorKeys(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL objectConstructorDefineProperty(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL objectConstructorDefineProperties(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL objectConstructorCreate(ExecState*, AJObject*, AJValue, const ArgList&);

ObjectConstructor::ObjectConstructor(ExecState* exec, NonNullPassRefPtr<Structure> structure, ObjectPrototype* objectPrototype, Structure* prototypeFunctionStructure)
: InternalFunction(&exec->globalData(), structure, Identifier(exec, "Object"))
{
    // ECMA 15.2.3.1
    putDirectWithoutTransition(exec->propertyNames().prototype, objectPrototype, DontEnum | DontDelete | ReadOnly);
    
    // no. of arguments for constructor
    putDirectWithoutTransition(exec->propertyNames().length, jsNumber(exec, 1), ReadOnly | DontEnum | DontDelete);
    
    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 1, exec->propertyNames().getPrototypeOf, objectConstructorGetPrototypeOf), DontEnum);
    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 2, exec->propertyNames().getOwnPropertyDescriptor, objectConstructorGetOwnPropertyDescriptor), DontEnum);
    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 1, exec->propertyNames().getOwnPropertyNames, objectConstructorGetOwnPropertyNames), DontEnum);
    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 1, exec->propertyNames().keys, objectConstructorKeys), DontEnum);
    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 3, exec->propertyNames().defineProperty, objectConstructorDefineProperty), DontEnum);
    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 2, exec->propertyNames().defineProperties, objectConstructorDefineProperties), DontEnum);
    putDirectFunctionWithoutTransition(exec, new (exec) NativeFunctionWrapper(exec, prototypeFunctionStructure, 2, exec->propertyNames().create, objectConstructorCreate), DontEnum);
}

// ECMA 15.2.2
static ALWAYS_INLINE AJObject* constructObject(ExecState* exec, const ArgList& args)
{
    AJValue arg = args.at(0);
    if (arg.isUndefinedOrNull())
        return new (exec) AJObject(exec->lexicalGlobalObject()->emptyObjectStructure());
    return arg.toObject(exec);
}

static AJObject* constructWithObjectConstructor(ExecState* exec, AJObject*, const ArgList& args)
{
    return constructObject(exec, args);
}

ConstructType ObjectConstructor::getConstructData(ConstructData& constructData)
{
    constructData.native.function = constructWithObjectConstructor;
    return ConstructTypeHost;
}

static AJValue JSC_HOST_CALL callObjectConstructor(ExecState* exec, AJObject*, AJValue, const ArgList& args)
{
    return constructObject(exec, args);
}

CallType ObjectConstructor::getCallData(CallData& callData)
{
    callData.native.function = callObjectConstructor;
    return CallTypeHost;
}

AJValue JSC_HOST_CALL objectConstructorGetPrototypeOf(ExecState* exec, AJObject*, AJValue, const ArgList& args)
{
    if (!args.at(0).isObject())
        return throwError(exec, TypeError, "Requested prototype of a value that is not an object.");
    return asObject(args.at(0))->prototype();
}

AJValue JSC_HOST_CALL objectConstructorGetOwnPropertyDescriptor(ExecState* exec, AJObject*, AJValue, const ArgList& args)
{
    if (!args.at(0).isObject())
        return throwError(exec, TypeError, "Requested property descriptor of a value that is not an object.");
    UString propertyName = args.at(1).toString(exec);
    if (exec->hadException())
        return jsNull();
    AJObject* object = asObject(args.at(0));
    PropertyDescriptor descriptor;
    if (!object->getOwnPropertyDescriptor(exec, Identifier(exec, propertyName), descriptor))
        return jsUndefined();
    if (exec->hadException())
        return jsUndefined();

    AJObject* description = constructEmptyObject(exec);
    if (!descriptor.isAccessorDescriptor()) {
        description->putDirect(exec->propertyNames().value, descriptor.value() ? descriptor.value() : jsUndefined(), 0);
        description->putDirect(exec->propertyNames().writable, jsBoolean(descriptor.writable()), 0);
    } else {
        description->putDirect(exec->propertyNames().get, descriptor.getter() ? descriptor.getter() : jsUndefined(), 0);
        description->putDirect(exec->propertyNames().set, descriptor.setter() ? descriptor.setter() : jsUndefined(), 0);
    }
    
    description->putDirect(exec->propertyNames().enumerable, jsBoolean(descriptor.enumerable()), 0);
    description->putDirect(exec->propertyNames().configurable, jsBoolean(descriptor.configurable()), 0);

    return description;
}

// FIXME: Use the enumeration cache.
AJValue JSC_HOST_CALL objectConstructorGetOwnPropertyNames(ExecState* exec, AJObject*, AJValue, const ArgList& args)
{
    if (!args.at(0).isObject())
        return throwError(exec, TypeError, "Requested property names of a value that is not an object.");
    PropertyNameArray properties(exec);
    asObject(args.at(0))->getOwnPropertyNames(exec, properties, IncludeDontEnumProperties);
    AJArray* names = constructEmptyArray(exec);
    size_t numProperties = properties.size();
    for (size_t i = 0; i < numProperties; i++)
        names->push(exec, jsOwnedString(exec, properties[i].ustring()));
    return names;
}

// FIXME: Use the enumeration cache.
AJValue JSC_HOST_CALL objectConstructorKeys(ExecState* exec, AJObject*, AJValue, const ArgList& args)
{
    if (!args.at(0).isObject())
        return throwError(exec, TypeError, "Requested keys of a value that is not an object.");
    PropertyNameArray properties(exec);
    asObject(args.at(0))->getOwnPropertyNames(exec, properties);
    AJArray* keys = constructEmptyArray(exec);
    size_t numProperties = properties.size();
    for (size_t i = 0; i < numProperties; i++)
        keys->push(exec, jsOwnedString(exec, properties[i].ustring()));
    return keys;
}

// ES5 8.10.5 ToPropertyDescriptor
static bool toPropertyDescriptor(ExecState* exec, AJValue in, PropertyDescriptor& desc)
{
    if (!in.isObject()) {
        throwError(exec, TypeError, "Property description must be an object.");
        return false;
    }
    AJObject* description = asObject(in);

    PropertySlot enumerableSlot(description);
    if (description->getPropertySlot(exec, exec->propertyNames().enumerable, enumerableSlot)) {
        desc.setEnumerable(enumerableSlot.getValue(exec, exec->propertyNames().enumerable).toBoolean(exec));
        if (exec->hadException())
            return false;
    }

    PropertySlot configurableSlot(description);
    if (description->getPropertySlot(exec, exec->propertyNames().configurable, configurableSlot)) {
        desc.setConfigurable(configurableSlot.getValue(exec, exec->propertyNames().configurable).toBoolean(exec));
        if (exec->hadException())
            return false;
    }

    AJValue value;
    PropertySlot valueSlot(description);
    if (description->getPropertySlot(exec, exec->propertyNames().value, valueSlot)) {
        desc.setValue(valueSlot.getValue(exec, exec->propertyNames().value));
        if (exec->hadException())
            return false;
    }

    PropertySlot writableSlot(description);
    if (description->getPropertySlot(exec, exec->propertyNames().writable, writableSlot)) {
        desc.setWritable(writableSlot.getValue(exec, exec->propertyNames().writable).toBoolean(exec));
        if (exec->hadException())
            return false;
    }

    PropertySlot getSlot(description);
    if (description->getPropertySlot(exec, exec->propertyNames().get, getSlot)) {
        AJValue get = getSlot.getValue(exec, exec->propertyNames().get);
        if (exec->hadException())
            return false;
        if (!get.isUndefined()) {
            CallData callData;
            if (get.getCallData(callData) == CallTypeNone) {
                throwError(exec, TypeError, "Getter must be a function.");
                return false;
            }
        } else
            get = AJValue();
        desc.setGetter(get);
    }

    PropertySlot setSlot(description);
    if (description->getPropertySlot(exec, exec->propertyNames().set, setSlot)) {
        AJValue set = setSlot.getValue(exec, exec->propertyNames().set);
        if (exec->hadException())
            return false;
        if (!set.isUndefined()) {
            CallData callData;
            if (set.getCallData(callData) == CallTypeNone) {
                throwError(exec, TypeError, "Setter must be a function.");
                return false;
            }
        } else
            set = AJValue();

        desc.setSetter(set);
    }

    if (!desc.isAccessorDescriptor())
        return true;

    if (desc.value()) {
        throwError(exec, TypeError, "Invalid property.  'value' present on property with getter or setter.");
        return false;
    }

    if (desc.writablePresent()) {
        throwError(exec, TypeError, "Invalid property.  'writable' present on property with getter or setter.");
        return false;
    }
    return true;
}

AJValue JSC_HOST_CALL objectConstructorDefineProperty(ExecState* exec, AJObject*, AJValue, const ArgList& args)
{
    if (!args.at(0).isObject())
        return throwError(exec, TypeError, "Properties can only be defined on Objects.");
    AJObject* O = asObject(args.at(0));
    UString propertyName = args.at(1).toString(exec);
    if (exec->hadException())
        return jsNull();
    PropertyDescriptor descriptor;
    if (!toPropertyDescriptor(exec, args.at(2), descriptor))
        return jsNull();
    ASSERT((descriptor.attributes() & (Getter | Setter)) || (!descriptor.isAccessorDescriptor()));
    ASSERT(!exec->hadException());
    O->defineOwnProperty(exec, Identifier(exec, propertyName), descriptor, true);
    return O;
}

static AJValue defineProperties(ExecState* exec, AJObject* object, AJObject* properties)
{
    PropertyNameArray propertyNames(exec);
    asObject(properties)->getOwnPropertyNames(exec, propertyNames);
    size_t numProperties = propertyNames.size();
    Vector<PropertyDescriptor> descriptors;
    MarkedArgumentBuffer markBuffer;
    for (size_t i = 0; i < numProperties; i++) {
        PropertySlot slot;
        AJValue prop = properties->get(exec, propertyNames[i]);
        if (exec->hadException())
            return jsNull();
        PropertyDescriptor descriptor;
        if (!toPropertyDescriptor(exec, prop, descriptor))
            return jsNull();
        descriptors.append(descriptor);
        // Ensure we mark all the values that we're accumulating
        if (descriptor.isDataDescriptor() && descriptor.value())
            markBuffer.append(descriptor.value());
        if (descriptor.isAccessorDescriptor()) {
            if (descriptor.getter())
                markBuffer.append(descriptor.getter());
            if (descriptor.setter())
                markBuffer.append(descriptor.setter());
        }
    }
    for (size_t i = 0; i < numProperties; i++) {
        object->defineOwnProperty(exec, propertyNames[i], descriptors[i], true);
        if (exec->hadException())
            return jsNull();
    }
    return object;
}

AJValue JSC_HOST_CALL objectConstructorDefineProperties(ExecState* exec, AJObject*, AJValue, const ArgList& args)
{
    if (!args.at(0).isObject())
        return throwError(exec, TypeError, "Properties can only be defined on Objects.");
    if (!args.at(1).isObject())
        return throwError(exec, TypeError, "Property descriptor list must be an Object.");
    return defineProperties(exec, asObject(args.at(0)), asObject(args.at(1)));
}

AJValue JSC_HOST_CALL objectConstructorCreate(ExecState* exec, AJObject*, AJValue, const ArgList& args)
{
    if (!args.at(0).isObject() && !args.at(0).isNull())
        return throwError(exec, TypeError, "Object prototype may only be an Object or null.");
    AJObject* newObject = constructEmptyObject(exec);
    newObject->setPrototype(args.at(0));
    if (args.at(1).isUndefined())
        return newObject;
    if (!args.at(1).isObject())
        return throwError(exec, TypeError, "Property descriptor list must be an Object.");
    return defineProperties(exec, newObject, asObject(args.at(1)));
}

} // namespace AJ
