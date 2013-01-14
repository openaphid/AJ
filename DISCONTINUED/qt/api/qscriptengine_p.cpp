
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
    Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"

#include "qscriptengine_p.h"

#include "qscriptprogram_p.h"
#include "qscriptvalue_p.h"

/*!
    Constructs a default QScriptEnginePrivate object, a new global context will be created.
    \internal
*/
QScriptEnginePrivate::QScriptEnginePrivate(const QScriptEngine* engine)
    : q_ptr(const_cast<QScriptEngine*>(engine))
    , m_context(AJGlobalContextCreate(0))
{
}

QScriptEnginePrivate::~QScriptEnginePrivate()
{
    AJGlobalContextRelease(m_context);
}

QScriptSyntaxCheckResultPrivate* QScriptEnginePrivate::checkSyntax(const QString& program)
{
    AJValueRef exception;
    AJStringRef source = QScriptConverter::toString(program);
    bool syntaxIsCorrect = AJCheckScriptSyntax(m_context, source, /* url */ 0, /* starting line */ 1, &exception);
    AJStringRelease(source);
    if (syntaxIsCorrect) {
        return new QScriptSyntaxCheckResultPrivate(this);
    }
    AJValueProtect(m_context, exception);
    return new QScriptSyntaxCheckResultPrivate(this, const_cast<AJObjectRef>(exception));
}

/*!
    Evaluates program and returns the result of the evaluation.
    \internal
*/
QScriptValuePrivate* QScriptEnginePrivate::evaluate(const QString& program, const QString& fileName, int lineNumber)
{
    AJStringRef script = QScriptConverter::toString(program);
    AJStringRef file = QScriptConverter::toString(fileName);
    QScriptValuePrivate* result = new QScriptValuePrivate(this, evaluate(script, file, lineNumber));
    AJStringRelease(script);
    AJStringRelease(file);
    return result;
}

/*!
    Evaluates program and returns the result of the evaluation.
    \internal
*/
QScriptValuePrivate* QScriptEnginePrivate::evaluate(const QScriptProgramPrivate* program)
{
    if (program->isNull())
        return new QScriptValuePrivate;
    return new QScriptValuePrivate(this, evaluate(program->program(), program->file(), program->line()));
}

QScriptValuePrivate* QScriptEnginePrivate::globalObject() const
{
    AJObjectRef globalObject = AJContextGetGlobalObject(context());
    return new QScriptValuePrivate(this, globalObject, globalObject);
}
