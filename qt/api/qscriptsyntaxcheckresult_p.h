
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
    Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)

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

#ifndef qscriptsyntaxcheckresult_p_h
#define qscriptsyntaxcheckresult_p_h

#include "qscriptconverter_p.h"
#include "qscriptengine_p.h"
#include "qscriptsyntaxcheckresult.h"
#include <AJCore/AJ.h>
#include <QtCore/qshareddata.h>

class QScriptSyntaxCheckResultPrivate : public QSharedData {
public:
    static inline QScriptSyntaxCheckResult get(QScriptSyntaxCheckResultPrivate* p);
    inline QScriptSyntaxCheckResultPrivate(const QScriptEnginePrivate* engine);
    inline QScriptSyntaxCheckResultPrivate(const QScriptEnginePrivate* engine, AJObjectRef value);
    ~QScriptSyntaxCheckResultPrivate();

    inline QScriptSyntaxCheckResult::State state() const;
    int errorLineNumber() const;
    inline int errorColumnNumber() const;
    QString errorMessage() const;
private:
    AJObjectRef m_exception;
    QScriptEnginePtr m_engine;
};

QScriptSyntaxCheckResult QScriptSyntaxCheckResultPrivate::get(QScriptSyntaxCheckResultPrivate* p)
{
    return QScriptSyntaxCheckResult(p);
}

QScriptSyntaxCheckResultPrivate::QScriptSyntaxCheckResultPrivate(const QScriptEnginePrivate* engine)
    : m_exception(0)
    , m_engine(const_cast<QScriptEnginePrivate*>(engine))
{}

QScriptSyntaxCheckResultPrivate::QScriptSyntaxCheckResultPrivate(const QScriptEnginePrivate* engine, AJObjectRef value)
    : m_exception(value)
    , m_engine(const_cast<QScriptEnginePrivate*>(engine))
{}

QScriptSyntaxCheckResult::State QScriptSyntaxCheckResultPrivate::state() const
{
    // FIXME This function doesn't return QScriptSyntaxCheckResult::Intermediate
    return m_exception ? QScriptSyntaxCheckResult::Error : QScriptSyntaxCheckResult::Valid;
}

int QScriptSyntaxCheckResultPrivate::errorColumnNumber() const
{
    // FIXME JSC C API doesn't expose the error column number.
    return m_exception ? 1 : -1;
}


#endif // qscriptsyntaxcheckresult_p_h
