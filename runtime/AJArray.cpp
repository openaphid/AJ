
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "config.h"
#include "AJArray.h"

#include "ArrayPrototype.h"
#include "CachedCall.h"
#include "Error.h"
#include "Executable.h"
#include "PropertyNameArray.h"
#include <wtf/AVLTree.h>
#include <wtf/Assertions.h>
#include <wtf/OwnPtr.h>
#include <Operations.h>

#define CHECK_ARRAY_CONSISTENCY 0

using namespace std;
using namespace ATF;

namespace AJ {

ASSERT_CLASS_FITS_IN_CELL(AJArray);

// Overview of AJArray
//
// Properties of AJArray objects may be stored in one of three locations:
//   * The regular AJObject property map.
//   * A storage vector.
//   * A sparse map of array entries.
//
// Properties with non-numeric identifiers, with identifiers that are not representable
// as an unsigned integer, or where the value is greater than  MAX_ARRAY_INDEX
// (specifically, this is only one property - the value 0xFFFFFFFFU as an unsigned 32-bit
// integer) are not considered array indices and will be stored in the AJObject property map.
//
// All properties with a numeric identifer, representable as an unsigned integer i,
// where (i <= MAX_ARRAY_INDEX), are an array index and will be stored in either the
// storage vector or the sparse map.  An array index i will be handled in the following
// fashion:
//
//   * Where (i < MIN_SPARSE_ARRAY_INDEX) the value will be stored in the storage vector.
//   * Where (MIN_SPARSE_ARRAY_INDEX <= i <= MAX_STORAGE_VECTOR_INDEX) the value will either
//     be stored in the storage vector or in the sparse array, depending on the density of
//     data that would be stored in the vector (a vector being used where at least
//     (1 / minDensityMultiplier) of the entries would be populated).
//   * Where (MAX_STORAGE_VECTOR_INDEX < i <= MAX_ARRAY_INDEX) the value will always be stored
//     in the sparse array.

// The definition of MAX_STORAGE_VECTOR_LENGTH is dependant on the definition storageSize
// function below - the MAX_STORAGE_VECTOR_LENGTH limit is defined such that the storage
// size calculation cannot overflow.  (sizeof(ArrayStorage) - sizeof(AJValue)) +
// (vectorLength * sizeof(AJValue)) must be <= 0xFFFFFFFFU (which is maximum value of size_t).
#define MAX_STORAGE_VECTOR_LENGTH static_cast<unsigned>((0xFFFFFFFFU - (sizeof(ArrayStorage) - sizeof(AJValue))) / sizeof(AJValue))

// These values have to be macros to be used in max() and min() without introducing
// a PIC branch in Mach-O binaries, see <rdar://problem/5971391>.
#define MIN_SPARSE_ARRAY_INDEX 10000U
#define MAX_STORAGE_VECTOR_INDEX (MAX_STORAGE_VECTOR_LENGTH - 1)
// 0xFFFFFFFF is a bit weird -- is not an array index even though it's an integer.
#define MAX_ARRAY_INDEX 0xFFFFFFFEU

// Our policy for when to use a vector and when to use a sparse map.
// For all array indices under MIN_SPARSE_ARRAY_INDEX, we always use a vector.
// When indices greater than MIN_SPARSE_ARRAY_INDEX are involved, we use a vector
// as long as it is 1/8 full. If more sparse than that, we use a map.
static const unsigned minDensityMultiplier = 8;

const ClassInfo AJArray::info = {"Array", 0, 0, 0};

static inline size_t storageSize(unsigned vectorLength)
{
    ASSERT(vectorLength <= MAX_STORAGE_VECTOR_LENGTH);

    // MAX_STORAGE_VECTOR_LENGTH is defined such that provided (vectorLength <= MAX_STORAGE_VECTOR_LENGTH)
    // - as asserted above - the following calculation cannot overflow.
    size_t size = (sizeof(ArrayStorage) - sizeof(AJValue)) + (vectorLength * sizeof(AJValue));
    // Assertion to detect integer overflow in previous calculation (should not be possible, provided that
    // MAX_STORAGE_VECTOR_LENGTH is correctly defined).
    ASSERT(((size - (sizeof(ArrayStorage) - sizeof(AJValue))) / sizeof(AJValue) == vectorLength) && (size >= (sizeof(ArrayStorage) - sizeof(AJValue))));

    return size;
}

static inline unsigned increasedVectorLength(unsigned newLength)
{
    ASSERT(newLength <= MAX_STORAGE_VECTOR_LENGTH);

    // Mathematically equivalent to:
    //   increasedLength = (newLength * 3 + 1) / 2;
    // or:
    //   increasedLength = (unsigned)ceil(newLength * 1.5));
    // This form is not prone to internal overflow.
    unsigned increasedLength = newLength + (newLength >> 1) + (newLength & 1);
    ASSERT(increasedLength >= newLength);

    return min(increasedLength, MAX_STORAGE_VECTOR_LENGTH);
}

static inline bool isDenseEnoughForVector(unsigned length, unsigned numValues)
{
    return length / minDensityMultiplier <= numValues;
}

#if !CHECK_ARRAY_CONSISTENCY

inline void AJArray::checkConsistency(ConsistencyCheckType)
{
}

#endif

AJArray::AJArray(NonNullPassRefPtr<Structure> structure)
    : AJObject(structure)
{
    unsigned initialCapacity = 0;

    m_storage = static_cast<ArrayStorage*>(fastZeroedMalloc(storageSize(initialCapacity)));
    m_vectorLength = initialCapacity;

    checkConsistency();
}

AJArray::AJArray(NonNullPassRefPtr<Structure> structure, unsigned initialLength)
    : AJObject(structure)
{
    unsigned initialCapacity = min(initialLength, MIN_SPARSE_ARRAY_INDEX);

    m_storage = static_cast<ArrayStorage*>(fastMalloc(storageSize(initialCapacity)));
    m_storage->m_length = initialLength;
    m_vectorLength = initialCapacity;
    m_storage->m_numValuesInVector = 0;
    m_storage->m_sparseValueMap = 0;
    m_storage->subclassData = 0;
    m_storage->reportedMapCapacity = 0;

    AJValue* vector = m_storage->m_vector;
    for (size_t i = 0; i < initialCapacity; ++i)
        vector[i] = AJValue();

    checkConsistency();

    Heap::heap(this)->reportExtraMemoryCost(initialCapacity * sizeof(AJValue));
}

AJArray::AJArray(NonNullPassRefPtr<Structure> structure, const ArgList& list)
    : AJObject(structure)
{
    unsigned initialCapacity = list.size();

    m_storage = static_cast<ArrayStorage*>(fastMalloc(storageSize(initialCapacity)));
    m_storage->m_length = initialCapacity;
    m_vectorLength = initialCapacity;
    m_storage->m_numValuesInVector = initialCapacity;
    m_storage->m_sparseValueMap = 0;
    m_storage->subclassData = 0;
    m_storage->reportedMapCapacity = 0;

    size_t i = 0;
    ArgList::const_iterator end = list.end();
    for (ArgList::const_iterator it = list.begin(); it != end; ++it, ++i)
        m_storage->m_vector[i] = *it;

    checkConsistency();

    Heap::heap(this)->reportExtraMemoryCost(storageSize(initialCapacity));
}

AJArray::~AJArray()
{
    ASSERT(vptr() == AJGlobalData::jsArrayVPtr);
    checkConsistency(DestructorConsistencyCheck);

    delete m_storage->m_sparseValueMap;
    fastFree(m_storage);
}

bool AJArray::getOwnPropertySlot(ExecState* exec, unsigned i, PropertySlot& slot)
{
    ArrayStorage* storage = m_storage;

    if (i >= storage->m_length) {
        if (i > MAX_ARRAY_INDEX)
            return getOwnPropertySlot(exec, Identifier::from(exec, i), slot);
        return false;
    }

    if (i < m_vectorLength) {
        AJValue& valueSlot = storage->m_vector[i];
        if (valueSlot) {
            slot.setValueSlot(&valueSlot);
            return true;
        }
    } else if (SparseArrayValueMap* map = storage->m_sparseValueMap) {
        if (i >= MIN_SPARSE_ARRAY_INDEX) {
            SparseArrayValueMap::iterator it = map->find(i);
            if (it != map->end()) {
                slot.setValueSlot(&it->second);
                return true;
            }
        }
    }

    return AJObject::getOwnPropertySlot(exec, Identifier::from(exec, i), slot);
}

bool AJArray::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    if (propertyName == exec->propertyNames().length) {
        slot.setValue(jsNumber(exec, length()));
        return true;
    }

