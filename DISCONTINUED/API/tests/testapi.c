
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

#include "AJCore.h"
#include "AJBasePrivate.h"
#include "AJContextRefPrivate.h"
#include "AJObjectRefPrivate.h"
#include <math.h>
#define ASSERT_DISABLED 0
#include <wtf/Assertions.h>
#include <wtf/UnusedParam.h>

#if COMPILER(MSVC)

#include <wtf/MathExtras.h>

static double nan(const char*)
{
    return std::numeric_limits<double>::quiet_NaN();
}

#endif

static AJGlobalContextRef context;
static int failed;
static void assertEqualsAsBoolean(AJValueRef value, bool expectedValue)
{
    if (AJValueToBoolean(context, value) != expectedValue) {
        fprintf(stderr, "assertEqualsAsBoolean failed: %p, %d\n", value, expectedValue);
        failed = 1;
    }
}

static void assertEqualsAsNumber(AJValueRef value, double expectedValue)
{
    double number = AJValueToNumber(context, value, NULL);

    // FIXME <rdar://4668451> - On i386 the isnan(double) macro tries to map to the isnan(float) function,
    // causing a build break with -Wshorten-64-to-32 enabled.  The issue is known by the appropriate team.
    // After that's resolved, we can remove these casts
    if (number != expectedValue && !(isnan((float)number) && isnan((float)expectedValue))) {
        fprintf(stderr, "assertEqualsAsNumber failed: %p, %lf\n", value, expectedValue);
        failed = 1;
    }
}

static void assertEqualsAsUTF8String(AJValueRef value, const char* expectedValue)
{
    AJStringRef valueAsString = AJValueToStringCopy(context, value, NULL);

    size_t jsSize = AJStringGetMaximumUTF8CStringSize(valueAsString);
    char* jsBuffer = (char*)malloc(jsSize);
    AJStringGetUTF8CString(valueAsString, jsBuffer, jsSize);
    
    unsigned i;
    for (i = 0; jsBuffer[i]; i++) {
        if (jsBuffer[i] != expectedValue[i]) {
            fprintf(stderr, "assertEqualsAsUTF8String failed at character %d: %c(%d) != %c(%d)\n", i, jsBuffer[i], jsBuffer[i], expectedValue[i], expectedValue[i]);
            failed = 1;
        }
    }

    if (jsSize < strlen(jsBuffer) + 1) {
        fprintf(stderr, "assertEqualsAsUTF8String failed: jsSize was too small\n");
        failed = 1;
    }

    free(jsBuffer);
    AJStringRelease(valueAsString);
}

static void assertEqualsAsCharactersPtr(AJValueRef value, const char* expectedValue)
{
    AJStringRef valueAsString = AJValueToStringCopy(context, value, NULL);

    size_t jsLength = AJStringGetLength(valueAsString);
    const AJChar* jsBuffer = AJStringGetCharactersPtr(valueAsString);

    CFStringRef expectedValueAsCFString = CFStringCreateWithCString(kCFAllocatorDefault, 
                                                                    expectedValue,
                                                                    kCFStringEncodingUTF8);    
    CFIndex cfLength = CFStringGetLength(expectedValueAsCFString);
    UniChar* cfBuffer = (UniChar*)malloc(cfLength * sizeof(UniChar));
    CFStringGetCharacters(expectedValueAsCFString, CFRangeMake(0, cfLength), cfBuffer);
    CFRelease(expectedValueAsCFString);

    if (memcmp(jsBuffer, cfBuffer, cfLength * sizeof(UniChar)) != 0) {
        fprintf(stderr, "assertEqualsAsCharactersPtr failed: jsBuffer != cfBuffer\n");
        failed = 1;
    }
    
    if (jsLength != (size_t)cfLength) {
        fprintf(stderr, "assertEqualsAsCharactersPtr failed: jsLength(%ld) != cfLength(%ld)\n", jsLength, cfLength);
        failed = 1;
    }

    free(cfBuffer);
    AJStringRelease(valueAsString);
}

static bool timeZoneIsPST()
{
    char timeZoneName[70];
    struct tm gtm;
    memset(&gtm, 0, sizeof(gtm));
    strftime(timeZoneName, sizeof(timeZoneName), "%Z", &gtm);

    return 0 == strcmp("PST", timeZoneName);
}

static AJValueRef jsGlobalValue; // non-stack value for testing AJValueProtect()

/* MyObject pseudo-class */

static bool MyObject_hasProperty(AJContextRef context, AJObjectRef object, AJStringRef propertyName)
{
    UNUSED_PARAM(context);
    UNUSED_PARAM(object);

    if (AJStringIsEqualToUTF8CString(propertyName, "alwaysOne")
        || AJStringIsEqualToUTF8CString(propertyName, "cantFind")
        || AJStringIsEqualToUTF8CString(propertyName, "throwOnGet")
        || AJStringIsEqualToUTF8CString(propertyName, "myPropertyName")
        || AJStringIsEqualToUTF8CString(propertyName, "hasPropertyLie")
        || AJStringIsEqualToUTF8CString(propertyName, "0")) {
        return true;
    }
    
    return false;
}

static AJValueRef MyObject_getProperty(AJContextRef context, AJObjectRef object, AJStringRef propertyName, AJValueRef* exception)
{
    UNUSED_PARAM(context);
    UNUSED_PARAM(object);
    
    if (AJStringIsEqualToUTF8CString(propertyName, "alwaysOne")) {
        return AJValueMakeNumber(context, 1);
    }
    
    if (AJStringIsEqualToUTF8CString(propertyName, "myPropertyName")) {
        return AJValueMakeNumber(context, 1);
    }

    if (AJStringIsEqualToUTF8CString(propertyName, "cantFind")) {
        return AJValueMakeUndefined(context);
    }
    
    if (AJStringIsEqualToUTF8CString(propertyName, "hasPropertyLie")) {
        return 0;
    }

    if (AJStringIsEqualToUTF8CString(propertyName, "throwOnGet")) {
        return AJEvaluateScript(context, AJStringCreateWithUTF8CString("throw 'an exception'"), object, AJStringCreateWithUTF8CString("test script"), 1, exception);
    }

    if (AJStringIsEqualToUTF8CString(propertyName, "0")) {
        *exception = AJValueMakeNumber(context, 1);
        return AJValueMakeNumber(context, 1);
    }
    
    return AJValueMakeNull(context);
}

static bool MyObject_setProperty(AJContextRef context, AJObjectRef object, AJStringRef propertyName, AJValueRef value, AJValueRef* exception)
{
    UNUSED_PARAM(context);
    UNUSED_PARAM(object);
    UNUSED_PARAM(value);
    UNUSED_PARAM(exception);

    if (AJStringIsEqualToUTF8CString(propertyName, "cantSet"))
        return true; // pretend we set the property in order to swallow it
    
    if (AJStringIsEqualToUTF8CString(propertyName, "throwOnSet")) {
        AJEvaluateScript(context, AJStringCreateWithUTF8CString("throw 'an exception'"), object, AJStringCreateWithUTF8CString("test script"), 1, exception);
    }
    
    return false;
}

static bool MyObject_deleteProperty(AJContextRef context, AJObjectRef object, AJStringRef propertyName, AJValueRef* exception)
{
    UNUSED_PARAM(context);
    UNUSED_PARAM(object);
    
    if (AJStringIsEqualToUTF8CString(propertyName, "cantDelete"))
        return true;
    
    if (AJStringIsEqualToUTF8CString(propertyName, "throwOnDelete")) {
        AJEvaluateScript(context, AJStringCreateWithUTF8CString("throw 'an exception'"), object, AJStringCreateWithUTF8CString("test script"), 1, exception);
        return false;
    }

    return false;
}

