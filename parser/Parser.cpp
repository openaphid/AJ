
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
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
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
#include "Parser.h"

#include "Debugger.h"
#include "Lexer.h"
#include <wtf/HashSet.h>
#include <wtf/Vector.h>

#ifndef yyparse
extern int jscyyparse(void*);
#endif

namespace AJ {

void Parser::parse(AJGlobalData* globalData, int* errLine, UString* errMsg)
{
    m_sourceElements = 0;

    int defaultErrLine;
    UString defaultErrMsg;

    if (!errLine)
        errLine = &defaultErrLine;
    if (!errMsg)
        errMsg = &defaultErrMsg;

    *errLine = -1;
    *errMsg = UString();

    Lexer& lexer = *globalData->lexer;
    lexer.setCode(*m_source, m_arena);

    int parseError = jscyyparse(globalData);
    bool lexError = lexer.sawError();
    int lineNumber = lexer.lineNumber();
    lexer.clear();

    if (parseError || lexError) {
        *errLine = lineNumber;
        *errMsg = "Parse error";
        m_sourceElements = 0;
    }
}

void Parser::didFinishParsing(SourceElements* sourceElements, ParserArenaData<DeclarationStacks::VarStack>* varStack, 
                              ParserArenaData<DeclarationStacks::FunctionStack>* funcStack, CodeFeatures features, int lastLine, int numConstants)
{
    m_sourceElements = sourceElements;
    m_varDeclarations = varStack;
    m_funcDeclarations = funcStack;
    m_features = features;
    m_lastLine = lastLine;
    m_numConstants = numConstants;
}

} // namespace AJ
