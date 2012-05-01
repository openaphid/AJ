
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
 *  Copyright (C) 2003, 2007, 2008, 2009 Apple Inc. All rights reserved.
 *  Copyright (C) 2003 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2006 Alexey Proskuryakov (ap@nypop.com)
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 *
 */

#include "config.h"
#include "ArrayPrototype.h"

#include "CachedCall.h"
#include "CodeBlock.h"
#include "Interpreter.h"
#include "JIT.h"
#include "AJStringBuilder.h"
#include "Lookup.h"
#include "ObjectPrototype.h"
#include "Operations.h"
#include <algorithm>
#include <wtf/Assertions.h>
#include <wtf/HashSet.h>

namespace AJ {

ASSERT_CLASS_FITS_IN_CELL(ArrayPrototype);

static AJValue JSC_HOST_CALL arrayProtoFuncToString(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL arrayProtoFuncToLocaleString(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL arrayProtoFuncConcat(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL arrayProtoFuncJoin(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL arrayProtoFuncPop(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL arrayProtoFuncPush(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL arrayProtoFuncReverse(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL arrayProtoFuncShift(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL arrayProtoFuncSlice(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL arrayProtoFuncSort(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL arrayProtoFuncSplice(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL arrayProtoFuncUnShift(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL arrayProtoFuncEvery(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL arrayProtoFuncForEach(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL arrayProtoFuncSome(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL arrayProtoFuncIndexOf(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL arrayProtoFuncFilter(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL arrayProtoFuncMap(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL arrayProtoFuncReduce(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL arrayProtoFuncReduceRight(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL arrayProtoFuncLastIndexOf(ExecState*, AJObject*, AJValue, const ArgList&);

}

#include "ArrayPrototype.lut.h"

namespace AJ {

static inline bool isNumericCompareFunction(ExecState* exec, CallType callType, const CallData& callData)
{
    if (callType != CallTypeJS)
        return false;

#if ENABLE(JIT)
    // If the JIT is enabled then we need to preserve the invariant that every
    // function with a CodeBlock also has JIT code.
    CodeBlock* codeBlock = 0;
#if ENABLE(INTERPRETER)
    if (!exec->globalData().canUseJIT())
        codeBlock = &callData.js.functionExecutable->bytecode(exec, callData.js.scopeChain);
    else
#endif
    {
        callData.js.functionExecutable->jitCode(exec, callData.js.scopeChain);
        codeBlock = &callData.js.functionExecutable->generatedBytecode();
    }
#else
    CodeBlock* codeBlock = &callData.js.functionExecutable->bytecode(exec, callData.js.scopeChain);
#endif

    return codeBlock->isNumericCompareFunction();
}

// ------------------------------ ArrayPrototype ----------------------------

const ClassInfo ArrayPrototype::info = {"Array", &AJArray::info, 0, ExecState::arrayTable};

/* Source for ArrayPrototype.lut.h
@begin arrayTable 16
  toString       arrayProtoFuncToString       DontEnum|Function 0
  toLocaleString arrayProtoFuncToLocaleString DontEnum|Function 0
  concat         arrayProtoFuncConcat         DontEnum|Function 1
  join           arrayProtoFuncJoin           DontEnum|Function 1
  pop            arrayProtoFuncPop            DontEnum|Function 0
  push           arrayProtoFuncPush           DontEnum|Function 1
  reverse        arrayProtoFuncReverse        DontEnum|Function 0
  shift          arrayProtoFuncShift          DontEnum|Function 0
  slice          arrayProtoFuncSlice          DontEnum|Function 2
  sort           arrayProtoFuncSort           DontEnum|Function 1
  splice         arrayProtoFuncSplice         DontEnum|Function 2
  unshift        arrayProtoFuncUnShift        DontEnum|Function 1
  every          arrayProtoFuncEvery          DontEnum|Function 1
  forEach        arrayProtoFuncForEach        DontEnum|Function 1
  some           arrayProtoFuncSome           DontEnum|Function 1
  indexOf        arrayProtoFuncIndexOf        DontEnum|Function 1
  lastIndexOf    arrayProtoFuncLastIndexOf    DontEnum|Function 1
  filter         arrayProtoFuncFilter         DontEnum|Function 1
  reduce         arrayProtoFuncReduce         DontEnum|Function 1
  reduceRight    arrayProtoFuncReduceRight    DontEnum|Function 1
  map            arrayProtoFuncMap            DontEnum|Function 1
@end
*/

// ECMA 15.4.4
ArrayPrototype::ArrayPrototype(NonNullPassRefPtr<Structure> structure)
    : AJArray(structure)
{
}

bool ArrayPrototype::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticFunctionSlot<AJArray>(exec, ExecState::arrayTable(exec), this, propertyName, slot);
}

bool ArrayPrototype::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    return getStaticFunctionDescriptor<AJArray>(exec, ExecState::arrayTable(exec), this, propertyName, descriptor);
}

// ------------------------------ Array Functions ----------------------------

// Helper function
static AJValue getProperty(ExecState* exec, AJObject* obj, unsigned index)
{
    PropertySlot slot(obj);
    if (!obj->getPropertySlot(exec, index, slot))
        return AJValue();
    return slot.getValue(exec, index);
}

static void putProperty(ExecState* exec, AJObject* obj, const Identifier& propertyName, AJValue value)
{
    PutPropertySlot slot;
    obj->put(exec, propertyName, value, slot);
}

AJValue JSC_HOST_CALL arrayProtoFuncToString(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    bool isRealArray = isAJArray(&exec->globalData(), thisValue);
    if (!isRealArray && !thisValue.inherits(&AJArray::info))
        return throwError(exec, TypeError);
    AJArray* thisObj = asArray(thisValue);
    
    HashSet<AJObject*>& arrayVisitedElements = exec->globalData().arrayVisitedElements;
    if (arrayVisitedElements.size() >= MaxSmallThreadReentryDepth) {
        if (arrayVisitedElements.size() >= exec->globalData().maxReentryDepth)
            return throwError(exec, RangeError, "Maximum call stack size exceeded.");    
    }

    bool alreadyVisited = !arrayVisitedElements.add(thisObj).second;
    if (alreadyVisited)
        return jsEmptyString(exec); // return an empty string, avoiding infinite recursion.

    unsigned length = thisObj->get(exec, exec->propertyNames().length).toUInt32(exec);
    unsigned totalSize = length ? length - 1 : 0;
    Vector<RefPtr<UString::Rep>, 256> strBuffer(length);
    for (unsigned k = 0; k < length; k++) {
        AJValue element;
        if (isRealArray && thisObj->canGetIndex(k))
            element = thisObj->getIndex(k);
        else
            element = thisObj->get(exec, k);
        
        if (element.isUndefinedOrNull())
            continue;
        
        UString str = element.toString(exec);
        strBuffer[k] = str.rep();
        totalSize += str.size();
        
        if (!strBuffer.data()) {
            throwOutOfMemoryError(exec);
        }
        
        if (exec->hadException())
            break;
    }
    arrayVisitedElements.remove(thisObj);
    if (!totalSize)
        return jsEmptyString(exec);
    Vector<UChar> buffer;
    buffer.reserveCapacity(totalSize);
    if (!buffer.data())
        return throwOutOfMemoryError(exec);
        
    for (unsigned i = 0; i < length; i++) {
        if (i)
            buffer.append(',');
        if (RefPtr<UString::Rep> rep = strBuffer[i])
            buffer.append(rep->characters(), rep->length());
    }
    ASSERT(buffer.size() == totalSize);
    return jsString(exec, UString::adopt(buffer));
}

AJValue JSC_HOST_CALL arrayProtoFuncToLocaleString(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&AJArray::info))
        return throwError(exec, TypeError);
    AJObject* thisObj = asArray(thisValue);

    HashSet<AJObject*>& arrayVisitedElements = exec->globalData().arrayVisitedElements;
    if (arrayVisitedElements.size() >= MaxSmallThreadReentryDepth) {
        if (arrayVisitedElements.size() >= exec->globalData().maxReentryDepth)
            return throwError(exec, RangeError, "Maximum call stack size exceeded.");    
    }

    bool alreadyVisited = !arrayVisitedElements.add(thisObj).second;
    if (alreadyVisited)
        return jsEmptyString(exec); // return an empty string, avoding infinite recursion.

    AJStringBuilder strBuffer;
    unsigned length = thisObj->get(exec, exec->propertyNames().length).toUInt32(exec);
    for (unsigned k = 0; k < length; k++) {
        if (k >= 1)
            strBuffer.append(',');

        AJValue element = thisObj->get(exec, k);
        if (!element.isUndefinedOrNull()) {
            AJObject* o = element.toObject(exec);
            AJValue conversionFunction = o->get(exec, exec->propertyNames().toLocaleString);
            UString str;
            CallData callData;
            CallType callType = conversionFunction.getCallData(callData);
            if (callType != CallTypeNone)
                str = call(exec, conversionFunction, callType, callData, element, exec->emptyList()).toString(exec);
            else
                str = element.toString(exec);
            strBuffer.append(str);
        }
    }
    arrayVisitedElements.remove(thisObj);
    return strBuffer.build(exec);
}

AJValue JSC_HOST_CALL arrayProtoFuncJoin(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    AJObject* thisObj = thisValue.toThisObject(exec);

    HashSet<AJObject*>& arrayVisitedElements = exec->globalData().arrayVisitedElements;
    if (arrayVisitedElements.size() >= MaxSmallThreadReentryDepth) {
        if (arrayVisitedElements.size() >= exec->globalData().maxReentryDepth)
            return throwError(exec, RangeError, "Maximum call stack size exceeded.");    
    }

    bool alreadyVisited = !arrayVisitedElements.add(thisObj).second;
    if (alreadyVisited)
        return jsEmptyString(exec); // return an empty string, avoding infinite recursion.

    AJStringBuilder strBuffer;

    UString separator;
    if (!args.at(0).isUndefined())
        separator = args.at(0).toString(exec);

    unsigned length = thisObj->get(exec, exec->propertyNames().length).toUInt32(exec);
    unsigned k = 0;
    if (isAJArray(&exec->globalData(), thisObj)) {
        AJArray* array = asArray(thisObj);
        for (; k < length; k++) {
            if (!array->canGetIndex(k))
                break;
            if (k >= 1) {
                if (separator.isNull())
                    strBuffer.append(',');
                else
                    strBuffer.append(separator);
            }
            AJValue element = array->getIndex(k);
            if (!element.isUndefinedOrNull())
                strBuffer.append(element.toString(exec));
        }
    }
    for (; k < length; k++) {
        if (k >= 1) {
            if (separator.isNull())
                strBuffer.append(',');
            else
                strBuffer.append(separator);
        }

        AJValue element = thisObj->get(exec, k);
        if (!element.isUndefinedOrNull())
            strBuffer.append(element.toString(exec));
    }
    arrayVisitedElements.remove(thisObj);
    return strBuffer.build(exec);
}

AJValue JSC_HOST_CALL arrayProtoFuncConcat(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    AJArray* arr = constructEmptyArray(exec);
    int n = 0;
    AJValue curArg = thisValue.toThisObject(exec);
    ArgList::const_iterator it = args.begin();
    ArgList::const_iterator end = args.end();
    while (1) {
        if (curArg.inherits(&AJArray::info)) {
            unsigned length = curArg.get(exec, exec->propertyNames().length).toUInt32(exec);
            AJObject* curObject = curArg.toObject(exec);
            for (unsigned k = 0; k < length; ++k) {
                if (AJValue v = getProperty(exec, curObject, k))
                    arr->put(exec, n, v);
                n++;
            }
        } else {
            arr->put(exec, n, curArg);
            n++;
        }
        if (it == end)
            break;
        curArg = (*it);
        ++it;
    }
    arr->setLength(n);
    return arr;
}

AJValue JSC_HOST_CALL arrayProtoFuncPop(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (isAJArray(&exec->globalData(), thisValue))
        return asArray(thisValue)->pop();

    AJObject* thisObj = thisValue.toThisObject(exec);
    AJValue result;
    unsigned length = thisObj->get(exec, exec->propertyNames().length).toUInt32(exec);
    if (length == 0) {
        putProperty(exec, thisObj, exec->propertyNames().length, jsNumber(exec, length));
        result = jsUndefined();
    } else {
        result = thisObj->get(exec, length - 1);
        thisObj->deleteProperty(exec, length - 1);
        putProperty(exec, thisObj, exec->propertyNames().length, jsNumber(exec, length - 1));
    }
    return result;
}

AJValue JSC_HOST_CALL arrayProtoFuncPush(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    if (isAJArray(&exec->globalData(), thisValue) && args.size() == 1) {
        AJArray* array = asArray(thisValue);
        array->push(exec, *args.begin());
        return jsNumber(exec, array->length());
    }

    AJObject* thisObj = thisValue.toThisObject(exec);
    unsigned length = thisObj->get(exec, exec->propertyNames().length).toUInt32(exec);
    for (unsigned n = 0; n < args.size(); n++)
        thisObj->put(exec, length + n, args.at(n));
    length += args.size();
    putProperty(exec, thisObj, exec->propertyNames().length, jsNumber(exec, length));
    return jsNumber(exec, length);
}

AJValue JSC_HOST_CALL arrayProtoFuncReverse(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    AJObject* thisObj = thisValue.toThisObject(exec);
    unsigned length = thisObj->get(exec, exec->propertyNames().length).toUInt32(exec);
    unsigned middle = length / 2;

    for (unsigned k = 0; k < middle; k++) {
        unsigned lk1 = length - k - 1;
        AJValue obj2 = getProperty(exec, thisObj, lk1);
        AJValue obj = getProperty(exec, thisObj, k);

        if (obj2)
            thisObj->put(exec, k, obj2);
        else
            thisObj->deleteProperty(exec, k);

        if (obj)
            thisObj->put(exec, lk1, obj);
        else
            thisObj->deleteProperty(exec, lk1);
    }
    return thisObj;
}

AJValue JSC_HOST_CALL arrayProtoFuncShift(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    AJObject* thisObj = thisValue.toThisObject(exec);
    AJValue result;

    unsigned length = thisObj->get(exec, exec->propertyNames().length).toUInt32(exec);
    if (length == 0) {
        putProperty(exec, thisObj, exec->propertyNames().length, jsNumber(exec, length));
        result = jsUndefined();
    } else {
        result = thisObj->get(exec, 0);
        for (unsigned k = 1; k < length; k++) {
            if (AJValue obj = getProperty(exec, thisObj, k))
                thisObj->put(exec, k - 1, obj);
            else
                thisObj->deleteProperty(exec, k - 1);
        }
        thisObj->deleteProperty(exec, length - 1);
        putProperty(exec, thisObj, exec->propertyNames().length, jsNumber(exec, length - 1));
    }
    return result;
}

AJValue JSC_HOST_CALL arrayProtoFuncSlice(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    // http://developer.netscape.com/docs/manuals/js/client/jsref/array.htm#1193713 or 15.4.4.10

    AJObject* thisObj = thisValue.toThisObject(exec);

    // We return a new array
    AJArray* resObj = constructEmptyArray(exec);
    AJValue result = resObj;
    double begin = args.at(0).toInteger(exec);
    unsigned length = thisObj->get(exec, exec->propertyNames().length).toUInt32(exec);
    if (begin >= 0) {
        if (begin > length)
            begin = length;
    } else {
        begin += length;
        if (begin < 0)
            begin = 0;
    }
    double end;
    if (args.at(1).isUndefined())
        end = length;
    else {
        end = args.at(1).toInteger(exec);
        if (end < 0) {
            end += length;
            if (end < 0)
                end = 0;
        } else {
            if (end > length)
                end = length;
        }
    }

    int n = 0;
    int b = static_cast<int>(begin);
    int e = static_cast<int>(end);
    for (int k = b; k < e; k++, n++) {
        if (AJValue v = getProperty(exec, thisObj, k))
            resObj->put(exec, n, v);
    }
    resObj->setLength(n);
    return result;
}

AJValue JSC_HOST_CALL arrayProtoFuncSort(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    AJObject* thisObj = thisValue.toThisObject(exec);

    AJValue function = args.at(0);
    CallData callData;
    CallType callType = function.getCallData(callData);

    if (thisObj->classInfo() == &AJArray::info) {
        if (isNumericCompareFunction(exec, callType, callData))
            asArray(thisObj)->sortNumeric(exec, function, callType, callData);
        else if (callType != CallTypeNone)
            asArray(thisObj)->sort(exec, function, callType, callData);
        else
            asArray(thisObj)->sort(exec);
        return thisObj;
    }

    unsigned length = thisObj->get(exec, exec->propertyNames().length).toUInt32(exec);

    if (!length)
        return thisObj;

    // "Min" sort. Not the fastest, but definitely less code than heapsort
    // or quicksort, and much less swapping than bubblesort/insertionsort.
    for (unsigned i = 0; i < length - 1; ++i) {
        AJValue iObj = thisObj->get(exec, i);
        unsigned themin = i;
        AJValue minObj = iObj;
        for (unsigned j = i + 1; j < length; ++j) {
            AJValue jObj = thisObj->get(exec, j);
            double compareResult;
            if (jObj.isUndefined())
                compareResult = 1; // don't check minObj because there's no need to differentiate == (0) from > (1)
            else if (minObj.isUndefined())
                compareResult = -1;
            else if (callType != CallTypeNone) {
                MarkedArgumentBuffer l;
                l.append(jObj);
                l.append(minObj);
                compareResult = call(exec, function, callType, callData, exec->globalThisValue(), l).toNumber(exec);
            } else
                compareResult = (jObj.toString(exec) < minObj.toString(exec)) ? -1 : 1;

            if (compareResult < 0) {
                themin = j;
                minObj = jObj;
            }
        }
        // Swap themin and i
        if (themin > i) {
            thisObj->put(exec, i, minObj);
            thisObj->put(exec, themin, iObj);
        }
    }
    return thisObj;
}

AJValue JSC_HOST_CALL arrayProtoFuncSplice(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    AJObject* thisObj = thisValue.toThisObject(exec);

    // 15.4.4.12
    AJArray* resObj = constructEmptyArray(exec);
    AJValue result = resObj;

    // FIXME: Firefox returns an empty array.
    if (!args.size())
        return jsUndefined();

    unsigned length = thisObj->get(exec, exec->propertyNames().length).toUInt32(exec);
    double relativeBegin = args.at(0).toInteger(exec);
    unsigned begin;
    if (relativeBegin < 0) {
        relativeBegin += length;
        begin = (relativeBegin < 0) ? 0 : static_cast<unsigned>(relativeBegin);
    } else
        begin = std::min<unsigned>(static_cast<unsigned>(relativeBegin), length);

    unsigned deleteCount;
    if (args.size() > 1)
        deleteCount = std::min<int>(std::max<int>(args.at(1).toUInt32(exec), 0), length - begin);
    else
        deleteCount = length - begin;

    for (unsigned k = 0; k < deleteCount; k++) {
        if (AJValue v = getProperty(exec, thisObj, k + begin))
            resObj->put(exec, k, v);
    }
    resObj->setLength(deleteCount);

    unsigned additionalArgs = std::max<int>(args.size() - 2, 0);
    if (additionalArgs != deleteCount) {
        if (additionalArgs < deleteCount) {
            for (unsigned k = begin; k < length - deleteCount; ++k) {
                if (AJValue v = getProperty(exec, thisObj, k + deleteCount))
                    thisObj->put(exec, k + additionalArgs, v);
                else
                    thisObj->deleteProperty(exec, k + additionalArgs);
            }
            for (unsigned k = length; k > length - deleteCount + additionalArgs; --k)
                thisObj->deleteProperty(exec, k - 1);
        } else {
            for (unsigned k = length - deleteCount; k > begin; --k) {
                if (AJValue obj = getProperty(exec, thisObj, k + deleteCount - 1))
                    thisObj->put(exec, k + additionalArgs - 1, obj);
                else
                    thisObj->deleteProperty(exec, k + additionalArgs - 1);
            }
        }
    }
    for (unsigned k = 0; k < additionalArgs; ++k)
        thisObj->put(exec, k + begin, args.at(k + 2));

    putProperty(exec, thisObj, exec->propertyNames().length, jsNumber(exec, length - deleteCount + additionalArgs));
    return result;
}

AJValue JSC_HOST_CALL arrayProtoFuncUnShift(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    AJObject* thisObj = thisValue.toThisObject(exec);

    // 15.4.4.13
    unsigned length = thisObj->get(exec, exec->propertyNames().length).toUInt32(exec);
    unsigned nrArgs = args.size();
    if (nrArgs) {
        for (unsigned k = length; k > 0; --k) {
            if (AJValue v = getProperty(exec, thisObj, k - 1))
                thisObj->put(exec, k + nrArgs - 1, v);
            else
                thisObj->deleteProperty(exec, k + nrArgs - 1);
        }
    }
    for (unsigned k = 0; k < nrArgs; ++k)
        thisObj->put(exec, k, args.at(k));
    AJValue result = jsNumber(exec, length + nrArgs);
    putProperty(exec, thisObj, exec->propertyNames().length, result);
    return result;
}

AJValue JSC_HOST_CALL arrayProtoFuncFilter(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    AJObject* thisObj = thisValue.toThisObject(exec);

    AJValue function = args.at(0);
    CallData callData;
    CallType callType = function.getCallData(callData);
    if (callType == CallTypeNone)
        return throwError(exec, TypeError);

    AJObject* applyThis = args.at(1).isUndefinedOrNull() ? exec->globalThisValue() : args.at(1).toObject(exec);
    AJArray* resultArray = constructEmptyArray(exec);

    unsigned filterIndex = 0;
    unsigned length = thisObj->get(exec, exec->propertyNames().length).toUInt32(exec);
    unsigned k = 0;
    if (callType == CallTypeJS && isAJArray(&exec->globalData(), thisObj)) {
        AJFunction* f = asFunction(function);
        AJArray* array = asArray(thisObj);
        CachedCall cachedCall(exec, f, 3, exec->exceptionSlot());
        for (; k < length && !exec->hadException(); ++k) {
            if (!array->canGetIndex(k))
                break;
            AJValue v = array->getIndex(k);
            cachedCall.setThis(applyThis);
            cachedCall.setArgument(0, v);
            cachedCall.setArgument(1, jsNumber(exec, k));
            cachedCall.setArgument(2, thisObj);
            
            AJValue result = cachedCall.call();
            if (result.toBoolean(exec))
                resultArray->put(exec, filterIndex++, v);
        }
        if (k == length)
            return resultArray;
    }
    for (; k < length && !exec->hadException(); ++k) {
        PropertySlot slot(thisObj);

        if (!thisObj->getPropertySlot(exec, k, slot))
            continue;

        AJValue v = slot.getValue(exec, k);

        MarkedArgumentBuffer eachArguments;

        eachArguments.append(v);
        eachArguments.append(jsNumber(exec, k));
        eachArguments.append(thisObj);

        AJValue result = call(exec, function, callType, callData, applyThis, eachArguments);

        if (result.toBoolean(exec))
            resultArray->put(exec, filterIndex++, v);
    }
    return resultArray;
}

AJValue JSC_HOST_CALL arrayProtoFuncMap(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    AJObject* thisObj = thisValue.toThisObject(exec);

    AJValue function = args.at(0);
    CallData callData;
    CallType callType = function.getCallData(callData);
    if (callType == CallTypeNone)
        return throwError(exec, TypeError);

    AJObject* applyThis = args.at(1).isUndefinedOrNull() ? exec->globalThisValue() : args.at(1).toObject(exec);

    unsigned length = thisObj->get(exec, exec->propertyNames().length).toUInt32(exec);

    AJArray* resultArray = constructEmptyArray(exec, length);
    unsigned k = 0;
    if (callType == CallTypeJS && isAJArray(&exec->globalData(), thisObj)) {
        AJFunction* f = asFunction(function);
        AJArray* array = asArray(thisObj);
        CachedCall cachedCall(exec, f, 3, exec->exceptionSlot());
        for (; k < length && !exec->hadException(); ++k) {
            if (UNLIKELY(!array->canGetIndex(k)))
                break;

            cachedCall.setThis(applyThis);
            cachedCall.setArgument(0, array->getIndex(k));
            cachedCall.setArgument(1, jsNumber(exec, k));
            cachedCall.setArgument(2, thisObj);

            resultArray->AJArray::put(exec, k, cachedCall.call());
        }
    }
    for (; k < length && !exec->hadException(); ++k) {
        PropertySlot slot(thisObj);
        if (!thisObj->getPropertySlot(exec, k, slot))
            continue;

        AJValue v = slot.getValue(exec, k);

        MarkedArgumentBuffer eachArguments;

        eachArguments.append(v);
        eachArguments.append(jsNumber(exec, k));
        eachArguments.append(thisObj);

        AJValue result = call(exec, function, callType, callData, applyThis, eachArguments);
        resultArray->put(exec, k, result);
    }

    return resultArray;
}

// Documentation for these three is available at:
// http://developer-test.mozilla.org/en/docs/Core_AJ_1.5_Reference:Objects:Array:every
// http://developer-test.mozilla.org/en/docs/Core_AJ_1.5_Reference:Objects:Array:forEach
// http://developer-test.mozilla.org/en/docs/Core_AJ_1.5_Reference:Objects:Array:some

AJValue JSC_HOST_CALL arrayProtoFuncEvery(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    AJObject* thisObj = thisValue.toThisObject(exec);

    AJValue function = args.at(0);
    CallData callData;
    CallType callType = function.getCallData(callData);
    if (callType == CallTypeNone)
        return throwError(exec, TypeError);

    AJObject* applyThis = args.at(1).isUndefinedOrNull() ? exec->globalThisValue() : args.at(1).toObject(exec);

    AJValue result = jsBoolean(true);

    unsigned length = thisObj->get(exec, exec->propertyNames().length).toUInt32(exec);
    unsigned k = 0;
    if (callType == CallTypeJS && isAJArray(&exec->globalData(), thisObj)) {
        AJFunction* f = asFunction(function);
        AJArray* array = asArray(thisObj);
        CachedCall cachedCall(exec, f, 3, exec->exceptionSlot());
        for (; k < length && !exec->hadException(); ++k) {
            if (UNLIKELY(!array->canGetIndex(k)))
                break;
            
            cachedCall.setThis(applyThis);
            cachedCall.setArgument(0, array->getIndex(k));
            cachedCall.setArgument(1, jsNumber(exec, k));
            cachedCall.setArgument(2, thisObj);
            AJValue result = cachedCall.call();
            if (!result.toBoolean(cachedCall.newCallFrame(exec)))
                return jsBoolean(false);
        }
    }
    for (; k < length && !exec->hadException(); ++k) {
        PropertySlot slot(thisObj);

        if (!thisObj->getPropertySlot(exec, k, slot))
            continue;

        MarkedArgumentBuffer eachArguments;

        eachArguments.append(slot.getValue(exec, k));
        eachArguments.append(jsNumber(exec, k));
        eachArguments.append(thisObj);

        bool predicateResult = call(exec, function, callType, callData, applyThis, eachArguments).toBoolean(exec);

        if (!predicateResult) {
            result = jsBoolean(false);
            break;
        }
    }

    return result;
}

AJValue JSC_HOST_CALL arrayProtoFuncForEach(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    AJObject* thisObj = thisValue.toThisObject(exec);

    AJValue function = args.at(0);
    CallData callData;
    CallType callType = function.getCallData(callData);
    if (callType == CallTypeNone)
        return throwError(exec, TypeError);

    AJObject* applyThis = args.at(1).isUndefinedOrNull() ? exec->globalThisValue() : args.at(1).toObject(exec);

    unsigned length = thisObj->get(exec, exec->propertyNames().length).toUInt32(exec);
    unsigned k = 0;
    if (callType == CallTypeJS && isAJArray(&exec->globalData(), thisObj)) {
        AJFunction* f = asFunction(function);
        AJArray* array = asArray(thisObj);
        CachedCall cachedCall(exec, f, 3, exec->exceptionSlot());
        for (; k < length && !exec->hadException(); ++k) {
            if (UNLIKELY(!array->canGetIndex(k)))
                break;

            cachedCall.setThis(applyThis);
            cachedCall.setArgument(0, array->getIndex(k));
            cachedCall.setArgument(1, jsNumber(exec, k));
            cachedCall.setArgument(2, thisObj);

            cachedCall.call();
        }
    }
    for (; k < length && !exec->hadException(); ++k) {
        PropertySlot slot(thisObj);
        if (!thisObj->getPropertySlot(exec, k, slot))
            continue;

        MarkedArgumentBuffer eachArguments;
        eachArguments.append(slot.getValue(exec, k));
        eachArguments.append(jsNumber(exec, k));
        eachArguments.append(thisObj);

        call(exec, function, callType, callData, applyThis, eachArguments);
    }
    return jsUndefined();
}

AJValue JSC_HOST_CALL arrayProtoFuncSome(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    AJObject* thisObj = thisValue.toThisObject(exec);

    AJValue function = args.at(0);
    CallData callData;
    CallType callType = function.getCallData(callData);
    if (callType == CallTypeNone)
        return throwError(exec, TypeError);

    AJObject* applyThis = args.at(1).isUndefinedOrNull() ? exec->globalThisValue() : args.at(1).toObject(exec);

    AJValue result = jsBoolean(false);

    unsigned length = thisObj->get(exec, exec->propertyNames().length).toUInt32(exec);
    unsigned k = 0;
    if (callType == CallTypeJS && isAJArray(&exec->globalData(), thisObj)) {
        AJFunction* f = asFunction(function);
        AJArray* array = asArray(thisObj);
        CachedCall cachedCall(exec, f, 3, exec->exceptionSlot());
        for (; k < length && !exec->hadException(); ++k) {
            if (UNLIKELY(!array->canGetIndex(k)))
                break;
            
            cachedCall.setThis(applyThis);
            cachedCall.setArgument(0, array->getIndex(k));
            cachedCall.setArgument(1, jsNumber(exec, k));
            cachedCall.setArgument(2, thisObj);
            AJValue result = cachedCall.call();
            if (result.toBoolean(cachedCall.newCallFrame(exec)))
                return jsBoolean(true);
        }
    }
    for (; k < length && !exec->hadException(); ++k) {
        PropertySlot slot(thisObj);
        if (!thisObj->getPropertySlot(exec, k, slot))
            continue;

        MarkedArgumentBuffer eachArguments;
        eachArguments.append(slot.getValue(exec, k));
        eachArguments.append(jsNumber(exec, k));
        eachArguments.append(thisObj);

        bool predicateResult = call(exec, function, callType, callData, applyThis, eachArguments).toBoolean(exec);

        if (predicateResult) {
            result = jsBoolean(true);
            break;
        }
    }
    return result;
}

AJValue JSC_HOST_CALL arrayProtoFuncReduce(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    AJObject* thisObj = thisValue.toThisObject(exec);
    
    AJValue function = args.at(0);
    CallData callData;
    CallType callType = function.getCallData(callData);
    if (callType == CallTypeNone)
        return throwError(exec, TypeError);

    unsigned i = 0;
    AJValue rv;
    unsigned length = thisObj->get(exec, exec->propertyNames().length).toUInt32(exec);
    if (!length && args.size() == 1)
        return throwError(exec, TypeError);
    AJArray* array = 0;
    if (isAJArray(&exec->globalData(), thisObj))
        array = asArray(thisObj);

    if (args.size() >= 2)
        rv = args.at(1);
    else if (array && array->canGetIndex(0)){
        rv = array->getIndex(0);
        i = 1;
    } else {
        for (i = 0; i < length; i++) {
            rv = getProperty(exec, thisObj, i);
            if (rv)
                break;
        }
        if (!rv)
            return throwError(exec, TypeError);
        i++;
    }

    if (callType == CallTypeJS && array) {
        CachedCall cachedCall(exec, asFunction(function), 4, exec->exceptionSlot());
        for (; i < length && !exec->hadException(); ++i) {
            cachedCall.setThis(jsNull());
            cachedCall.setArgument(0, rv);
            AJValue v;
            if (LIKELY(array->canGetIndex(i)))
                v = array->getIndex(i);
            else
                break; // length has been made unsafe while we enumerate fallback to slow path
            cachedCall.setArgument(1, v);
            cachedCall.setArgument(2, jsNumber(exec, i));
            cachedCall.setArgument(3, array);
            rv = cachedCall.call();
        }
        if (i == length) // only return if we reached the end of the array
            return rv;
    }

    for (; i < length && !exec->hadException(); ++i) {
        AJValue prop = getProperty(exec, thisObj, i);
        if (!prop)
            continue;
        
        MarkedArgumentBuffer eachArguments;
        eachArguments.append(rv);
        eachArguments.append(prop);
        eachArguments.append(jsNumber(exec, i));
        eachArguments.append(thisObj);
        
        rv = call(exec, function, callType, callData, jsNull(), eachArguments);
    }
    return rv;
}

AJValue JSC_HOST_CALL arrayProtoFuncReduceRight(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    AJObject* thisObj = thisValue.toThisObject(exec);
    
    AJValue function = args.at(0);
    CallData callData;
    CallType callType = function.getCallData(callData);
    if (callType == CallTypeNone)
        return throwError(exec, TypeError);
    
    unsigned i = 0;
    AJValue rv;
    unsigned length = thisObj->get(exec, exec->propertyNames().length).toUInt32(exec);
    if (!length && args.size() == 1)
        return throwError(exec, TypeError);
    AJArray* array = 0;
    if (isAJArray(&exec->globalData(), thisObj))
        array = asArray(thisObj);
    
    if (args.size() >= 2)
        rv = args.at(1);
    else if (array && array->canGetIndex(length - 1)){
        rv = array->getIndex(length - 1);
        i = 1;
    } else {
        for (i = 0; i < length; i++) {
            rv = getProperty(exec, thisObj, length - i - 1);
            if (rv)
                break;
        }
        if (!rv)
            return throwError(exec, TypeError);
        i++;
    }
    
    if (callType == CallTypeJS && array) {
        CachedCall cachedCall(exec, asFunction(function), 4, exec->exceptionSlot());
        for (; i < length && !exec->hadException(); ++i) {
            unsigned idx = length - i - 1;
            cachedCall.setThis(jsNull());
            cachedCall.setArgument(0, rv);
            if (UNLIKELY(!array->canGetIndex(idx)))
                break; // length has been made unsafe while we enumerate fallback to slow path
            cachedCall.setArgument(1, array->getIndex(idx));
            cachedCall.setArgument(2, jsNumber(exec, idx));
            cachedCall.setArgument(3, array);
            rv = cachedCall.call();
        }
        if (i == length) // only return if we reached the end of the array
            return rv;
    }
    
    for (; i < length && !exec->hadException(); ++i) {
        unsigned idx = length - i - 1;
        AJValue prop = getProperty(exec, thisObj, idx);
        if (!prop)
            continue;
        
        MarkedArgumentBuffer eachArguments;
        eachArguments.append(rv);
        eachArguments.append(prop);
        eachArguments.append(jsNumber(exec, idx));
        eachArguments.append(thisObj);
        
        rv = call(exec, function, callType, callData, jsNull(), eachArguments);
    }
    return rv;        
}

AJValue JSC_HOST_CALL arrayProtoFuncIndexOf(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    // AJ 1.5 Extension by Mozilla
    // Documentation: http://developer.mozilla.org/en/docs/Core_AJ_1.5_Reference:Global_Objects:Array:indexOf

    AJObject* thisObj = thisValue.toThisObject(exec);

    unsigned index = 0;
    double d = args.at(1).toInteger(exec);
    unsigned length = thisObj->get(exec, exec->propertyNames().length).toUInt32(exec);
    if (d < 0)
        d += length;
    if (d > 0) {
        if (d > length)
            index = length;
        else
            index = static_cast<unsigned>(d);
    }

    AJValue searchElement = args.at(0);
    for (; index < length; ++index) {
        AJValue e = getProperty(exec, thisObj, index);
        if (!e)
            continue;
        if (AJValue::strictEqual(exec, searchElement, e))
            return jsNumber(exec, index);
    }

    return jsNumber(exec, -1);
}

AJValue JSC_HOST_CALL arrayProtoFuncLastIndexOf(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    // AJ 1.6 Extension by Mozilla
    // Documentation: http://developer.mozilla.org/en/docs/Core_AJ_1.5_Reference:Global_Objects:Array:lastIndexOf

    AJObject* thisObj = thisValue.toThisObject(exec);

    unsigned length = thisObj->get(exec, exec->propertyNames().length).toUInt32(exec);
    int index = length - 1;
    double d = args.at(1).toIntegerPreserveNaN(exec);

    if (d < 0) {
        d += length;
        if (d < 0)
            return jsNumber(exec, -1);
    }
    if (d < length)
        index = static_cast<int>(d);

    AJValue searchElement = args.at(0);
    for (; index >= 0; --index) {
        AJValue e = getProperty(exec, thisObj, index);
        if (!e)
            continue;
        if (AJValue::strictEqual(exec, searchElement, e))
            return jsNumber(exec, index);
    }

    return jsNumber(exec, -1);
}

} // namespace AJ