    bool isArrayIndex;
    unsigned i = propertyName.toArrayIndex(&isArrayIndex);
    if (isArrayIndex)
        return AJArray::getOwnPropertySlot(exec, i, slot);

    return AJObject::getOwnPropertySlot(exec, propertyName, slot);
}

bool AJArray::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    if (propertyName == exec->propertyNames().length) {
        descriptor.setDescriptor(jsNumber(exec, length()), DontDelete | DontEnum);
        return true;
    }
    
    bool isArrayIndex;
    unsigned i = propertyName.toArrayIndex(&isArrayIndex);
    if (isArrayIndex) {
        if (i >= m_storage->m_length)
            return false;
        if (i < m_vectorLength) {
            AJValue& value = m_storage->m_vector[i];
            if (value) {
                descriptor.setDescriptor(value, 0);
                return true;
            }
        } else if (SparseArrayValueMap* map = m_storage->m_sparseValueMap) {
            if (i >= MIN_SPARSE_ARRAY_INDEX) {
                SparseArrayValueMap::iterator it = map->find(i);
                if (it != map->end()) {
                    descriptor.setDescriptor(it->second, 0);
                    return true;
                }
            }
        }
    }
    return AJObject::getOwnPropertyDescriptor(exec, propertyName, descriptor);
}

