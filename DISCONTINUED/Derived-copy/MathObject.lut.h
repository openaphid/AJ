
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
// Automatically generated from AJCore/runtime/MathObject.cpp using AJCore/create_hash_table. DO NOT EDIT!

#include "Lookup.h"

namespace AJ {
#if ENABLE(JIT)
#define THUNK_GENERATOR(generator) , generator
#else
#define THUNK_GENERATOR(generator)
#endif

static const struct HashTableValue mathTableValues[19] = {
   { "abs", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncAbs), (intptr_t)1 THUNK_GENERATOR(0) },
   { "acos", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncACos), (intptr_t)1 THUNK_GENERATOR(0) },
   { "asin", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncASin), (intptr_t)1 THUNK_GENERATOR(0) },
   { "atan", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncATan), (intptr_t)1 THUNK_GENERATOR(0) },
   { "atan2", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncATan2), (intptr_t)2 THUNK_GENERATOR(0) },
   { "ceil", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncCeil), (intptr_t)1 THUNK_GENERATOR(0) },
   { "cos", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncCos), (intptr_t)1 THUNK_GENERATOR(0) },
   { "exp", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncExp), (intptr_t)1 THUNK_GENERATOR(0) },
   { "floor", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncFloor), (intptr_t)1 THUNK_GENERATOR(0) },
   { "log", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncLog), (intptr_t)1 THUNK_GENERATOR(0) },
   { "max", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncMax), (intptr_t)2 THUNK_GENERATOR(0) },
   { "min", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncMin), (intptr_t)2 THUNK_GENERATOR(0) },
   { "pow", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncPow), (intptr_t)2 THUNK_GENERATOR(powThunkGenerator) },
   { "random", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncRandom), (intptr_t)0 THUNK_GENERATOR(0) },
   { "round", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncRound), (intptr_t)1 THUNK_GENERATOR(0) },
   { "sin", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncSin), (intptr_t)1 THUNK_GENERATOR(0) },
   { "sqrt", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncSqrt), (intptr_t)1 THUNK_GENERATOR(sqrtThunkGenerator) },
   { "tan", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(mathProtoFuncTan), (intptr_t)1 THUNK_GENERATOR(0) },
   { 0, 0, 0, 0 THUNK_GENERATOR(0) }
};

#undef THUNK_GENERATOR
extern JSC_CONST_HASHTABLE HashTable mathTable =
    { 67, 63, mathTableValues, 0 };
} // namespace
