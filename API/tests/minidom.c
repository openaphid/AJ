
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
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
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

#include "AJContextRef.h"
#include "JSNode.h"
#include "AJObjectRef.h"
#include "AJStringRef.h"
#include <stdio.h>
#include <stdlib.h>
#include <wtf/Assertions.h>
#include <wtf/UnusedParam.h>

static char* createStringWithContentsOfFile(const char* fileName);
static AJValueRef print(AJContextRef context, AJObjectRef object, AJObjectRef thisObject, size_t argumentCount, const AJValueRef arguments[], AJValueRef* exception);

int main(int argc, char* argv[])
{
    const char *scriptPath = "minidom.js";
    if (argc > 1) {
        scriptPath = argv[1];
    }
    
    AJGlobalContextRef context = AJGlobalContextCreateInGroup(NULL, NULL);
    AJObjectRef globalObject = AJContextGetGlobalObject(context);
    
    AJStringRef printIString = AJStringCreateWithUTF8CString("print");
    AJObjectSetProperty(context, globalObject, printIString, AJObjectMakeFunctionWithCallback(context, printIString, print), kAJPropertyAttributeNone, NULL);
    AJStringRelease(printIString);
    
    AJStringRef node = AJStringCreateWithUTF8CString("Node");
    AJObjectSetProperty(context, globalObject, node, AJObjectMakeConstructor(context, JSNode_class(context), JSNode_construct), kAJPropertyAttributeNone, NULL);
    AJStringRelease(node);
    
    char* scriptUTF8 = createStringWithContentsOfFile(scriptPath);
    AJStringRef script = AJStringCreateWithUTF8CString(scriptUTF8);
    AJValueRef exception;
    AJValueRef result = AJEvaluateScript(context, script, NULL, NULL, 1, &exception);
    if (result)
        printf("PASS: Test script executed successfully.\n");
    else {
        printf("FAIL: Test script threw exception:\n");
        AJStringRef exceptionIString = AJValueToStringCopy(context, exception, NULL);
        size_t exceptionUTF8Size = AJStringGetMaximumUTF8CStringSize(exceptionIString);
        char* exceptionUTF8 = (char*)malloc(exceptionUTF8Size);
        AJStringGetUTF8CString(exceptionIString, exceptionUTF8, exceptionUTF8Size);
        printf("%s\n", exceptionUTF8);
        free(exceptionUTF8);
        AJStringRelease(exceptionIString);
    }
    AJStringRelease(script);
    free(scriptUTF8);

    globalObject = 0;
    AJGlobalContextRelease(context);
    printf("PASS: Program exited normally.\n");
    return 0;
}

static AJValueRef print(AJContextRef context, AJObjectRef object, AJObjectRef thisObject, size_t argumentCount, const AJValueRef arguments[], AJValueRef* exception)
{
    UNUSED_PARAM(object);
    UNUSED_PARAM(thisObject);

    if (argumentCount > 0) {
        AJStringRef string = AJValueToStringCopy(context, arguments[0], exception);
        size_t numChars = AJStringGetMaximumUTF8CStringSize(string);
        char stringUTF8[numChars];
        AJStringGetUTF8CString(string, stringUTF8, numChars);
        printf("%s\n", stringUTF8);
    }
    
    return AJValueMakeUndefined(context);
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
        if (buffer_size == buffer_capacity) { /* guarantees space for trailing '\0' */
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