// ECMA 15.4.5.1
void AJArray::put(ExecState* exec, const Identifier& propertyName, AJValue value, PutPropertySlot& slot)
{
    bool isArrayIndex;
    unsigned i = propertyName.toArrayIndex(&isArrayIndex);
    if (isArrayIndex) {
        put(exec, i, value);
        return;
    }

    if (propertyName == exec->propertyNames().length) {
        unsigned newLength = value.toUInt32(exec);
        if (value.toNumber(exec) != static_cast<double>(newLength)) {
            throwError(exec, RangeError, "Invalid array length.");
            return;
        }
        setLength(newLength);
        return;
    }

    AJObject::put(exec, propertyName, value, slot);
}

void AJArray::put(ExecState* exec, unsigned i, AJValue value)
{
    checkConsistency();

    unsigned length = m_storage->m_length;
    if (i >= length && i <= MAX_ARRAY_INDEX) {
        length = i + 1;
        m_storage->m_length = length;
    }

    if (i < m_vectorLength) {
        AJValue& valueSlot = m_storage->m_vector[i];
        if (valueSlot) {
            valueSlot = value;
            checkConsistency();
            return;
        }
        valueSlot = value;
        ++m_storage->m_numValuesInVector;
        checkConsistency();
        return;
    }

    putSlowCase(exec, i, value);
}

NEVER_INLINE void AJArray::putSlowCase(ExecState* exec, unsigned i, AJValue value)
{
    ArrayStorage* storage = m_storage;
    SparseArrayValueMap* map = storage->m_sparseValueMap;

    if (i >= MIN_SPARSE_ARRAY_INDEX) {
        if (i > MAX_ARRAY_INDEX) {
            PutPropertySlot slot;
            put(exec, Identifier::from(exec, i), value, slot);
            return;
        }

        // We miss some cases where we could compact the storage, such as a large array that is being filled from the end
        // (which will only be compacted as we reach indices that are less than MIN_SPARSE_ARRAY_INDEX) - but this makes the check much faster.
        if ((i > MAX_STORAGE_VECTOR_INDEX) || !isDenseEnoughForVector(i + 1, storage->m_numValuesInVector + 1)) {
            if (!map) {
                map = new SparseArrayValueMap;
                storage->m_sparseValueMap = map;
            }

            pair<SparseArrayValueMap::iterator, bool> result = map->add(i, value);
            if (!result.second) { // pre-existing entry
                result.first->second = value;
                return;
            }

            size_t capacity = map->capacity();
            if (capacity != storage->reportedMapCapacity) {
                Heap::heap(this)->reportExtraMemoryCost((capacity - storage->reportedMapCapacity) * (sizeof(unsigned) + sizeof(AJValue)));
                storage->reportedMapCapacity = capacity;
            }
            return;
        }
    }

    // We have decided that we'll put the new item into the vector.
    // Fast case is when there is no sparse map, so we can increase the vector size without moving values from it.
    if (!map || map->isEmpty()) {
        if (increaseVectorLength(i + 1)) {
            storage = m_storage;
            storage->m_vector[i] = value;
            ++storage->m_numValuesInVector;
            checkConsistency();
        } else
            throwOutOfMemoryError(exec);
        return;
    }

    // Decide how many values it would be best to move from the map.
    unsigned newNumValuesInVector = storage->m_numValuesInVector + 1;
    unsigned newVectorLength = increasedVectorLength(i + 1);
    for (unsigned j = max(m_vectorLength, MIN_SPARSE_ARRAY_INDEX); j < newVectorLength; ++j)
        newNumValuesInVector += map->contains(j);
    if (i >= MIN_SPARSE_ARRAY_INDEX)
        newNumValuesInVector -= map->contains(i);
    if (isDenseEnoughForVector(newVectorLength, newNumValuesInVector)) {
        unsigned proposedNewNumValuesInVector = newNumValuesInVector;
        // If newVectorLength is already the maximum - MAX_STORAGE_VECTOR_LENGTH - then do not attempt to grow any further.
        while (newVectorLength < MAX_STORAGE_VECTOR_LENGTH) {
            unsigned proposedNewVectorLength = increasedVectorLength(newVectorLength + 1);
            for (unsigned j = max(newVectorLength, MIN_SPARSE_ARRAY_INDEX); j < proposedNewVectorLength; ++j)
                proposedNewNumValuesInVector += map->contains(j);
            if (!isDenseEnoughForVector(proposedNewVectorLength, proposedNewNumValuesInVector))
                break;
            newVectorLength = proposedNewVectorLength;
            newNumValuesInVector = proposedNewNumValuesInVector;
        }
    }

    if (!tryFastRealloc(storage, storageSize(newVectorLength)).getValue(storage)) {
        throwOutOfMemoryError(exec);
        return;
    }

    unsigned vectorLength = m_vectorLength;

    if (newNumValuesInVector == storage->m_numValuesInVector + 1) {
        for (unsigned j = vectorLength; j < newVectorLength; ++j)
            storage->m_vector[j] = AJValue();
        if (i > MIN_SPARSE_ARRAY_INDEX)
            map->remove(i);
    } else {
        for (unsigned j = vectorLength; j < max(vectorLength, MIN_SPARSE_ARRAY_INDEX); ++j)
            storage->m_vector[j] = AJValue();
        for (unsigned j = max(vectorLength, MIN_SPARSE_ARRAY_INDEX); j < newVectorLength; ++j)
            storage->m_vector[j] = map->take(j);
    }

    storage->m_vector[i] = value;

    m_vectorLength = newVectorLength;
    storage->m_numValuesInVector = newNumValuesInVector;

    m_storage = storage;

    checkConsistency();

    Heap::heap(this)->reportExtraMemoryCost(storageSize(newVectorLength) - storageSize(vectorLength));
}

