
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
// Automatically generated from AJCore/runtime/RegExpObject.cpp using AJCore/create_hash_table. DO NOT EDIT!

#include "Lookup.h"

namespace AJ {
#if ENABLE(JIT)
#define THUNK_GENERATOR(generator) , generator
#else
#define THUNK_GENERATOR(generator)
#endif

static const struct HashTableValue regExpTableValues[6] = {
   { "global", DontDelete|ReadOnly|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpObjectGlobal), (intptr_t)0 THUNK_GENERATOR(0) },
   { "ignoreCase", DontDelete|ReadOnly|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpObjectIgnoreCase), (intptr_t)0 THUNK_GENERATOR(0) },
   { "multiline", DontDelete|ReadOnly|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpObjectMultiline), (intptr_t)0 THUNK_GENERATOR(0) },
   { "source", DontDelete|ReadOnly|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpObjectSource), (intptr_t)0 THUNK_GENERATOR(0) },
   { "lastIndex", DontDelete|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpObjectLastIndex), (intptr_t)setRegExpObjectLastIndex THUNK_GENERATOR(0) },
   { 0, 0, 0, 0 THUNK_GENERATOR(0) }
};

#undef THUNK_GENERATOR
extern JSC_CONST_HASHTABLE HashTable regExpTable =
    { 17, 15, regExpTableValues, 0 };
} // namespace