static void MyObject_getPropertyNames(AJContextRef context, AJObjectRef object, AJPropertyNameAccumulatorRef propertyNames)
{
    UNUSED_PARAM(context);
    UNUSED_PARAM(object);
    
    AJStringRef propertyName;
    
    propertyName = AJStringCreateWithUTF8CString("alwaysOne");
    AJPropertyNameAccumulatorAddName(propertyNames, propertyName);
    AJStringRelease(propertyName);
    
    propertyName = AJStringCreateWithUTF8CString("myPropertyName");
    AJPropertyNameAccumulatorAddName(propertyNames, propertyName);
    AJStringRelease(propertyName);
}

static AJValueRef MyObject_callAsFunction(AJContextRef context, AJObjectRef object, AJObjectRef thisObject, size_t argumentCount, const AJValueRef arguments[], AJValueRef* exception)
{
    UNUSED_PARAM(context);
    UNUSED_PARAM(object);
    UNUSED_PARAM(thisObject);
    UNUSED_PARAM(exception);

    if (argumentCount > 0 && AJValueIsString(context, arguments[0]) && AJStringIsEqualToUTF8CString(AJValueToStringCopy(context, arguments[0], 0), "throwOnCall")) {
        AJEvaluateScript(context, AJStringCreateWithUTF8CString("throw 'an exception'"), object, AJStringCreateWithUTF8CString("test script"), 1, exception);
        return AJValueMakeUndefined(context);
    }

    if (argumentCount > 0 && AJValueIsStrictEqual(context, arguments[0], AJValueMakeNumber(context, 0)))
        return AJValueMakeNumber(context, 1);
    
    return AJValueMakeUndefined(context);
}

static AJObjectRef MyObject_callAsConstructor(AJContextRef context, AJObjectRef object, size_t argumentCount, const AJValueRef arguments[], AJValueRef* exception)
{
    UNUSED_PARAM(context);
    UNUSED_PARAM(object);

    if (argumentCount > 0 && AJValueIsString(context, arguments[0]) && AJStringIsEqualToUTF8CString(AJValueToStringCopy(context, arguments[0], 0), "throwOnConstruct")) {
        AJEvaluateScript(context, AJStringCreateWithUTF8CString("throw 'an exception'"), object, AJStringCreateWithUTF8CString("test script"), 1, exception);
        return object;
    }

    if (argumentCount > 0 && AJValueIsStrictEqual(context, arguments[0], AJValueMakeNumber(context, 0)))
        return AJValueToObject(context, AJValueMakeNumber(context, 1), exception);
    
    return AJValueToObject(context, AJValueMakeNumber(context, 0), exception);
}

static bool MyObject_hasInstance(AJContextRef context, AJObjectRef constructor, AJValueRef possibleValue, AJValueRef* exception)
{
    UNUSED_PARAM(context);
    UNUSED_PARAM(constructor);

    if (AJValueIsString(context, possibleValue) && AJStringIsEqualToUTF8CString(AJValueToStringCopy(context, possibleValue, 0), "throwOnHasInstance")) {
        AJEvaluateScript(context, AJStringCreateWithUTF8CString("throw 'an exception'"), constructor, AJStringCreateWithUTF8CString("test script"), 1, exception);
        return false;
    }

    AJStringRef numberString = AJStringCreateWithUTF8CString("Number");
    AJObjectRef numberConstructor = AJValueToObject(context, AJObjectGetProperty(context, AJContextGetGlobalObject(context), numberString, exception), exception);
    AJStringRelease(numberString);

    return AJValueIsInstanceOfConstructor(context, possibleValue, numberConstructor, exception);
}

static AJValueRef MyObject_convertToType(AJContextRef context, AJObjectRef object, AJType type, AJValueRef* exception)
{
    UNUSED_PARAM(object);
    UNUSED_PARAM(exception);
    
    switch (type) {
    case kAJTypeNumber:
        return AJValueMakeNumber(context, 1);
    case kAJTypeString:
        {
            AJStringRef string = AJStringCreateWithUTF8CString("MyObjectAsString");
            AJValueRef result = AJValueMakeString(context, string);
            AJStringRelease(string);
            return result;
        }
    default:
        break;
    }

    // string conversion -- forward to default object class
    return AJValueMakeNull(context);
}

static AJStaticValue evilStaticValues[] = {
    { "nullGetSet", 0, 0, kAJPropertyAttributeNone },
    { 0, 0, 0, 0 }
};

static AJStaticFunction evilStaticFunctions[] = {
    { "nullCall", 0, kAJPropertyAttributeNone },
    { 0, 0, 0 }
};

AJClassDefinition MyObject_definition = {
    0,
    kAJClassAttributeNone,
    
    "MyObject",
    NULL,
    
    evilStaticValues,
    evilStaticFunctions,
    
    NULL,
    NULL,
    MyObject_hasProperty,
    MyObject_getProperty,
    MyObject_setProperty,
    MyObject_deleteProperty,
    MyObject_getPropertyNames,
    MyObject_callAsFunction,
    MyObject_callAsConstructor,
    MyObject_hasInstance,
    MyObject_convertToType,
};

static AJClassRef MyObject_class(AJContextRef context)
{
    UNUSED_PARAM(context);

    static AJClassRef jsClass;
    if (!jsClass)
        jsClass = AJClassCreate(&MyObject_definition);
    
    return jsClass;
}

static bool EvilExceptionObject_hasInstance(AJContextRef context, AJObjectRef constructor, AJValueRef possibleValue, AJValueRef* exception)
{
    UNUSED_PARAM(context);
    UNUSED_PARAM(constructor);
    
    AJStringRef hasInstanceName = AJStringCreateWithUTF8CString("hasInstance");
    AJValueRef hasInstance = AJObjectGetProperty(context, constructor, hasInstanceName, exception);
    AJStringRelease(hasInstanceName);
    if (!hasInstance)
        return false;
    AJObjectRef function = AJValueToObject(context, hasInstance, exception);
    AJValueRef result = AJObjectCallAsFunction(context, function, constructor, 1, &possibleValue, exception);
    return result && AJValueToBoolean(context, result);
}

static AJValueRef EvilExceptionObject_convertToType(AJContextRef context, AJObjectRef object, AJType type, AJValueRef* exception)
{
    UNUSED_PARAM(object);
    UNUSED_PARAM(exception);
    AJStringRef funcName;
    switch (type) {
    case kAJTypeNumber:
        funcName = AJStringCreateWithUTF8CString("toNumber");
        break;
    case kAJTypeString:
        funcName = AJStringCreateWithUTF8CString("toStringExplicit");
        break;
    default:
        return AJValueMakeNull(context);
        break;
    }
    
    AJValueRef func = AJObjectGetProperty(context, object, funcName, exception);
    AJStringRelease(funcName);    
    AJObjectRef function = AJValueToObject(context, func, exception);
    if (!function)
        return AJValueMakeNull(context);
    AJValueRef value = AJObjectCallAsFunction(context, function, object, 0, NULL, exception);
    if (!value) {
        AJStringRef errorString = AJStringCreateWithUTF8CString("convertToType failed"); 
        AJValueRef errorStringRef = AJValueMakeString(context, errorString);
        AJStringRelease(errorString);
        return errorStringRef;
    }
    return value;
}