bool AJArray::deleteProperty(ExecState* exec, const Identifier& propertyName)
{
    bool isArrayIndex;
    unsigned i = propertyName.toArrayIndex(&isArrayIndex);
    if (isArrayIndex)
        return deleteProperty(exec, i);

    if (propertyName == exec->propertyNames().length)
        return false;

    return AJObject::deleteProperty(exec, propertyName);
}

bool AJArray::deleteProperty(ExecState* exec, unsigned i)
{
    checkConsistency();

    ArrayStorage* storage = m_storage;

    if (i < m_vectorLength) {
        AJValue& valueSlot = storage->m_vector[i];
        if (!valueSlot) {
            checkConsistency();
            return false;
        }
        valueSlot = AJValue();
        --storage->m_numValuesInVector;
        checkConsistency();
        return true;
    }

    if (SparseArrayValueMap* map = storage->m_sparseValueMap) {
        if (i >= MIN_SPARSE_ARRAY_INDEX) {
            SparseArrayValueMap::iterator it = map->find(i);
            if (it != map->end()) {
                map->remove(it);
                checkConsistency();
                return true;
            }
        }
    }

    checkConsistency();

    if (i > MAX_ARRAY_INDEX)
        return deleteProperty(exec, Identifier::from(exec, i));

    return false;
}

void AJArray::getOwnPropertyNames(ExecState* exec, PropertyNameArray& propertyNames, EnumerationMode mode)
{
    // FIXME: Filling PropertyNameArray with an identifier for every integer
    // is incredibly inefficient for large arrays. We need a different approach,
    // which almost certainly means a different structure for PropertyNameArray.

    ArrayStorage* storage = m_storage;

    unsigned usedVectorLength = min(storage->m_length, m_vectorLength);
    for (unsigned i = 0; i < usedVectorLength; ++i) {
        if (storage->m_vector[i])
            propertyNames.add(Identifier::from(exec, i));
    }

    if (SparseArrayValueMap* map = storage->m_sparseValueMap) {
        SparseArrayValueMap::iterator end = map->end();
        for (SparseArrayValueMap::iterator it = map->begin(); it != end; ++it)
            propertyNames.add(Identifier::from(exec, it->first));
    }

    if (mode == IncludeDontEnumProperties)
        propertyNames.add(exec->propertyNames().length);

    AJObject::getOwnPropertyNames(exec, propertyNames, mode);
}

bool AJArray::increaseVectorLength(unsigned newLength)
{
    // This function leaves the array in an internally inconsistent state, because it does not move any values from sparse value map
    // to the vector. Callers have to account for that, because they can do it more efficiently.

    ArrayStorage* storage = m_storage;

    unsigned vectorLength = m_vectorLength;
    ASSERT(newLength > vectorLength);
    ASSERT(newLength <= MAX_STORAGE_VECTOR_INDEX);
    unsigned newVectorLength = increasedVectorLength(newLength);

    if (!tryFastRealloc(storage, storageSize(newVectorLength)).getValue(storage))
        return false;

    m_vectorLength = newVectorLength;

    for (unsigned i = vectorLength; i < newVectorLength; ++i)
        storage->m_vector[i] = AJValue();

    m_storage = storage;

    Heap::heap(this)->reportExtraMemoryCost(storageSize(newVectorLength) - storageSize(vectorLength));

    return true;
}

