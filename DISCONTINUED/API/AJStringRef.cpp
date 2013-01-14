
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
 * Copyright (C) 2006, 2007 Apple Inc. All rights reserved.
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
#include "AJStringRef.h"

#include "InitializeThreading.h"
#include "OpaqueAJString.h"
#include <wtf/unicode/UTF8.h>

using namespace AJ;
using namespace ATF::Unicode;

AJStringRef AJStringCreateWithCharacters(const AJChar* chars, size_t numChars)
{
    initializeThreading();
    return OpaqueAJString::create(chars, numChars).releaseRef();
}

AJStringRef AJStringCreateWithUTF8CString(const char* string)
{
    initializeThreading();
    if (string) {
        size_t length = strlen(string);
        Vector<UChar, 1024> buffer(length);
        UChar* p = buffer.data();
        if (conversionOK == convertUTF8ToUTF16(&string, string + length, &p, p + length))
            return OpaqueAJString::create(buffer.data(), p - buffer.data()).releaseRef();
    }

    // Null string.
    return OpaqueAJString::create().releaseRef();
}

AJStringRef AJStringRetain(AJStringRef string)
{
    string->ref();
    return string;
}

void AJStringRelease(AJStringRef string)
{
    string->deref();
}

size_t AJStringGetLength(AJStringRef string)
{
    return string->length();
}

const AJChar* AJStringGetCharactersPtr(AJStringRef string)
{
    return string->characters();
}

size_t AJStringGetMaximumUTF8CStringSize(AJStringRef string)
{
    // Any UTF8 character > 3 bytes encodes as a UTF16 surrogate pair.
    return string->length() * 3 + 1; // + 1 for terminating '\0'
}

size_t AJStringGetUTF8CString(AJStringRef string, char* buffer, size_t bufferSize)
{
    if (!bufferSize)
        return 0;

    char* p = buffer;
    const UChar* d = string->characters();
    ConversionResult result = convertUTF16ToUTF8(&d, d + string->length(), &p, p + bufferSize - 1, true);
    *p++ = '\0';
    if (result != conversionOK && result != targetExhausted)
        return 0;

    return p - buffer;
}

bool AJStringIsEqual(AJStringRef a, AJStringRef b)
{
    unsigned len = a->length();
    return len == b->length() && 0 == memcmp(a->characters(), b->characters(), len * sizeof(UChar));
}

bool AJStringIsEqualToUTF8CString(AJStringRef a, const char* b)
{
    AJStringRef bBuf = AJStringCreateWithUTF8CString(b);
    bool result = AJStringIsEqual(a, bBuf);
    AJStringRelease(bBuf);
    
    return result;
}