AJClassDefinition EvilExceptionObject_definition = {
    0,
    kAJClassAttributeNone,

    "EvilExceptionObject",
    NULL,

    NULL,
    NULL,

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    EvilExceptionObject_hasInstance,
    EvilExceptionObject_convertToType,
};

static AJClassRef EvilExceptionObject_class(AJContextRef context)
{
    UNUSED_PARAM(context);
    
    static AJClassRef jsClass;
    if (!jsClass)
        jsClass = AJClassCreate(&EvilExceptionObject_definition);
    
    return jsClass;
}

AJClassDefinition EmptyObject_definition = {
    0,
    kAJClassAttributeNone,
    
    NULL,
    NULL,
    
    NULL,
    NULL,
    
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

static AJClassRef EmptyObject_class(AJContextRef context)
{
    UNUSED_PARAM(context);
    
    static AJClassRef jsClass;
    if (!jsClass)
        jsClass = AJClassCreate(&EmptyObject_definition);
    
    return jsClass;
}


static AJValueRef Base_get(AJContextRef ctx, AJObjectRef object, AJStringRef propertyName, AJValueRef* exception)
{
    UNUSED_PARAM(object);
    UNUSED_PARAM(propertyName);
    UNUSED_PARAM(exception);

    return AJValueMakeNumber(ctx, 1); // distinguish base get form derived get
}

static bool Base_set(AJContextRef ctx, AJObjectRef object, AJStringRef propertyName, AJValueRef value, AJValueRef* exception)
{
    UNUSED_PARAM(object);
    UNUSED_PARAM(propertyName);
    UNUSED_PARAM(value);

    *exception = AJValueMakeNumber(ctx, 1); // distinguish base set from derived set
    return true;
}

static AJValueRef Base_callAsFunction(AJContextRef ctx, AJObjectRef function, AJObjectRef thisObject, size_t argumentCount, const AJValueRef arguments[], AJValueRef* exception)
{
    UNUSED_PARAM(function);
    UNUSED_PARAM(thisObject);
    UNUSED_PARAM(argumentCount);
    UNUSED_PARAM(arguments);
    UNUSED_PARAM(exception);
    
    return AJValueMakeNumber(ctx, 1); // distinguish base call from derived call
}

static AJStaticFunction Base_staticFunctions[] = {
    { "baseProtoDup", NULL, kAJPropertyAttributeNone },
    { "baseProto", Base_callAsFunction, kAJPropertyAttributeNone },
    { 0, 0, 0 }
};

static AJStaticValue Base_staticValues[] = {
    { "baseDup", Base_get, Base_set, kAJPropertyAttributeNone },
    { "baseOnly", Base_get, Base_set, kAJPropertyAttributeNone },
    { 0, 0, 0, 0 }
};

static bool TestInitializeFinalize;
static void Base_initialize(AJContextRef context, AJObjectRef object)
{
    UNUSED_PARAM(context);

    if (TestInitializeFinalize) {
        ASSERT((void*)1 == AJObjectGetPrivate(object));
        AJObjectSetPrivate(object, (void*)2);
    }
}

static unsigned Base_didFinalize;
static void Base_finalize(AJObjectRef object)
{
    UNUSED_PARAM(object);
    if (TestInitializeFinalize) {
        ASSERT((void*)4 == AJObjectGetPrivate(object));
        Base_didFinalize = true;
    }
}

static AJClassRef Base_class(AJContextRef context)
{
    UNUSED_PARAM(context);

    static AJClassRef jsClass;
    if (!jsClass) {
        AJClassDefinition definition = kAJClassDefinitionEmpty;
        definition.staticValues = Base_staticValues;
        definition.staticFunctions = Base_staticFunctions;
        definition.initialize = Base_initialize;
        definition.finalize = Base_finalize;
        jsClass = AJClassCreate(&definition);
    }
    return jsClass;
}

static AJValueRef Derived_get(AJContextRef ctx, AJObjectRef object, AJStringRef propertyName, AJValueRef* exception)
{
    UNUSED_PARAM(object);
    UNUSED_PARAM(propertyName);
    UNUSED_PARAM(exception);

    return AJValueMakeNumber(ctx, 2); // distinguish base get form derived get
}

static bool Derived_set(AJContextRef ctx, AJObjectRef object, AJStringRef propertyName, AJValueRef value, AJValueRef* exception)
{
    UNUSED_PARAM(ctx);
    UNUSED_PARAM(object);
    UNUSED_PARAM(propertyName);
    UNUSED_PARAM(value);

    *exception = AJValueMakeNumber(ctx, 2); // distinguish base set from derived set
    return true;
}

static AJValueRef Derived_callAsFunction(AJContextRef ctx, AJObjectRef function, AJObjectRef thisObject, size_t argumentCount, const AJValueRef arguments[], AJValueRef* exception)
{
    UNUSED_PARAM(function);
    UNUSED_PARAM(thisObject);
    UNUSED_PARAM(argumentCount);
    UNUSED_PARAM(arguments);
    UNUSED_PARAM(exception);
    
    return AJValueMakeNumber(ctx, 2); // distinguish base call from derived call
}

static AJStaticFunction Derived_staticFunctions[] = {
    { "protoOnly", Derived_callAsFunction, kAJPropertyAttributeNone },
    { "protoDup", NULL, kAJPropertyAttributeNone },
    { "baseProtoDup", Derived_callAsFunction, kAJPropertyAttributeNone },
    { 0, 0, 0 }
};

static AJStaticValue Derived_staticValues[] = {
    { "derivedOnly", Derived_get, Derived_set, kAJPropertyAttributeNone },
    { "protoDup", Derived_get, Derived_set, kAJPropertyAttributeNone },
    { "baseDup", Derived_get, Derived_set, kAJPropertyAttributeNone },
    { 0, 0, 0, 0 }
};

static void Derived_initialize(AJContextRef context, AJObjectRef object)
{
    UNUSED_PARAM(context);

    if (TestInitializeFinalize) {
        ASSERT((void*)2 == AJObjectGetPrivate(object));
        AJObjectSetPrivate(object, (void*)3);
    }
}

static void Derived_finalize(AJObjectRef object)
{
    if (TestInitializeFinalize) {
        ASSERT((void*)3 == AJObjectGetPrivate(object));
        AJObjectSetPrivate(object, (void*)4);
    }
}

static AJClassRef Derived_class(AJContextRef context)
{
    static AJClassRef jsClass;
    if (!jsClass) {
        AJClassDefinition definition = kAJClassDefinitionEmpty;
        definition.parentClass = Base_class(context);
        definition.staticValues = Derived_staticValues;
        definition.staticFunctions = Derived_staticFunctions;
        definition.initialize = Derived_initialize;
        definition.finalize = Derived_finalize;
        jsClass = AJClassCreate(&definition);
    }
    return jsClass;
}

static AJClassRef Derived2_class(AJContextRef context)
{
    static AJClassRef jsClass;
    if (!jsClass) {
        AJClassDefinition definition = kAJClassDefinitionEmpty;
        definition.parentClass = Derived_class(context);
        jsClass = AJClassCreate(&definition);
    }
    return jsClass;
}

static AJValueRef print_callAsFunction(AJContextRef ctx, AJObjectRef functionObject, AJObjectRef thisObject, size_t argumentCount, const AJValueRef arguments[], AJValueRef* exception)
{
    UNUSED_PARAM(functionObject);
    UNUSED_PARAM(thisObject);
    UNUSED_PARAM(exception);

    ASSERT(AJContextGetGlobalContext(ctx) == context);
    
    if (argumentCount > 0) {
        AJStringRef string = AJValueToStringCopy(ctx, arguments[0], NULL);
        size_t sizeUTF8 = AJStringGetMaximumUTF8CStringSize(string);
        char* stringUTF8 = (char*)malloc(sizeUTF8);
        AJStringGetUTF8CString(string, stringUTF8, sizeUTF8);
        printf("%s\n", stringUTF8);
        free(stringUTF8);
        AJStringRelease(string);
    }
    
    return AJValueMakeUndefined(ctx);
}

static AJObjectRef myConstructor_callAsConstructor(AJContextRef context, AJObjectRef constructorObject, size_t argumentCount, const AJValueRef arguments[], AJValueRef* exception)
{
    UNUSED_PARAM(constructorObject);
    UNUSED_PARAM(exception);
    
    AJObjectRef result = AJObjectMake(context, NULL, NULL);
    if (argumentCount > 0) {
        AJStringRef value = AJStringCreateWithUTF8CString("value");
        AJObjectSetProperty(context, result, value, arguments[0], kAJPropertyAttributeNone, NULL);
        AJStringRelease(value);
    }
    
    return result;
}


static void globalObject_initialize(AJContextRef context, AJObjectRef object)
{
    UNUSED_PARAM(object);
    // Ensure that an execution context is passed in
    ASSERT(context);

    // Ensure that the global object is set to the object that we were passed
    AJObjectRef globalObject = AJContextGetGlobalObject(context);
    ASSERT(globalObject);
    ASSERT(object == globalObject);

    // Ensure that the standard global properties have been set on the global object
    AJStringRef array = AJStringCreateWithUTF8CString("Array");
    AJObjectRef arrayConstructor = AJValueToObject(context, AJObjectGetProperty(context, globalObject, array, NULL), NULL);
    AJStringRelease(array);

    UNUSED_PARAM(arrayConstructor);
    ASSERT(arrayConstructor);
}

static AJValueRef globalObject_get(AJContextRef ctx, AJObjectRef object, AJStringRef propertyName, AJValueRef* exception)
{
    UNUSED_PARAM(object);
    UNUSED_PARAM(propertyName);
    UNUSED_PARAM(exception);

    return AJValueMakeNumber(ctx, 3);
}

static bool globalObject_set(AJContextRef ctx, AJObjectRef object, AJStringRef propertyName, AJValueRef value, AJValueRef* exception)
{
    UNUSED_PARAM(object);
    UNUSED_PARAM(propertyName);
    UNUSED_PARAM(value);

    *exception = AJValueMakeNumber(ctx, 3);
    return true;
}

static AJValueRef globalObject_call(AJContextRef ctx, AJObjectRef function, AJObjectRef thisObject, size_t argumentCount, const AJValueRef arguments[], AJValueRef* exception)
{
    UNUSED_PARAM(function);
    UNUSED_PARAM(thisObject);
    UNUSED_PARAM(argumentCount);
    UNUSED_PARAM(arguments);
    UNUSED_PARAM(exception);

    return AJValueMakeNumber(ctx, 3);
}

static AJValueRef functionGC(AJContextRef context, AJObjectRef function, AJObjectRef thisObject, size_t argumentCount, const AJValueRef arguments[], AJValueRef* exception)
{
    UNUSED_PARAM(function);
    UNUSED_PARAM(thisObject);
    UNUSED_PARAM(argumentCount);
    UNUSED_PARAM(arguments);
    UNUSED_PARAM(exception);
    AJGarbageCollect(context);
    return AJValueMakeUndefined(context);
}

static AJStaticValue globalObject_staticValues[] = {
    { "globalStaticValue", globalObject_get, globalObject_set, kAJPropertyAttributeNone },
    { 0, 0, 0, 0 }
};

static AJStaticFunction globalObject_staticFunctions[] = {
    { "globalStaticFunction", globalObject_call, kAJPropertyAttributeNone },
    { "gc", functionGC, kAJPropertyAttributeNone },
    { 0, 0, 0 }
};

static char* createStringWithContentsOfFile(const char* fileName);

static void testInitializeFinalize()
{
    AJObjectRef o = AJObjectMake(context, Derived_class(context), (void*)1);
    UNUSED_PARAM(o);
    ASSERT(AJObjectGetPrivate(o) == (void*)3);
}

static AJValueRef jsNumberValue =  NULL;

static AJObjectRef aHeapRef = NULL;

static void makeGlobalNumberValue(AJContextRef context) {
    AJValueRef v = AJValueMakeNumber(context, 420);
    AJValueProtect(context, v);
    jsNumberValue = v;
    v = NULL;
}

int main(int argc, char* argv[])
{
    const char *scriptPath = "testapi.js";
    if (argc > 1) {
        scriptPath = argv[1];
    }
    
    // Test garbage collection with a fresh context
    context = AJGlobalContextCreateInGroup(NULL, NULL);
    TestInitializeFinalize = true;
    testInitializeFinalize();
    AJGlobalContextRelease(context);
    TestInitializeFinalize = false;

    ASSERT(Base_didFinalize);

    AJClassDefinition globalObjectClassDefinition = kAJClassDefinitionEmpty;
    globalObjectClassDefinition.initialize = globalObject_initialize;
    globalObjectClassDefinition.staticValues = globalObject_staticValues;
    globalObjectClassDefinition.staticFunctions = globalObject_staticFunctions;
    globalObjectClassDefinition.attributes = kAJClassAttributeNoAutomaticPrototype;
    AJClassRef globalObjectClass = AJClassCreate(&globalObjectClassDefinition);
    context = AJGlobalContextCreateInGroup(NULL, globalObjectClass);

    AJGlobalContextRetain(context);
    AJGlobalContextRelease(context);
    ASSERT(AJContextGetGlobalContext(context) == context);
    
    JSReportExtraMemoryCost(context, 0);
    JSReportExtraMemoryCost(context, 1);
    JSReportExtraMemoryCost(context, 1024);

    AJObjectRef globalObject = AJContextGetGlobalObject(context);
    ASSERT(AJValueIsObject(context, globalObject));
    
    AJValueRef jsUndefined = AJValueMakeUndefined(context);
    AJValueRef jsNull = AJValueMakeNull(context);
    AJValueRef jsTrue = AJValueMakeBoolean(context, true);
    AJValueRef jsFalse = AJValueMakeBoolean(context, false);
    AJValueRef jsZero = AJValueMakeNumber(context, 0);
    AJValueRef jsOne = AJValueMakeNumber(context, 1);
    AJValueRef jsOneThird = AJValueMakeNumber(context, 1.0 / 3.0);
    AJObjectRef jsObjectNoProto = AJObjectMake(context, NULL, NULL);
    AJObjectSetPrototype(context, jsObjectNoProto, AJValueMakeNull(context));

    // FIXME: test funny utf8 characters
    AJStringRef jsEmptyIString = AJStringCreateWithUTF8CString("");
    AJValueRef jsEmptyString = AJValueMakeString(context, jsEmptyIString);
    
    AJStringRef jsOneIString = AJStringCreateWithUTF8CString("1");
    AJValueRef jsOneString = AJValueMakeString(context, jsOneIString);

    UniChar singleUniChar = 65; // Capital A
    CFMutableStringRef cfString = 
        CFStringCreateMutableWithExternalCharactersNoCopy(kCFAllocatorDefault,
                                                          &singleUniChar,
                                                          1,
                                                          1,
                                                          kCFAllocatorNull);

    AJStringRef jsCFIString = AJStringCreateWithCFString(cfString);
    AJValueRef jsCFString = AJValueMakeString(context, jsCFIString);
    
    CFStringRef cfEmptyString = CFStringCreateWithCString(kCFAllocatorDefault, "", kCFStringEncodingUTF8);
    
    AJStringRef jsCFEmptyIString = AJStringCreateWithCFString(cfEmptyString);
    AJValueRef jsCFEmptyString = AJValueMakeString(context, jsCFEmptyIString);

    CFIndex cfStringLength = CFStringGetLength(cfString);
    UniChar* buffer = (UniChar*)malloc(cfStringLength * sizeof(UniChar));
    CFStringGetCharacters(cfString, 
                          CFRangeMake(0, cfStringLength), 
                          buffer);
    AJStringRef jsCFIStringWithCharacters = AJStringCreateWithCharacters((AJChar*)buffer, cfStringLength);
    AJValueRef jsCFStringWithCharacters = AJValueMakeString(context, jsCFIStringWithCharacters);
    
    AJStringRef jsCFEmptyIStringWithCharacters = AJStringCreateWithCharacters((AJChar*)buffer, CFStringGetLength(cfEmptyString));
    free(buffer);
    AJValueRef jsCFEmptyStringWithCharacters = AJValueMakeString(context, jsCFEmptyIStringWithCharacters);

    ASSERT(AJValueGetType(context, jsUndefined) == kAJTypeUndefined);
    ASSERT(AJValueGetType(context, jsNull) == kAJTypeNull);
    ASSERT(AJValueGetType(context, jsTrue) == kAJTypeBoolean);
    ASSERT(AJValueGetType(context, jsFalse) == kAJTypeBoolean);
    ASSERT(AJValueGetType(context, jsZero) == kAJTypeNumber);
    ASSERT(AJValueGetType(context, jsOne) == kAJTypeNumber);
    ASSERT(AJValueGetType(context, jsOneThird) == kAJTypeNumber);
    ASSERT(AJValueGetType(context, jsEmptyString) == kAJTypeString);
    ASSERT(AJValueGetType(context, jsOneString) == kAJTypeString);
    ASSERT(AJValueGetType(context, jsCFString) == kAJTypeString);
    ASSERT(AJValueGetType(context, jsCFStringWithCharacters) == kAJTypeString);
    ASSERT(AJValueGetType(context, jsCFEmptyString) == kAJTypeString);
    ASSERT(AJValueGetType(context, jsCFEmptyStringWithCharacters) == kAJTypeString);

    AJObjectRef myObject = AJObjectMake(context, MyObject_class(context), NULL);
    AJStringRef myObjectIString = AJStringCreateWithUTF8CString("MyObject");
    AJObjectSetProperty(context, globalObject, myObjectIString, myObject, kAJPropertyAttributeNone, NULL);
    AJStringRelease(myObjectIString);
    
    AJObjectRef EvilExceptionObject = AJObjectMake(context, EvilExceptionObject_class(context), NULL);
    AJStringRef EvilExceptionObjectIString = AJStringCreateWithUTF8CString("EvilExceptionObject");
    AJObjectSetProperty(context, globalObject, EvilExceptionObjectIString, EvilExceptionObject, kAJPropertyAttributeNone, NULL);
    AJStringRelease(EvilExceptionObjectIString);
    
    AJObjectRef EmptyObject = AJObjectMake(context, EmptyObject_class(context), NULL);
    AJStringRef EmptyObjectIString = AJStringCreateWithUTF8CString("EmptyObject");
    AJObjectSetProperty(context, globalObject, EmptyObjectIString, EmptyObject, kAJPropertyAttributeNone, NULL);
    AJStringRelease(EmptyObjectIString);
    
    AJStringRef lengthStr = AJStringCreateWithUTF8CString("length");
    aHeapRef = AJObjectMakeArray(context, 0, 0, 0);
    AJObjectSetProperty(context, aHeapRef, lengthStr, AJValueMakeNumber(context, 10), 0, 0);
    AJStringRef privatePropertyName = AJStringCreateWithUTF8CString("privateProperty");
    if (!AJObjectSetPrivateProperty(context, myObject, privatePropertyName, aHeapRef)) {
        printf("FAIL: Could not set private property.\n");
        failed = 1;        
    } else {
        printf("PASS: Set private property.\n");
    }
    if (AJObjectSetPrivateProperty(context, aHeapRef, privatePropertyName, aHeapRef)) {
        printf("FAIL: AJObjectSetPrivateProperty should fail on non-API objects.\n");
        failed = 1;        
    } else {
        printf("PASS: Did not allow AJObjectSetPrivateProperty on a non-API object.\n");
    }
    if (AJObjectGetPrivateProperty(context, myObject, privatePropertyName) != aHeapRef) {
        printf("FAIL: Could not retrieve private property.\n");
        failed = 1;
    } else
        printf("PASS: Retrieved private property.\n");
    if (AJObjectGetPrivateProperty(context, aHeapRef, privatePropertyName)) {
        printf("FAIL: AJObjectGetPrivateProperty should return NULL when called on a non-API object.\n");
        failed = 1;
    } else
        printf("PASS: AJObjectGetPrivateProperty return NULL.\n");
    
    if (AJObjectGetProperty(context, myObject, privatePropertyName, 0) == aHeapRef) {
        printf("FAIL: Accessed private property through ordinary property lookup.\n");
        failed = 1;
    } else
        printf("PASS: Cannot access private property through ordinary property lookup.\n");
    
    AJGarbageCollect(context);
    
    for (int i = 0; i < 10000; i++)
        AJObjectMake(context, 0, 0);

    if (AJValueToNumber(context, AJObjectGetProperty(context, aHeapRef, lengthStr, 0), 0) != 10) {
        printf("FAIL: Private property has been collected.\n");
        failed = 1;
    } else
        printf("PASS: Private property does not appear to have been collected.\n");
    AJStringRelease(lengthStr);
    
    AJStringRef validJSON = AJStringCreateWithUTF8CString("{\"aProperty\":true}");
    AJValueRef jsonObject = AJValueMakeFromJSONString(context, validJSON);
    AJStringRelease(validJSON);
    if (!AJValueIsObject(context, jsonObject)) {
        printf("FAIL: Did not parse valid JSON correctly\n");
        failed = 1;
    } else
        printf("PASS: Parsed valid JSON string.\n");
    AJStringRef propertyName = AJStringCreateWithUTF8CString("aProperty");
    assertEqualsAsBoolean(AJObjectGetProperty(context, AJValueToObject(context, jsonObject, 0), propertyName, 0), true);
    AJStringRelease(propertyName);
    AJStringRef invalidJSON = AJStringCreateWithUTF8CString("fail!");
    if (AJValueMakeFromJSONString(context, invalidJSON)) {
        printf("FAIL: Should return null for invalid JSON data\n");
        failed = 1;
    } else
        printf("PASS: Correctly returned null for invalid JSON data.\n");
    AJValueRef exception;
    AJStringRef str = AJValueCreateJSONString(context, jsonObject, 0, 0);
    if (!AJStringIsEqualToUTF8CString(str, "{\"aProperty\":true}")) {
        printf("FAIL: Did not correctly serialise with indent of 0.\n");
        failed = 1;
    } else
        printf("PASS: Correctly serialised with indent of 0.\n");
    AJStringRelease(str);

    str = AJValueCreateJSONString(context, jsonObject, 4, 0);
    if (!AJStringIsEqualToUTF8CString(str, "{\n    \"aProperty\": true\n}")) {
        printf("FAIL: Did not correctly serialise with indent of 4.\n");
        failed = 1;
    } else
        printf("PASS: Correctly serialised with indent of 4.\n");
    AJStringRelease(str);
    AJStringRef src = AJStringCreateWithUTF8CString("({get a(){ throw '';}})");
    AJValueRef unstringifiableObj = AJEvaluateScript(context, src, NULL, NULL, 1, NULL);
    
    str = AJValueCreateJSONString(context, unstringifiableObj, 4, 0);
    if (str) {
        printf("FAIL: Didn't return null when attempting to serialize unserializable value.\n");
        AJStringRelease(str);
        failed = 1;
    } else
        printf("PASS: returned null when attempting to serialize unserializable value.\n");
    
    str = AJValueCreateJSONString(context, unstringifiableObj, 4, &exception);
    if (str) {
        printf("FAIL: Didn't return null when attempting to serialize unserializable value.\n");
        AJStringRelease(str);
        failed = 1;
    } else
        printf("PASS: returned null when attempting to serialize unserializable value.\n");
    if (!exception) {
        printf("FAIL: Did not set exception on serialisation error\n");
        failed = 1;
    } else
        printf("PASS: set exception on serialisation error\n");
    // Conversions that throw exceptions
    exception = NULL;
    ASSERT(NULL == AJValueToObject(context, jsNull, &exception));
    ASSERT(exception);
    
    exception = NULL;
    // FIXME <rdar://4668451> - On i386 the isnan(double) macro tries to map to the isnan(float) function,
    // causing a build break with -Wshorten-64-to-32 enabled.  The issue is known by the appropriate team.
    // After that's resolved, we can remove these casts
    ASSERT(isnan((float)AJValueToNumber(context, jsObjectNoProto, &exception)));
    ASSERT(exception);

    exception = NULL;
    ASSERT(!AJValueToStringCopy(context, jsObjectNoProto, &exception));
    ASSERT(exception);
    
    ASSERT(AJValueToBoolean(context, myObject));
    
    exception = NULL;
    ASSERT(!AJValueIsEqual(context, jsObjectNoProto, AJValueMakeNumber(context, 1), &exception));
    ASSERT(exception);
    
    exception = NULL;
    AJObjectGetPropertyAtIndex(context, myObject, 0, &exception);
    ASSERT(1 == AJValueToNumber(context, exception, NULL));

    assertEqualsAsBoolean(jsUndefined, false);
    assertEqualsAsBoolean(jsNull, false);
    assertEqualsAsBoolean(jsTrue, true);
    assertEqualsAsBoolean(jsFalse, false);
    assertEqualsAsBoolean(jsZero, false);
    assertEqualsAsBoolean(jsOne, true);
    assertEqualsAsBoolean(jsOneThird, true);
    assertEqualsAsBoolean(jsEmptyString, false);
    assertEqualsAsBoolean(jsOneString, true);
    assertEqualsAsBoolean(jsCFString, true);
    assertEqualsAsBoolean(jsCFStringWithCharacters, true);
    assertEqualsAsBoolean(jsCFEmptyString, false);
    assertEqualsAsBoolean(jsCFEmptyStringWithCharacters, false);
    
    assertEqualsAsNumber(jsUndefined, nan(""));
    assertEqualsAsNumber(jsNull, 0);
    assertEqualsAsNumber(jsTrue, 1);
    assertEqualsAsNumber(jsFalse, 0);
    assertEqualsAsNumber(jsZero, 0);
    assertEqualsAsNumber(jsOne, 1);
    assertEqualsAsNumber(jsOneThird, 1.0 / 3.0);
    assertEqualsAsNumber(jsEmptyString, 0);
    assertEqualsAsNumber(jsOneString, 1);
    assertEqualsAsNumber(jsCFString, nan(""));
    assertEqualsAsNumber(jsCFStringWithCharacters, nan(""));
    assertEqualsAsNumber(jsCFEmptyString, 0);
    assertEqualsAsNumber(jsCFEmptyStringWithCharacters, 0);
    ASSERT(sizeof(AJChar) == sizeof(UniChar));
    
    assertEqualsAsCharactersPtr(jsUndefined, "undefined");
    assertEqualsAsCharactersPtr(jsNull, "null");
    assertEqualsAsCharactersPtr(jsTrue, "true");
    assertEqualsAsCharactersPtr(jsFalse, "false");
    assertEqualsAsCharactersPtr(jsZero, "0");
    assertEqualsAsCharactersPtr(jsOne, "1");
    assertEqualsAsCharactersPtr(jsOneThird, "0.3333333333333333");
    assertEqualsAsCharactersPtr(jsEmptyString, "");
    assertEqualsAsCharactersPtr(jsOneString, "1");
    assertEqualsAsCharactersPtr(jsCFString, "A");
    assertEqualsAsCharactersPtr(jsCFStringWithCharacters, "A");
    assertEqualsAsCharactersPtr(jsCFEmptyString, "");
    assertEqualsAsCharactersPtr(jsCFEmptyStringWithCharacters, "");
    
    assertEqualsAsUTF8String(jsUndefined, "undefined");
    assertEqualsAsUTF8String(jsNull, "null");
    assertEqualsAsUTF8String(jsTrue, "true");
    assertEqualsAsUTF8String(jsFalse, "false");
    assertEqualsAsUTF8String(jsZero, "0");
    assertEqualsAsUTF8String(jsOne, "1");
    assertEqualsAsUTF8String(jsOneThird, "0.3333333333333333");
    assertEqualsAsUTF8String(jsEmptyString, "");
    assertEqualsAsUTF8String(jsOneString, "1");
    assertEqualsAsUTF8String(jsCFString, "A");
    assertEqualsAsUTF8String(jsCFStringWithCharacters, "A");
    assertEqualsAsUTF8String(jsCFEmptyString, "");
    assertEqualsAsUTF8String(jsCFEmptyStringWithCharacters, "");
    
    ASSERT(AJValueIsStrictEqual(context, jsTrue, jsTrue));
    ASSERT(!AJValueIsStrictEqual(context, jsOne, jsOneString));

    ASSERT(AJValueIsEqual(context, jsOne, jsOneString, NULL));
    ASSERT(!AJValueIsEqual(context, jsTrue, jsFalse, NULL));
    
    CFStringRef cfAJString = AJStringCopyCFString(kCFAllocatorDefault, jsCFIString);
    CFStringRef cfJSEmptyString = AJStringCopyCFString(kCFAllocatorDefault, jsCFEmptyIString);
    ASSERT(CFEqual(cfAJString, cfString));
    ASSERT(CFEqual(cfJSEmptyString, cfEmptyString));
    CFRelease(cfAJString);
    CFRelease(cfJSEmptyString);

    CFRelease(cfString);
    CFRelease(cfEmptyString);
    
    jsGlobalValue = AJObjectMake(context, NULL, NULL);
    makeGlobalNumberValue(context);
    AJValueProtect(context, jsGlobalValue);
    AJGarbageCollect(context);
    ASSERT(AJValueIsObject(context, jsGlobalValue));
    AJValueUnprotect(context, jsGlobalValue);
    AJValueUnprotect(context, jsNumberValue);

    AJStringRef goodSyntax = AJStringCreateWithUTF8CString("x = 1;");
    AJStringRef badSyntax = AJStringCreateWithUTF8CString("x := 1;");
    ASSERT(AJCheckScriptSyntax(context, goodSyntax, NULL, 0, NULL));
    ASSERT(!AJCheckScriptSyntax(context, badSyntax, NULL, 0, NULL));

    AJValueRef result;
    AJValueRef v;
    AJObjectRef o;
    AJStringRef string;

    result = AJEvaluateScript(context, goodSyntax, NULL, NULL, 1, NULL);
    ASSERT(result);
    ASSERT(AJValueIsEqual(context, result, jsOne, NULL));

    exception = NULL;
    result = AJEvaluateScript(context, badSyntax, NULL, NULL, 1, &exception);
    ASSERT(!result);
    ASSERT(AJValueIsObject(context, exception));
    
    AJStringRef array = AJStringCreateWithUTF8CString("Array");
    AJObjectRef arrayConstructor = AJValueToObject(context, AJObjectGetProperty(context, globalObject, array, NULL), NULL);
    AJStringRelease(array);
    result = AJObjectCallAsConstructor(context, arrayConstructor, 0, NULL, NULL);
    ASSERT(result);
    ASSERT(AJValueIsObject(context, result));
    ASSERT(AJValueIsInstanceOfConstructor(context, result, arrayConstructor, NULL));
    ASSERT(!AJValueIsInstanceOfConstructor(context, AJValueMakeNull(context), arrayConstructor, NULL));

    o = AJValueToObject(context, result, NULL);
    exception = NULL;
    ASSERT(AJValueIsUndefined(context, AJObjectGetPropertyAtIndex(context, o, 0, &exception)));
    ASSERT(!exception);
    
    AJObjectSetPropertyAtIndex(context, o, 0, AJValueMakeNumber(context, 1), &exception);
    ASSERT(!exception);
    
    exception = NULL;
    ASSERT(1 == AJValueToNumber(context, AJObjectGetPropertyAtIndex(context, o, 0, &exception), &exception));
    ASSERT(!exception);

    AJStringRef functionBody;
    AJObjectRef function;
    
    exception = NULL;
    functionBody = AJStringCreateWithUTF8CString("rreturn Array;");
    AJStringRef line = AJStringCreateWithUTF8CString("line");
    ASSERT(!AJObjectMakeFunction(context, NULL, 0, NULL, functionBody, NULL, 1, &exception));
    ASSERT(AJValueIsObject(context, exception));
    v = AJObjectGetProperty(context, AJValueToObject(context, exception, NULL), line, NULL);
    assertEqualsAsNumber(v, 1);
    AJStringRelease(functionBody);
    AJStringRelease(line);

    exception = NULL;
    functionBody = AJStringCreateWithUTF8CString("return Array;");
    function = AJObjectMakeFunction(context, NULL, 0, NULL, functionBody, NULL, 1, &exception);
    AJStringRelease(functionBody);
    ASSERT(!exception);
    ASSERT(AJObjectIsFunction(context, function));
    v = AJObjectCallAsFunction(context, function, NULL, 0, NULL, NULL);
    ASSERT(v);
    ASSERT(AJValueIsEqual(context, v, arrayConstructor, NULL));
    
    exception = NULL;
    function = AJObjectMakeFunction(context, NULL, 0, NULL, jsEmptyIString, NULL, 0, &exception);
    ASSERT(!exception);
    v = AJObjectCallAsFunction(context, function, NULL, 0, NULL, &exception);
    ASSERT(v && !exception);
    ASSERT(AJValueIsUndefined(context, v));
    
    exception = NULL;
    v = NULL;
    AJStringRef foo = AJStringCreateWithUTF8CString("foo");
    AJStringRef argumentNames[] = { foo };
    functionBody = AJStringCreateWithUTF8CString("return foo;");
    function = AJObjectMakeFunction(context, foo, 1, argumentNames, functionBody, NULL, 1, &exception);
    ASSERT(function && !exception);
    AJValueRef arguments[] = { AJValueMakeNumber(context, 2) };
    v = AJObjectCallAsFunction(context, function, NULL, 1, arguments, &exception);
    AJStringRelease(foo);
    AJStringRelease(functionBody);
    
    string = AJValueToStringCopy(context, function, NULL);
    assertEqualsAsUTF8String(AJValueMakeString(context, string), "function foo(foo) { return foo;\n}");
    AJStringRelease(string);

    AJStringRef print = AJStringCreateWithUTF8CString("print");
    AJObjectRef printFunction = AJObjectMakeFunctionWithCallback(context, print, print_callAsFunction);
    AJObjectSetProperty(context, globalObject, print, printFunction, kAJPropertyAttributeNone, NULL); 
    AJStringRelease(print);
    
    ASSERT(!AJObjectSetPrivate(printFunction, (void*)1));
    ASSERT(!AJObjectGetPrivate(printFunction));

    AJStringRef myConstructorIString = AJStringCreateWithUTF8CString("MyConstructor");
    AJObjectRef myConstructor = AJObjectMakeConstructor(context, NULL, myConstructor_callAsConstructor);
    AJObjectSetProperty(context, globalObject, myConstructorIString, myConstructor, kAJPropertyAttributeNone, NULL);
    AJStringRelease(myConstructorIString);
    
    ASSERT(!AJObjectSetPrivate(myConstructor, (void*)1));
    ASSERT(!AJObjectGetPrivate(myConstructor));
    
    string = AJStringCreateWithUTF8CString("Base");
    AJObjectRef baseConstructor = AJObjectMakeConstructor(context, Base_class(context), NULL);
    AJObjectSetProperty(context, globalObject, string, baseConstructor, kAJPropertyAttributeNone, NULL);
    AJStringRelease(string);
    
    string = AJStringCreateWithUTF8CString("Derived");
    AJObjectRef derivedConstructor = AJObjectMakeConstructor(context, Derived_class(context), NULL);
    AJObjectSetProperty(context, globalObject, string, derivedConstructor, kAJPropertyAttributeNone, NULL);
    AJStringRelease(string);
    
    string = AJStringCreateWithUTF8CString("Derived2");
    AJObjectRef derived2Constructor = AJObjectMakeConstructor(context, Derived2_class(context), NULL);
    AJObjectSetProperty(context, globalObject, string, derived2Constructor, kAJPropertyAttributeNone, NULL);
    AJStringRelease(string);

    o = AJObjectMake(context, NULL, NULL);
    AJObjectSetProperty(context, o, jsOneIString, AJValueMakeNumber(context, 1), kAJPropertyAttributeNone, NULL);
    AJObjectSetProperty(context, o, jsCFIString,  AJValueMakeNumber(context, 1), kAJPropertyAttributeDontEnum, NULL);
    AJPropertyNameArrayRef nameArray = AJObjectCopyPropertyNames(context, o);
    size_t expectedCount = AJPropertyNameArrayGetCount(nameArray);
    size_t count;
    for (count = 0; count < expectedCount; ++count)
        AJPropertyNameArrayGetNameAtIndex(nameArray, count);
    AJPropertyNameArrayRelease(nameArray);
    ASSERT(count == 1); // jsCFString should not be enumerated

    AJValueRef argumentsArrayValues[] = { AJValueMakeNumber(context, 10), AJValueMakeNumber(context, 20) };
    o = AJObjectMakeArray(context, sizeof(argumentsArrayValues) / sizeof(AJValueRef), argumentsArrayValues, NULL);
    string = AJStringCreateWithUTF8CString("length");
    v = AJObjectGetProperty(context, o, string, NULL);
    assertEqualsAsNumber(v, 2);
    v = AJObjectGetPropertyAtIndex(context, o, 0, NULL);
    assertEqualsAsNumber(v, 10);
    v = AJObjectGetPropertyAtIndex(context, o, 1, NULL);
    assertEqualsAsNumber(v, 20);

    o = AJObjectMakeArray(context, 0, NULL, NULL);
    v = AJObjectGetProperty(context, o, string, NULL);
    assertEqualsAsNumber(v, 0);
    AJStringRelease(string);

    AJValueRef argumentsDateValues[] = { AJValueMakeNumber(context, 0) };
    o = AJObjectMakeDate(context, 1, argumentsDateValues, NULL);
    if (timeZoneIsPST())
        assertEqualsAsUTF8String(o, "Wed Dec 31 1969 16:00:00 GMT-0800 (PST)");

    string = AJStringCreateWithUTF8CString("an error message");
    AJValueRef argumentsErrorValues[] = { AJValueMakeString(context, string) };
    o = AJObjectMakeError(context, 1, argumentsErrorValues, NULL);
    assertEqualsAsUTF8String(o, "Error: an error message");
    AJStringRelease(string);

    string = AJStringCreateWithUTF8CString("foo");
    AJStringRef string2 = AJStringCreateWithUTF8CString("gi");
    AJValueRef argumentsRegExpValues[] = { AJValueMakeString(context, string), AJValueMakeString(context, string2) };
    o = AJObjectMakeRegExp(context, 2, argumentsRegExpValues, NULL);
    assertEqualsAsUTF8String(o, "/foo/gi");
    AJStringRelease(string);
    AJStringRelease(string2);

    AJClassDefinition nullDefinition = kAJClassDefinitionEmpty;
    nullDefinition.attributes = kAJClassAttributeNoAutomaticPrototype;
    AJClassRef nullClass = AJClassCreate(&nullDefinition);
    AJClassRelease(nullClass);
    
    nullDefinition = kAJClassDefinitionEmpty;
    nullClass = AJClassCreate(&nullDefinition);
    AJClassRelease(nullClass);

    functionBody = AJStringCreateWithUTF8CString("return this;");
    function = AJObjectMakeFunction(context, NULL, 0, NULL, functionBody, NULL, 1, NULL);
    AJStringRelease(functionBody);
    v = AJObjectCallAsFunction(context, function, NULL, 0, NULL, NULL);
    ASSERT(AJValueIsEqual(context, v, globalObject, NULL));
    v = AJObjectCallAsFunction(context, function, o, 0, NULL, NULL);
    ASSERT(AJValueIsEqual(context, v, o, NULL));

    functionBody = AJStringCreateWithUTF8CString("return eval(\"this\");");
    function = AJObjectMakeFunction(context, NULL, 0, NULL, functionBody, NULL, 1, NULL);
    AJStringRelease(functionBody);
    v = AJObjectCallAsFunction(context, function, NULL, 0, NULL, NULL);
    ASSERT(AJValueIsEqual(context, v, globalObject, NULL));
    v = AJObjectCallAsFunction(context, function, o, 0, NULL, NULL);
    ASSERT(AJValueIsEqual(context, v, o, NULL));

    AJStringRef script = AJStringCreateWithUTF8CString("this;");
    v = AJEvaluateScript(context, script, NULL, NULL, 1, NULL);
    ASSERT(AJValueIsEqual(context, v, globalObject, NULL));
    v = AJEvaluateScript(context, script, o, NULL, 1, NULL);
    ASSERT(AJValueIsEqual(context, v, o, NULL));
    AJStringRelease(script);

    script = AJStringCreateWithUTF8CString("eval(this);");
    v = AJEvaluateScript(context, script, NULL, NULL, 1, NULL);
    ASSERT(AJValueIsEqual(context, v, globalObject, NULL));
    v = AJEvaluateScript(context, script, o, NULL, 1, NULL);
    ASSERT(AJValueIsEqual(context, v, o, NULL));
    AJStringRelease(script);

    // Verify that creating a constructor for a class with no static functions does not trigger
    // an assert inside putDirect or lead to a crash during GC. <https://bugs.webkit.org/show_bug.cgi?id=25785>
    nullDefinition = kAJClassDefinitionEmpty;
    nullClass = AJClassCreate(&nullDefinition);
    myConstructor = AJObjectMakeConstructor(context, nullClass, 0);
    AJClassRelease(nullClass);

    char* scriptUTF8 = createStringWithContentsOfFile(scriptPath);
    if (!scriptUTF8) {
        printf("FAIL: Test script could not be loaded.\n");
        failed = 1;
    } else {
        script = AJStringCreateWithUTF8CString(scriptUTF8);
        result = AJEvaluateScript(context, script, NULL, NULL, 1, &exception);
        if (result && AJValueIsUndefined(context, result))
            printf("PASS: Test script executed successfully.\n");
        else {
            printf("FAIL: Test script returned unexpected value:\n");
            AJStringRef exceptionIString = AJValueToStringCopy(context, exception, NULL);
            CFStringRef exceptionCF = AJStringCopyCFString(kCFAllocatorDefault, exceptionIString);
            CFShow(exceptionCF);
            CFRelease(exceptionCF);
            AJStringRelease(exceptionIString);
            failed = 1;
        }
        AJStringRelease(script);
        free(scriptUTF8);
    }

    // Clear out local variables pointing at AJObjectRefs to allow their values to be collected
    function = NULL;
    v = NULL;
    o = NULL;
    globalObject = NULL;
    myConstructor = NULL;

    AJStringRelease(jsEmptyIString);
    AJStringRelease(jsOneIString);
    AJStringRelease(jsCFIString);
    AJStringRelease(jsCFEmptyIString);
    AJStringRelease(jsCFIStringWithCharacters);
    AJStringRelease(jsCFEmptyIStringWithCharacters);
    AJStringRelease(goodSyntax);
    AJStringRelease(badSyntax);

    AJGlobalContextRelease(context);
    AJClassRelease(globalObjectClass);

    // Test for an infinite prototype chain that used to be created. This test
    // passes if the call to AJObjectHasProperty() does not hang.

    AJClassDefinition prototypeLoopClassDefinition = kAJClassDefinitionEmpty;
    prototypeLoopClassDefinition.staticFunctions = globalObject_staticFunctions;
    AJClassRef prototypeLoopClass = AJClassCreate(&prototypeLoopClassDefinition);
    AJGlobalContextRef prototypeLoopContext = AJGlobalContextCreateInGroup(NULL, prototypeLoopClass);

    AJStringRef nameProperty = AJStringCreateWithUTF8CString("name");
    AJObjectHasProperty(prototypeLoopContext, AJContextGetGlobalObject(prototypeLoopContext), nameProperty);

    AJGlobalContextRelease(prototypeLoopContext);
    AJClassRelease(prototypeLoopClass);

    printf("PASS: Infinite prototype chain does not occur.\n");

    if (failed) {
        printf("FAIL: Some tests failed.\n");
        return 1;
    }

    printf("PASS: Program exited normally.\n");
    return 0;
}

static char* createStringWithContentsOfFile(const char* fileName)
{
    char* buffer;
    
    size_t buffer_size = 0;
    size_t buffer_capacity = 1024;
    buffer = (char*)malloc(buffer_capacity);
    
    FILE* f = fopen(fileName, "r");
    if (!f) {
        fprintf(stderr, "Could not open file: %s\n", fileName);
        return 0;
    }
    
    while (!feof(f) && !ferror(f)) {
        buffer_size += fread(buffer + buffer_size, 1, buffer_capacity - buffer_size, f);
        if (buffer_size == buffer_capacity) { // guarantees space for trailing '\0'
            buffer_capacity *= 2;
            buffer = (char*)realloc(buffer, buffer_capacity);
            ASSERT(buffer);
        }
        
        ASSERT(buffer_size < buffer_capacity);
    }
    fclose(f);
    buffer[buffer_size] = '\0';
    
    return buffer;
}