void AJArray::setLength(unsigned newLength)
{
    checkConsistency();

    ArrayStorage* storage = m_storage;

    unsigned length = m_storage->m_length;

    if (newLength < length) {
        unsigned usedVectorLength = min(length, m_vectorLength);
        for (unsigned i = newLength; i < usedVectorLength; ++i) {
            AJValue& valueSlot = storage->m_vector[i];
            bool hadValue = valueSlot;
            valueSlot = AJValue();
            storage->m_numValuesInVector -= hadValue;
        }

        if (SparseArrayValueMap* map = storage->m_sparseValueMap) {
            SparseArrayValueMap copy = *map;
            SparseArrayValueMap::iterator end = copy.end();
            for (SparseArrayValueMap::iterator it = copy.begin(); it != end; ++it) {
                if (it->first >= newLength)
                    map->remove(it->first);
            }
            if (map->isEmpty()) {
                delete map;
                storage->m_sparseValueMap = 0;
            }
        }
    }

    m_storage->m_length = newLength;

    checkConsistency();
}

AJValue AJArray::pop()
{
    checkConsistency();

    unsigned length = m_storage->m_length;
    if (!length)
        return jsUndefined();

    --length;

    AJValue result;

    if (length < m_vectorLength) {
        AJValue& valueSlot = m_storage->m_vector[length];
        if (valueSlot) {
            --m_storage->m_numValuesInVector;
            result = valueSlot;
            valueSlot = AJValue();
        } else
            result = jsUndefined();
    } else {
        result = jsUndefined();
        if (SparseArrayValueMap* map = m_storage->m_sparseValueMap) {
            SparseArrayValueMap::iterator it = map->find(length);
            if (it != map->end()) {
                result = it->second;
                map->remove(it);
                if (map->isEmpty()) {
                    delete map;
                    m_storage->m_sparseValueMap = 0;
                }
            }
        }
    }

    m_storage->m_length = length;

    checkConsistency();

    return result;
}

void AJArray::push(ExecState* exec, AJValue value)
{
    checkConsistency();

    if (m_storage->m_length < m_vectorLength) {
        m_storage->m_vector[m_storage->m_length] = value;
        ++m_storage->m_numValuesInVector;
        ++m_storage->m_length;
        checkConsistency();
        return;
    }

    if (m_storage->m_length < MIN_SPARSE_ARRAY_INDEX) {
        SparseArrayValueMap* map = m_storage->m_sparseValueMap;
        if (!map || map->isEmpty()) {
            if (increaseVectorLength(m_storage->m_length + 1)) {
                m_storage->m_vector[m_storage->m_length] = value;
                ++m_storage->m_numValuesInVector;
                ++m_storage->m_length;
                checkConsistency();
                return;
            }
            checkConsistency();
            throwOutOfMemoryError(exec);
            return;
        }
    }

    putSlowCase(exec, m_storage->m_length++, value);
}

void AJArray::markChildren(MarkStack& markStack)
{
    markChildrenDirect(markStack);
}

static int compareNumbersForQSort(const void* a, const void* b)
{
    double da = static_cast<const AJValue*>(a)->uncheckedGetNumber();
    double db = static_cast<const AJValue*>(b)->uncheckedGetNumber();
    return (da > db) - (da < db);
}

static int compareByStringPairForQSort(const void* a, const void* b)
{
    const ValueStringPair* va = static_cast<const ValueStringPair*>(a);
    const ValueStringPair* vb = static_cast<const ValueStringPair*>(b);
    return compare(va->second, vb->second);
}

void AJArray::sortNumeric(ExecState* exec, AJValue compareFunction, CallType callType, const CallData& callData)
{
    unsigned lengthNotIncludingUndefined = compactForSorting();
    if (m_storage->m_sparseValueMap) {
        throwOutOfMemoryError(exec);
        return;
    }

    if (!lengthNotIncludingUndefined)
        return;
        
    bool allValuesAreNumbers = true;
    size_t size = m_storage->m_numValuesInVector;
    for (size_t i = 0; i < size; ++i) {
        if (!m_storage->m_vector[i].isNumber()) {
            allValuesAreNumbers = false;
            break;
        }
    }

    if (!allValuesAreNumbers)
        return sort(exec, compareFunction, callType, callData);

    // For numeric comparison, which is fast, qsort is faster than mergesort. We
    // also don't require mergesort's stability, since there's no user visible
    // side-effect from swapping the order of equal primitive values.
    qsort(m_storage->m_vector, size, sizeof(AJValue), compareNumbersForQSort);

    checkConsistency(SortConsistencyCheck);
}

