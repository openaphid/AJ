
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
// Automatically generated from AJCore/runtime/NumberConstructor.cpp using AJCore/create_hash_table. DO NOT EDIT!

#include "Lookup.h"

namespace AJ {
#if ENABLE(JIT)
#define THUNK_GENERATOR(generator) , generator
#else
#define THUNK_GENERATOR(generator)
#endif

static const struct HashTableValue numberTableValues[6] = {
   { "NaN", DontEnum|DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(numberConstructorNaNValue), (intptr_t)0 THUNK_GENERATOR(0) },
   { "NEGATIVE_INFINITY", DontEnum|DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(numberConstructorNegInfinity), (intptr_t)0 THUNK_GENERATOR(0) },
   { "POSITIVE_INFINITY", DontEnum|DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(numberConstructorPosInfinity), (intptr_t)0 THUNK_GENERATOR(0) },
   { "MAX_VALUE", DontEnum|DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(numberConstructorMaxValue), (intptr_t)0 THUNK_GENERATOR(0) },
   { "MIN_VALUE", DontEnum|DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(numberConstructorMinValue), (intptr_t)0 THUNK_GENERATOR(0) },
   { 0, 0, 0, 0 THUNK_GENERATOR(0) }
};

#undef THUNK_GENERATOR
extern JSC_CONST_HASHTABLE HashTable numberTable =
    { 16, 15, numberTableValues, 0 };
} // namespace
