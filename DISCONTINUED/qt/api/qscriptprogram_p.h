
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

#ifndef qscriptprogram_p_h
#define qscriptprogram_p_h

#include "qscriptconverter_p.h"
#include "qscriptprogram.h"
#include <AJCore/AJ.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qstring.h>

/*
   FIXME The QScriptProgramPrivate potentially could be much faster. In current implementation we
   gain CPU time only by avoiding QString -> AJStringRef conversion. In the ideal world we should
   have a function inside the JSC C API that could provide us "parse once, execute multiple times"
   functionality.
*/

class QScriptProgramPrivate : public QSharedData {
public:
    inline static QScriptProgramPrivate* get(const QScriptProgram& program);
    inline QScriptProgramPrivate();
    inline QScriptProgramPrivate(const QString& sourceCode,
                   const QString fileName,
                   int firstLineNumber);

    inline ~QScriptProgramPrivate();

    inline bool isNull() const;

    inline QString sourceCode() const;
    inline QString fileName() const;
    inline int firstLineNumber() const;

    inline bool operator==(const QScriptProgramPrivate& other) const;
    inline bool operator!=(const QScriptProgramPrivate& other) const;

    inline AJStringRef program() const;
    inline AJStringRef file() const;
    inline int line() const;
private:
    AJStringRef m_program;
    AJStringRef m_fileName;
    int m_line;
};

QScriptProgramPrivate* QScriptProgramPrivate::get(const QScriptProgram& program)
{
    return const_cast<QScriptProgramPrivate*>(program.d_ptr.constData());
}

QScriptProgramPrivate::QScriptProgramPrivate()
    : m_program(0)
    , m_fileName(0)
    , m_line(-1)
{}

QScriptProgramPrivate::QScriptProgramPrivate(const QString& sourceCode,
               const QString fileName,
               int firstLineNumber)
                   : m_program(QScriptConverter::toString(sourceCode))
                   , m_fileName(QScriptConverter::toString(fileName))
                   , m_line(firstLineNumber)
{}

QScriptProgramPrivate::~QScriptProgramPrivate()
{
    if (!isNull()) {
        AJStringRelease(m_program);
        AJStringRelease(m_fileName);
    }
}

bool QScriptProgramPrivate::isNull() const
{
    return !m_program;
}

QString QScriptProgramPrivate::sourceCode() const
{
    return QScriptConverter::toString(m_program);
}

QString QScriptProgramPrivate::fileName() const
{
    return QScriptConverter::toString(m_fileName);
}

int QScriptProgramPrivate::firstLineNumber() const
{
    return m_line;
}

bool QScriptProgramPrivate::operator==(const QScriptProgramPrivate& other) const
{
    return m_line == other.m_line
            && AJStringIsEqual(m_fileName, other.m_fileName)
            && AJStringIsEqual(m_program, other.m_program);
}

bool QScriptProgramPrivate::operator!=(const QScriptProgramPrivate& other) const
{
    return m_line != other.m_line
            || !AJStringIsEqual(m_fileName, other.m_fileName)
            || !AJStringIsEqual(m_program, other.m_program);
}

AJStringRef QScriptProgramPrivate::program() const { return m_program; }
AJStringRef QScriptProgramPrivate::file() const {return m_fileName; }
int QScriptProgramPrivate::line() const { return m_line; }

#endif // qscriptprogram_p_h