void AJArray::sort(ExecState* exec)
{
    unsigned lengthNotIncludingUndefined = compactForSorting();
    if (m_storage->m_sparseValueMap) {
        throwOutOfMemoryError(exec);
        return;
    }

    if (!lengthNotIncludingUndefined)
        return;

    // Converting AJ values to strings can be expensive, so we do it once up front and sort based on that.
    // This is a considerable improvement over doing it twice per comparison, though it requires a large temporary
    // buffer. Besides, this protects us from crashing if some objects have custom toString methods that return
    // random or otherwise changing results, effectively making compare function inconsistent.

    Vector<ValueStringPair> values(lengthNotIncludingUndefined);
    if (!values.begin()) {
        throwOutOfMemoryError(exec);
        return;
    }
    
    Heap::heap(this)->pushTempSortVector(&values);

    for (size_t i = 0; i < lengthNotIncludingUndefined; i++) {
        AJValue value = m_storage->m_vector[i];
        ASSERT(!value.isUndefined());
        values[i].first = value;
    }

    // FIXME: The following loop continues to call toString on subsequent values even after
    // a toString call raises an exception.

    for (size_t i = 0; i < lengthNotIncludingUndefined; i++)
        values[i].second = values[i].first.toString(exec);

    if (exec->hadException()) {
        Heap::heap(this)->popTempSortVector(&values);
        return;
    }

    // FIXME: Since we sort by string value, a fast algorithm might be to use a radix sort. That would be O(N) rather
    // than O(N log N).

#if HAVE(MERGESORT)
    mergesort(values.begin(), values.size(), sizeof(ValueStringPair), compareByStringPairForQSort);
#else
    // FIXME: The qsort library function is likely to not be a stable sort.
    // ECMAScript-262 does not specify a stable sort, but in practice, browsers perform a stable sort.
    qsort(values.begin(), values.size(), sizeof(ValueStringPair), compareByStringPairForQSort);
#endif

    // If the toString function changed the length of the array or vector storage,
    // increase the length to handle the orignal number of actual values.
    if (m_vectorLength < lengthNotIncludingUndefined)
        increaseVectorLength(lengthNotIncludingUndefined);
    if (m_storage->m_length < lengthNotIncludingUndefined)
        m_storage->m_length = lengthNotIncludingUndefined;
        
    for (size_t i = 0; i < lengthNotIncludingUndefined; i++)
        m_storage->m_vector[i] = values[i].first;

    Heap::heap(this)->popTempSortVector(&values);
    
    checkConsistency(SortConsistencyCheck);
}

struct AVLTreeNodeForArrayCompare {
    AJValue value;

    // Child pointers.  The high bit of gt is robbed and used as the
    // balance factor sign.  The high bit of lt is robbed and used as
    // the magnitude of the balance factor.
    int32_t gt;
    int32_t lt;
};

struct AVLTreeAbstractorForArrayCompare {
    typedef int32_t handle; // Handle is an index into m_nodes vector.
    typedef AJValue key;
    typedef int32_t size;

    Vector<AVLTreeNodeForArrayCompare> m_nodes;
    ExecState* m_exec;
    AJValue m_compareFunction;
    CallType m_compareCallType;
    const CallData* m_compareCallData;
    AJValue m_globalThisValue;
    OwnPtr<CachedCall> m_cachedCall;

    handle get_less(handle h) { return m_nodes[h].lt & 0x7FFFFFFF; }
    void set_less(handle h, handle lh) { m_nodes[h].lt &= 0x80000000; m_nodes[h].lt |= lh; }
    handle get_greater(handle h) { return m_nodes[h].gt & 0x7FFFFFFF; }
    void set_greater(handle h, handle gh) { m_nodes[h].gt &= 0x80000000; m_nodes[h].gt |= gh; }

    int get_balance_factor(handle h)
    {
        if (m_nodes[h].gt & 0x80000000)
            return -1;
        return static_cast<unsigned>(m_nodes[h].lt) >> 31;
    }

    void set_balance_factor(handle h, int bf)
    {
        if (bf == 0) {
            m_nodes[h].lt &= 0x7FFFFFFF;
            m_nodes[h].gt &= 0x7FFFFFFF;
        } else {
            m_nodes[h].lt |= 0x80000000;
            if (bf < 0)
                m_nodes[h].gt |= 0x80000000;
            else
                m_nodes[h].gt &= 0x7FFFFFFF;
        }
    }

    int compare_key_key(key va, key vb)
    {
        ASSERT(!va.isUndefined());
        ASSERT(!vb.isUndefined());

        if (m_exec->hadException())
            return 1;

        double compareResult;
        if (m_cachedCall) {
            m_cachedCall->setThis(m_globalThisValue);
            m_cachedCall->setArgument(0, va);
            m_cachedCall->setArgument(1, vb);
            compareResult = m_cachedCall->call().toNumber(m_cachedCall->newCallFrame(m_exec));
        } else {
            MarkedArgumentBuffer arguments;
            arguments.append(va);
            arguments.append(vb);
            compareResult = call(m_exec, m_compareFunction, m_compareCallType, *m_compareCallData, m_globalThisValue, arguments).toNumber(m_exec);
        }
        return (compareResult < 0) ? -1 : 1; // Not passing equality through, because we need to store all values, even if equivalent.
    }

