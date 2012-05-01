
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
 * Copyright (C) 2009 Jian Li <jianli@chromium.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"

#include "ThreadSpecific.h"
#include <wtf/Noncopyable.h>

#if USE(PTHREADS)
#error This file should not be compiled by ports that do not use Windows native ThreadSpecific implementation.
#endif

namespace ATF {

long& tlsKeyCount()
{
    static long count;
    return count;
}

DWORD* tlsKeys()
{
    static DWORD keys[kMaxTlsKeySize];
    return keys;
}

void ThreadSpecificThreadExit()
{
    for (long i = 0; i < tlsKeyCount(); i++) {
        // The layout of ThreadSpecific<T>::Data does not depend on T. So we are safe to do the static cast to ThreadSpecific<int> in order to access its data member.
        ThreadSpecific<int>::Data* data = static_cast<ThreadSpecific<int>::Data*>(TlsGetValue(tlsKeys()[i]));
        if (data)
            data->destructor(data);
    }
}

} // namespace ATF
