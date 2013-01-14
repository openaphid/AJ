
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
 *  Copyright (C) 2003, 2006, 2008 Apple Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "ScopeChain.h"

#include "JSActivation.h"
#include "AJGlobalObject.h"
#include "AJObject.h"
#include "PropertyNameArray.h"
#include <stdio.h>

namespace AJ {

#ifndef NDEBUG

void ScopeChainNode::print() const
{
    ScopeChainIterator scopeEnd = end();
    for (ScopeChainIterator scopeIter = begin(); scopeIter != scopeEnd; ++scopeIter) {
        AJObject* o = *scopeIter;
        PropertyNameArray propertyNames(globalObject->globalExec());
        o->getPropertyNames(globalObject->globalExec(), propertyNames);
        PropertyNameArray::const_iterator propEnd = propertyNames.end();

        fprintf(stderr, "----- [scope %p] -----\n", o);
        for (PropertyNameArray::const_iterator propIter = propertyNames.begin(); propIter != propEnd; propIter++) {
            Identifier name = *propIter;
            fprintf(stderr, "%s, ", name.ascii());
        }
        fprintf(stderr, "\n");
    }
}

#endif

int ScopeChain::localDepth() const
{
    int scopeDepth = 0;
    ScopeChainIterator iter = this->begin();
    ScopeChainIterator end = this->end();
    while (!(*iter)->inherits(&JSActivation::info)) {
        ++iter;
        if (iter == end)
            break;
        ++scopeDepth;
    }
    return scopeDepth;
}

} // namespace AJ
