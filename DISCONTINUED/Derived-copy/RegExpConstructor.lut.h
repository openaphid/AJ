
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
// Automatically generated from AJCore/runtime/RegExpConstructor.cpp using AJCore/create_hash_table. DO NOT EDIT!

#include "Lookup.h"

namespace AJ {
#if ENABLE(JIT)
#define THUNK_GENERATOR(generator) , generator
#else
#define THUNK_GENERATOR(generator)
#endif

static const struct HashTableValue regExpConstructorTableValues[22] = {
   { "input", None, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorInput), (intptr_t)setRegExpConstructorInput THUNK_GENERATOR(0) },
   { "$_", DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorInput), (intptr_t)setRegExpConstructorInput THUNK_GENERATOR(0) },
   { "multiline", None, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorMultiline), (intptr_t)setRegExpConstructorMultiline THUNK_GENERATOR(0) },
   { "$*", DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorMultiline), (intptr_t)setRegExpConstructorMultiline THUNK_GENERATOR(0) },
   { "lastMatch", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorLastMatch), (intptr_t)0 THUNK_GENERATOR(0) },
   { "$&", DontDelete|ReadOnly|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorLastMatch), (intptr_t)0 THUNK_GENERATOR(0) },
   { "lastParen", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorLastParen), (intptr_t)0 THUNK_GENERATOR(0) },
   { "$+", DontDelete|ReadOnly|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorLastParen), (intptr_t)0 THUNK_GENERATOR(0) },
   { "leftContext", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorLeftContext), (intptr_t)0 THUNK_GENERATOR(0) },
   { "$`", DontDelete|ReadOnly|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorLeftContext), (intptr_t)0 THUNK_GENERATOR(0) },
   { "rightContext", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorRightContext), (intptr_t)0 THUNK_GENERATOR(0) },
   { "$'", DontDelete|ReadOnly|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorRightContext), (intptr_t)0 THUNK_GENERATOR(0) },
   { "$1", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorDollar1), (intptr_t)0 THUNK_GENERATOR(0) },
   { "$2", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorDollar2), (intptr_t)0 THUNK_GENERATOR(0) },
   { "$3", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorDollar3), (intptr_t)0 THUNK_GENERATOR(0) },
   { "$4", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorDollar4), (intptr_t)0 THUNK_GENERATOR(0) },
   { "$5", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorDollar5), (intptr_t)0 THUNK_GENERATOR(0) },
   { "$6", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorDollar6), (intptr_t)0 THUNK_GENERATOR(0) },
   { "$7", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorDollar7), (intptr_t)0 THUNK_GENERATOR(0) },
   { "$8", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorDollar8), (intptr_t)0 THUNK_GENERATOR(0) },
   { "$9", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpConstructorDollar9), (intptr_t)0 THUNK_GENERATOR(0) },
   { 0, 0, 0, 0 THUNK_GENERATOR(0) }
};

#undef THUNK_GENERATOR
extern JSC_CONST_HASHTABLE HashTable regExpConstructorTable =
    { 65, 63, regExpConstructorTableValues, 0 };
} // namespace