    int compare_key_node(key k, handle h) { return compare_key_key(k, m_nodes[h].value); }
    int compare_node_node(handle h1, handle h2) { return compare_key_key(m_nodes[h1].value, m_nodes[h2].value); }

    static handle null() { return 0x7FFFFFFF; }
};

void AJArray::sort(ExecState* exec, AJValue compareFunction, CallType callType, const CallData& callData)
{
    checkConsistency();

    // FIXME: This ignores exceptions raised in the compare function or in toNumber.

    // The maximum tree depth is compiled in - but the caller is clearly up to no good
    // if a larger array is passed.
    ASSERT(m_storage->m_length <= static_cast<unsigned>(std::numeric_limits<int>::max()));
    if (m_storage->m_length > static_cast<unsigned>(std::numeric_limits<int>::max()))
        return;

    if (!m_storage->m_length)
        return;

    unsigned usedVectorLength = min(m_storage->m_length, m_vectorLength);

    AVLTree<AVLTreeAbstractorForArrayCompare, 44> tree; // Depth 44 is enough for 2^31 items
    tree.abstractor().m_exec = exec;
    tree.abstractor().m_compareFunction = compareFunction;
    tree.abstractor().m_compareCallType = callType;
    tree.abstractor().m_compareCallData = &callData;
    tree.abstractor().m_globalThisValue = exec->globalThisValue();
    tree.abstractor().m_nodes.resize(usedVectorLength + (m_storage->m_sparseValueMap ? m_storage->m_sparseValueMap->size() : 0));

    if (callType == CallTypeJS)
        tree.abstractor().m_cachedCall.set(new CachedCall(exec, asFunction(compareFunction), 2, exec->exceptionSlot()));

    if (!tree.abstractor().m_nodes.begin()) {
        throwOutOfMemoryError(exec);
        return;
    }

    // FIXME: If the compare function modifies the array, the vector, map, etc. could be modified
    // right out from under us while we're building the tree here.

    unsigned numDefined = 0;
    unsigned numUndefined = 0;

    // Iterate over the array, ignoring missing values, counting undefined ones, and inserting all other ones into the tree.
    for (; numDefined < usedVectorLength; ++numDefined) {
        AJValue v = m_storage->m_vector[numDefined];
        if (!v || v.isUndefined())
            break;
        tree.abstractor().m_nodes[numDefined].value = v;
        tree.insert(numDefined);
    }
    for (unsigned i = numDefined; i < usedVectorLength; ++i) {
        AJValue v = m_storage->m_vector[i];
        if (v) {
            if (v.isUndefined())
                ++numUndefined;
            else {
                tree.abstractor().m_nodes[numDefined].value = v;
                tree.insert(numDefined);
                ++numDefined;
            }
        }
    }

    unsigned newUsedVectorLength = numDefined + numUndefined;

    if (SparseArrayValueMap* map = m_storage->m_sparseValueMap) {
        newUsedVectorLength += map->size();
        if (newUsedVectorLength > m_vectorLength) {
            // Check that it is possible to allocate an array large enough to hold all the entries.
            if ((newUsedVectorLength > MAX_STORAGE_VECTOR_LENGTH) || !increaseVectorLength(newUsedVectorLength)) {
                throwOutOfMemoryError(exec);
                return;
            }
        }

        SparseArrayValueMap::iterator end = map->end();
        for (SparseArrayValueMap::iterator it = map->begin(); it != end; ++it) {
            tree.abstractor().m_nodes[numDefined].value = it->second;
            tree.insert(numDefined);
            ++numDefined;
        }

        delete map;
        m_storage->m_sparseValueMap = 0;
    }

    ASSERT(tree.abstractor().m_nodes.size() >= numDefined);

    // FIXME: If the compare function changed the length of the array, the following might be
    // modifying the vector incorrectly.

    // Copy the values back into m_storage.
    AVLTree<AVLTreeAbstractorForArrayCompare, 44>::Iterator iter;
    iter.start_iter_least(tree);
    for (unsigned i = 0; i < numDefined; ++i) {
        m_storage->m_vector[i] = tree.abstractor().m_nodes[*iter].value;
        ++iter;
    }

    // Put undefined values back in.
    for (unsigned i = numDefined; i < newUsedVectorLength; ++i)
        m_storage->m_vector[i] = jsUndefined();

    // Ensure that unused values in the vector are zeroed out.
    for (unsigned i = newUsedVectorLength; i < usedVectorLength; ++i)
        m_storage->m_vector[i] = AJValue();

    m_storage->m_numValuesInVector = newUsedVectorLength;

    checkConsistency(SortConsistencyCheck);
}

