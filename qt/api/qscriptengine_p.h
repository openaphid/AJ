
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

#ifndef qscriptengine_p_h
#define qscriptengine_p_h

#include "qscriptconverter_p.h"
#include "qscriptengine.h"
#include "qscriptstring_p.h"
#include "qscriptsyntaxcheckresult_p.h"
#include "qscriptvalue.h"
#include <AJCore/AJ.h>
#include <AJBasePrivate.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qstring.h>

class QScriptEngine;
class QScriptSyntaxCheckResultPrivate;

class QScriptEnginePrivate : public QSharedData {
public:
    static QScriptEnginePrivate* get(const QScriptEngine* q) { Q_ASSERT(q); return q->d_ptr.data(); }
    static QScriptEngine* get(const QScriptEnginePrivate* d) { Q_ASSERT(d); return d->q_ptr; }

    QScriptEnginePrivate(const QScriptEngine*);
    ~QScriptEnginePrivate();

    QScriptSyntaxCheckResultPrivate* checkSyntax(const QString& program);
    QScriptValuePrivate* evaluate(const QString& program, const QString& fileName, int lineNumber);
    QScriptValuePrivate* evaluate(const QScriptProgramPrivate* program);
    inline AJValueRef evaluate(AJStringRef program, AJStringRef fileName, int lineNumber);

    inline void collectGarbage();
    inline void reportAdditionalMemoryCost(int cost);

    inline AJValueRef makeAJValue(double number) const;
    inline AJValueRef makeAJValue(int number) const;
    inline AJValueRef makeAJValue(uint number) const;
    inline AJValueRef makeAJValue(const QString& string) const;
    inline AJValueRef makeAJValue(bool number) const;
    inline AJValueRef makeAJValue(QScriptValue::SpecialValue value) const;

    QScriptValuePrivate* globalObject() const;

    inline QScriptStringPrivate* toStringHandle(const QString& str) const;

    inline AJGlobalContextRef context() const;
private:
    QScriptEngine* q_ptr;
    AJGlobalContextRef m_context;
};


/*!
  Evaluates given AJ program and returns result of the evaluation.
  \attention this function doesn't take ownership of the parameters.
  \internal
*/
AJValueRef QScriptEnginePrivate::evaluate(AJStringRef program, AJStringRef fileName, int lineNumber)
{
    AJValueRef exception;
    AJValueRef result = AJEvaluateScript(m_context, program, /* Global Object */ 0, fileName, lineNumber, &exception);
    if (!result)
        return exception; // returns an exception
    return result;
}

void QScriptEnginePrivate::collectGarbage()
{
    AJGarbageCollect(m_context);
}

void QScriptEnginePrivate::reportAdditionalMemoryCost(int cost)
{
    if (cost > 0)
        JSReportExtraMemoryCost(m_context, cost);
}

AJValueRef QScriptEnginePrivate::makeAJValue(double number) const
{
    return AJValueMakeNumber(m_context, number);
}

AJValueRef QScriptEnginePrivate::makeAJValue(int number) const
{
    return AJValueMakeNumber(m_context, number);
}

AJValueRef QScriptEnginePrivate::makeAJValue(uint number) const
{
    return AJValueMakeNumber(m_context, number);
}

AJValueRef QScriptEnginePrivate::makeAJValue(const QString& string) const
{
    AJStringRef tmp = QScriptConverter::toString(string);
    AJValueRef result = AJValueMakeString(m_context, tmp);
    AJStringRelease(tmp);
    return result;
}

AJValueRef QScriptEnginePrivate::makeAJValue(bool value) const
{
    return AJValueMakeBoolean(m_context, value);
}

AJValueRef QScriptEnginePrivate::makeAJValue(QScriptValue::SpecialValue value) const
{
    if (value == QScriptValue::NullValue)
        return AJValueMakeNull(m_context);
    return AJValueMakeUndefined(m_context);
}

QScriptStringPrivate* QScriptEnginePrivate::toStringHandle(const QString& str) const
{
    return new QScriptStringPrivate(str);
}

AJGlobalContextRef QScriptEnginePrivate::context() const
{
    return m_context;
}

#endif
