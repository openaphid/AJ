
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
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef Label_h
#define Label_h

#include "CodeBlock.h"
#include "Instruction.h"
#include <wtf/Assertions.h>
#include <wtf/Vector.h>
#include <limits.h>

namespace AJ {

    class Label {
    public:
        explicit Label(CodeBlock* codeBlock)
            : m_refCount(0)
            , m_location(invalidLocation)
            , m_codeBlock(codeBlock)
        {
        }

        void setLocation(unsigned location)
        {
            m_location = location;

            unsigned size = m_unresolvedJumps.size();
            for (unsigned i = 0; i < size; ++i)
                m_codeBlock->instructions()[m_unresolvedJumps[i].second].u.operand = m_location - m_unresolvedJumps[i].first;
        }

        int bind(int opcode, int offset) const
        {
            if (m_location == invalidLocation) {
                m_unresolvedJumps.append(std::make_pair(opcode, offset));
                return 0;
            }
            return m_location - opcode;
        }

        void ref() { ++m_refCount; }
        void deref()
        {
            --m_refCount;
            ASSERT(m_refCount >= 0);
        }
        int refCount() const { return m_refCount; }

        bool isForward() const { return m_location == invalidLocation; }

    private:
        typedef Vector<std::pair<int, int>, 8> JumpVector;

        static const unsigned invalidLocation = UINT_MAX;

        int m_refCount;
        unsigned m_location;
        CodeBlock* m_codeBlock;
        mutable JumpVector m_unresolvedJumps;
    };

} // namespace AJ

#endif // Label_h