void AJArray::fillArgList(ExecState* exec, MarkedArgumentBuffer& args)
{
    AJValue* vector = m_storage->m_vector;
    unsigned vectorEnd = min(m_storage->m_length, m_vectorLength);
    unsigned i = 0;
    for (; i < vectorEnd; ++i) {
        AJValue& v = vector[i];
        if (!v)
            break;
        args.append(v);
    }

    for (; i < m_storage->m_length; ++i)
        args.append(get(exec, i));
}

void AJArray::copyToRegisters(ExecState* exec, Register* buffer, uint32_t maxSize)
{
    ASSERT(m_storage->m_length >= maxSize);
    UNUSED_PARAM(maxSize);
    AJValue* vector = m_storage->m_vector;
    unsigned vectorEnd = min(maxSize, m_vectorLength);
    unsigned i = 0;
    for (; i < vectorEnd; ++i) {
        AJValue& v = vector[i];
        if (!v)
            break;
        buffer[i] = v;
    }

    for (; i < maxSize; ++i)
        buffer[i] = get(exec, i);
}

unsigned AJArray::compactForSorting()
{
    checkConsistency();

    ArrayStorage* storage = m_storage;

    unsigned usedVectorLength = min(m_storage->m_length, m_vectorLength);

    unsigned numDefined = 0;
    unsigned numUndefined = 0;

    for (; numDefined < usedVectorLength; ++numDefined) {
        AJValue v = storage->m_vector[numDefined];
        if (!v || v.isUndefined())
            break;
    }
    for (unsigned i = numDefined; i < usedVectorLength; ++i) {
        AJValue v = storage->m_vector[i];
        if (v) {
            if (v.isUndefined())
                ++numUndefined;
            else
                storage->m_vector[numDefined++] = v;
        }
    }

    unsigned newUsedVectorLength = numDefined + numUndefined;

    if (SparseArrayValueMap* map = storage->m_sparseValueMap) {
        newUsedVectorLength += map->size();
        if (newUsedVectorLength > m_vectorLength) {
            // Check that it is possible to allocate an array large enough to hold all the entries - if not,
            // exception is thrown by caller.
            if ((newUsedVectorLength > MAX_STORAGE_VECTOR_LENGTH) || !increaseVectorLength(newUsedVectorLength))
                return 0;
            storage = m_storage;
        }

        SparseArrayValueMap::iterator end = map->end();
        for (SparseArrayValueMap::iterator it = map->begin(); it != end; ++it)
            storage->m_vector[numDefined++] = it->second;

        delete map;
        storage->m_sparseValueMap = 0;
    }

    for (unsigned i = numDefined; i < newUsedVectorLength; ++i)
        storage->m_vector[i] = jsUndefined();
    for (unsigned i = newUsedVectorLength; i < usedVectorLength; ++i)
        storage->m_vector[i] = AJValue();

    storage->m_numValuesInVector = newUsedVectorLength;

    checkConsistency(SortConsistencyCheck);

    return numDefined;
}

void* AJArray::subclassData() const
{
    return m_storage->subclassData;
}

void AJArray::setSubclassData(void* d)
{
    m_storage->subclassData = d;
}

#if CHECK_ARRAY_CONSISTENCY

void AJArray::checkConsistency(ConsistencyCheckType type)
{
    ASSERT(m_storage);
    if (type == SortConsistencyCheck)
        ASSERT(!m_storage->m_sparseValueMap);

    unsigned numValuesInVector = 0;
    for (unsigned i = 0; i < m_vectorLength; ++i) {
        if (AJValue value = m_storage->m_vector[i]) {
            ASSERT(i < m_storage->m_length);
            if (type != DestructorConsistencyCheck)
                value->type(); // Likely to crash if the object was deallocated.
            ++numValuesInVector;
        } else {
            if (type == SortConsistencyCheck)
                ASSERT(i >= m_storage->m_numValuesInVector);
        }
    }
    ASSERT(numValuesInVector == m_storage->m_numValuesInVector);
    ASSERT(numValuesInVector <= m_storage->m_length);

    if (m_storage->m_sparseValueMap) {
        SparseArrayValueMap::iterator end = m_storage->m_sparseValueMap->end();
        for (SparseArrayValueMap::iterator it = m_storage->m_sparseValueMap->begin(); it != end; ++it) {
            unsigned index = it->first;
            ASSERT(index < m_storage->m_length);
            ASSERT(index >= m_vectorLength);
            ASSERT(index <= MAX_ARRAY_INDEX);
            ASSERT(it->second);
            if (type != DestructorConsistencyCheck)
                it->second->type(); // Likely to crash if the object was deallocated.
        }
    }
}

#endif

} // namespace AJ
