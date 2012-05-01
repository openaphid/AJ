
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
 *  Copyright (C) 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
 *  Copyright (C) 2008, 2009 Torch Mobile, Inc. All rights reserved.
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
#include "DatePrototype.h"

#include "DateConversion.h"
#include "DateInstance.h"
#include "Error.h"
#include "AJString.h"
#include "AJStringBuilder.h"
#include "Lookup.h"
#include "ObjectPrototype.h"

#if !PLATFORM(MAC) && HAVE(LANGINFO_H)
#include <langinfo.h>
#endif

#include <limits.h>
#include <locale.h>
#include <math.h>
#include <time.h>
#include <wtf/Assertions.h>
#include <wtf/DateMath.h>
#include <wtf/MathExtras.h>
#include <wtf/StringExtras.h>
#include <wtf/UnusedParam.h>

#if HAVE(SYS_PARAM_H)
#include <sys/param.h>
#endif

#if HAVE(SYS_TIME_H)
#include <sys/time.h>
#endif

#if HAVE(SYS_TIMEB_H)
#include <sys/timeb.h>
#endif

#include <CoreFoundation/CoreFoundation.h>

#if OS(WINCE) && !PLATFORM(QT)
extern "C" size_t strftime(char * const s, const size_t maxsize, const char * const format, const struct tm * const t); //provided by libce
#endif

using namespace ATF;

namespace AJ {

ASSERT_CLASS_FITS_IN_CELL(DatePrototype);

static AJValue JSC_HOST_CALL dateProtoFuncGetDate(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncGetDay(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncGetFullYear(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncGetHours(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncGetMilliSeconds(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncGetMinutes(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncGetMonth(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncGetSeconds(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncGetTime(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncGetTimezoneOffset(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncGetUTCDate(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncGetUTCDay(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncGetUTCFullYear(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncGetUTCHours(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncGetUTCMilliseconds(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncGetUTCMinutes(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncGetUTCMonth(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncGetUTCSeconds(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncGetYear(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncSetDate(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncSetFullYear(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncSetHours(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncSetMilliSeconds(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncSetMinutes(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncSetMonth(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncSetSeconds(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncSetTime(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncSetUTCDate(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncSetUTCFullYear(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncSetUTCHours(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncSetUTCMilliseconds(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncSetUTCMinutes(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncSetUTCMonth(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncSetUTCSeconds(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncSetYear(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncToDateString(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncToGMTString(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncToLocaleDateString(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncToLocaleString(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncToLocaleTimeString(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncToString(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncToTimeString(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncToUTCString(ExecState*, AJObject*, AJValue, const ArgList&);
static AJValue JSC_HOST_CALL dateProtoFuncToISOString(ExecState*, AJObject*, AJValue, const ArgList&);

static AJValue JSC_HOST_CALL dateProtoFuncToJSON(ExecState*, AJObject*, AJValue, const ArgList&);

}

#include "DatePrototype.lut.h"

namespace AJ {

enum LocaleDateTimeFormat { LocaleDateAndTime, LocaleDate, LocaleTime };
 

// FIXME: Since this is superior to the strftime-based version, why limit this to PLATFORM(MAC)?
// Instead we should consider using this whenever PLATFORM(CF) is true.

static CFDateFormatterStyle styleFromArgString(const UString& string, CFDateFormatterStyle defaultStyle)
{
    if (string == "short")
        return kCFDateFormatterShortStyle;
    if (string == "medium")
        return kCFDateFormatterMediumStyle;
    if (string == "long")
        return kCFDateFormatterLongStyle;
    if (string == "full")
        return kCFDateFormatterFullStyle;
    return defaultStyle;
}

static AJCell* formatLocaleDate(ExecState* exec, DateInstance*, double timeInMilliseconds, LocaleDateTimeFormat format, const ArgList& args)
{
    CFDateFormatterStyle dateStyle = (format != LocaleTime ? kCFDateFormatterLongStyle : kCFDateFormatterNoStyle);
    CFDateFormatterStyle timeStyle = (format != LocaleDate ? kCFDateFormatterLongStyle : kCFDateFormatterNoStyle);

    bool useCustomFormat = false;
    UString customFormatString;

    UString arg0String = args.at(0).toString(exec);
    if (arg0String == "custom" && !args.at(1).isUndefined()) {
        useCustomFormat = true;
        customFormatString = args.at(1).toString(exec);
    } else if (format == LocaleDateAndTime && !args.at(1).isUndefined()) {
        dateStyle = styleFromArgString(arg0String, dateStyle);
        timeStyle = styleFromArgString(args.at(1).toString(exec), timeStyle);
    } else if (format != LocaleTime && !args.at(0).isUndefined())
        dateStyle = styleFromArgString(arg0String, dateStyle);
    else if (format != LocaleDate && !args.at(0).isUndefined())
        timeStyle = styleFromArgString(arg0String, timeStyle);

    CFLocaleRef locale = CFLocaleCopyCurrent();
    CFDateFormatterRef formatter = CFDateFormatterCreate(0, locale, dateStyle, timeStyle);
    CFRelease(locale);

    if (useCustomFormat) {
        CFStringRef customFormatCFString = CFStringCreateWithCharacters(0, customFormatString.data(), customFormatString.size());
        CFDateFormatterSetFormat(formatter, customFormatCFString);
        CFRelease(customFormatCFString);
    }

    CFStringRef string = CFDateFormatterCreateStringWithAbsoluteTime(0, formatter, floor(timeInMilliseconds / msPerSecond) - kCFAbsoluteTimeIntervalSince1970);

    CFRelease(formatter);

    // We truncate the string returned from CFDateFormatter if it's absurdly long (> 200 characters).
    // That's not great error handling, but it just won't happen so it doesn't matter.
    UChar buffer[200];
    const size_t bufferLength = sizeof(buffer) / sizeof(buffer[0]);
    size_t length = CFStringGetLength(string);
    ASSERT(length <= bufferLength);
    if (length > bufferLength)
        length = bufferLength;
    CFStringGetCharacters(string, CFRangeMake(0, length), buffer);

    CFRelease(string);

    return jsNontrivialString(exec, UString(buffer, length));
}


// Converts a list of arguments sent to a Date member function into milliseconds, updating
// ms (representing milliseconds) and t (representing the rest of the date structure) appropriately.
//
// Format of member function: f([hour,] [min,] [sec,] [ms])
static bool fillStructuresUsingTimeArgs(ExecState* exec, const ArgList& args, int maxArgs, double* ms, GregorianDateTime* t)
{
    double milliseconds = 0;
    bool ok = true;
    int idx = 0;
    int numArgs = args.size();
    
    // JS allows extra trailing arguments -- ignore them
    if (numArgs > maxArgs)
        numArgs = maxArgs;

    // hours
    if (maxArgs >= 4 && idx < numArgs) {
        t->hour = 0;
        milliseconds += args.at(idx++).toInt32(exec, ok) * msPerHour;
    }

    // minutes
    if (maxArgs >= 3 && idx < numArgs && ok) {
        t->minute = 0;
        milliseconds += args.at(idx++).toInt32(exec, ok) * msPerMinute;
    }
    
    // seconds
    if (maxArgs >= 2 && idx < numArgs && ok) {
        t->second = 0;
        milliseconds += args.at(idx++).toInt32(exec, ok) * msPerSecond;
    }
    
    if (!ok)
        return false;
        
    // milliseconds
    if (idx < numArgs) {
        double millis = args.at(idx).toNumber(exec);
        ok = isfinite(millis);
        milliseconds += millis;
    } else
        milliseconds += *ms;
    
    *ms = milliseconds;
    return ok;
}

// Converts a list of arguments sent to a Date member function into years, months, and milliseconds, updating
// ms (representing milliseconds) and t (representing the rest of the date structure) appropriately.
//
// Format of member function: f([years,] [months,] [days])
static bool fillStructuresUsingDateArgs(ExecState *exec, const ArgList& args, int maxArgs, double *ms, GregorianDateTime *t)
{
    int idx = 0;
    bool ok = true;
    int numArgs = args.size();
  
    // JS allows extra trailing arguments -- ignore them
    if (numArgs > maxArgs)
        numArgs = maxArgs;
  
    // years
    if (maxArgs >= 3 && idx < numArgs)
        t->year = args.at(idx++).toInt32(exec, ok) - 1900;
    
    // months
    if (maxArgs >= 2 && idx < numArgs && ok)   
        t->month = args.at(idx++).toInt32(exec, ok);
    
    // days
    if (idx < numArgs && ok) {   
        t->monthDay = 0;
        *ms += args.at(idx).toInt32(exec, ok) * msPerDay;
    }
    
    return ok;
}

const ClassInfo DatePrototype::info = {"Date", &DateInstance::info, 0, ExecState::dateTable};

/* Source for DatePrototype.lut.h
@begin dateTable
  toString              dateProtoFuncToString                DontEnum|Function       0
  toISOString           dateProtoFuncToISOString             DontEnum|Function       0
  toUTCString           dateProtoFuncToUTCString             DontEnum|Function       0
  toDateString          dateProtoFuncToDateString            DontEnum|Function       0
  toTimeString          dateProtoFuncToTimeString            DontEnum|Function       0
  toLocaleString        dateProtoFuncToLocaleString          DontEnum|Function       0
  toLocaleDateString    dateProtoFuncToLocaleDateString      DontEnum|Function       0
  toLocaleTimeString    dateProtoFuncToLocaleTimeString      DontEnum|Function       0
  valueOf               dateProtoFuncGetTime                 DontEnum|Function       0
  getTime               dateProtoFuncGetTime                 DontEnum|Function       0
  getFullYear           dateProtoFuncGetFullYear             DontEnum|Function       0
  getUTCFullYear        dateProtoFuncGetUTCFullYear          DontEnum|Function       0
  toGMTString           dateProtoFuncToGMTString             DontEnum|Function       0
  getMonth              dateProtoFuncGetMonth                DontEnum|Function       0
  getUTCMonth           dateProtoFuncGetUTCMonth             DontEnum|Function       0
  getDate               dateProtoFuncGetDate                 DontEnum|Function       0
  getUTCDate            dateProtoFuncGetUTCDate              DontEnum|Function       0
  getDay                dateProtoFuncGetDay                  DontEnum|Function       0
  getUTCDay             dateProtoFuncGetUTCDay               DontEnum|Function       0
  getHours              dateProtoFuncGetHours                DontEnum|Function       0
  getUTCHours           dateProtoFuncGetUTCHours             DontEnum|Function       0
  getMinutes            dateProtoFuncGetMinutes              DontEnum|Function       0
  getUTCMinutes         dateProtoFuncGetUTCMinutes           DontEnum|Function       0
  getSeconds            dateProtoFuncGetSeconds              DontEnum|Function       0
  getUTCSeconds         dateProtoFuncGetUTCSeconds           DontEnum|Function       0
  getMilliseconds       dateProtoFuncGetMilliSeconds         DontEnum|Function       0
  getUTCMilliseconds    dateProtoFuncGetUTCMilliseconds      DontEnum|Function       0
  getTimezoneOffset     dateProtoFuncGetTimezoneOffset       DontEnum|Function       0
  setTime               dateProtoFuncSetTime                 DontEnum|Function       1
  setMilliseconds       dateProtoFuncSetMilliSeconds         DontEnum|Function       1
  setUTCMilliseconds    dateProtoFuncSetUTCMilliseconds      DontEnum|Function       1
  setSeconds            dateProtoFuncSetSeconds              DontEnum|Function       2
  setUTCSeconds         dateProtoFuncSetUTCSeconds           DontEnum|Function       2
  setMinutes            dateProtoFuncSetMinutes              DontEnum|Function       3
  setUTCMinutes         dateProtoFuncSetUTCMinutes           DontEnum|Function       3
  setHours              dateProtoFuncSetHours                DontEnum|Function       4
  setUTCHours           dateProtoFuncSetUTCHours             DontEnum|Function       4
  setDate               dateProtoFuncSetDate                 DontEnum|Function       1
  setUTCDate            dateProtoFuncSetUTCDate              DontEnum|Function       1
  setMonth              dateProtoFuncSetMonth                DontEnum|Function       2
  setUTCMonth           dateProtoFuncSetUTCMonth             DontEnum|Function       2
  setFullYear           dateProtoFuncSetFullYear             DontEnum|Function       3
  setUTCFullYear        dateProtoFuncSetUTCFullYear          DontEnum|Function       3
  setYear               dateProtoFuncSetYear                 DontEnum|Function       1
  getYear               dateProtoFuncGetYear                 DontEnum|Function       0
  toJSON                dateProtoFuncToJSON                  DontEnum|Function       0
@end
*/

// ECMA 15.9.4

DatePrototype::DatePrototype(ExecState* exec, NonNullPassRefPtr<Structure> structure)
    : DateInstance(exec, structure)
{
    // The constructor will be added later, after DateConstructor has been built.
}

bool DatePrototype::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticFunctionSlot<AJObject>(exec, ExecState::dateTable(exec), this, propertyName, slot);
}


bool DatePrototype::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    return getStaticFunctionDescriptor<AJObject>(exec, ExecState::dateTable(exec), this, propertyName, descriptor);
}

// Functions

AJValue JSC_HOST_CALL dateProtoFuncToString(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTime(exec);
    if (!gregorianDateTime)
        return jsNontrivialString(exec, "Invalid Date");
    DateConversionBuffer date;
    DateConversionBuffer time;
    formatDate(*gregorianDateTime, date);
    formatTime(*gregorianDateTime, time);
    return jsMakeNontrivialString(exec, date, " ", time);
}

AJValue JSC_HOST_CALL dateProtoFuncToUTCString(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTimeUTC(exec);
    if (!gregorianDateTime)
        return jsNontrivialString(exec, "Invalid Date");
    DateConversionBuffer date;
    DateConversionBuffer time;
    formatDateUTCVariant(*gregorianDateTime, date);
    formatTimeUTC(*gregorianDateTime, time);
    return jsMakeNontrivialString(exec, date, " ", time);
}

AJValue JSC_HOST_CALL dateProtoFuncToISOString(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);
    
    DateInstance* thisDateObj = asDateInstance(thisValue); 
    
    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTimeUTC(exec);
    if (!gregorianDateTime)
        return jsNontrivialString(exec, "Invalid Date");
    // Maximum amount of space we need in buffer: 6 (max. digits in year) + 2 * 5 (2 characters each for month, day, hour, minute, second) + 4 (. + 3 digits for milliseconds)
    // 6 for formatting and one for null termination = 27.  We add one extra character to allow us to force null termination.
    char buffer[28];
    snprintf(buffer, sizeof(buffer) - 1, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ", 1900 + gregorianDateTime->year, gregorianDateTime->month + 1, gregorianDateTime->monthDay, gregorianDateTime->hour, gregorianDateTime->minute, gregorianDateTime->second, static_cast<int>(fmod(thisDateObj->internalNumber(), 1000)));
    buffer[sizeof(buffer) - 1] = 0;
    return jsNontrivialString(exec, buffer);
}

AJValue JSC_HOST_CALL dateProtoFuncToDateString(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTime(exec);
    if (!gregorianDateTime)
        return jsNontrivialString(exec, "Invalid Date");
    DateConversionBuffer date;
    formatDate(*gregorianDateTime, date);
    return jsNontrivialString(exec, date);
}

AJValue JSC_HOST_CALL dateProtoFuncToTimeString(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTime(exec);
    if (!gregorianDateTime)
        return jsNontrivialString(exec, "Invalid Date");
    DateConversionBuffer time;
    formatTime(*gregorianDateTime, time);
    return jsNontrivialString(exec, time);
}

AJValue JSC_HOST_CALL dateProtoFuncToLocaleString(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 
    return formatLocaleDate(exec, thisDateObj, thisDateObj->internalNumber(), LocaleDateAndTime, args);
}

AJValue JSC_HOST_CALL dateProtoFuncToLocaleDateString(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 
    return formatLocaleDate(exec, thisDateObj, thisDateObj->internalNumber(), LocaleDate, args);
}

AJValue JSC_HOST_CALL dateProtoFuncToLocaleTimeString(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 
    return formatLocaleDate(exec, thisDateObj, thisDateObj->internalNumber(), LocaleTime, args);
}

AJValue JSC_HOST_CALL dateProtoFuncGetTime(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    return asDateInstance(thisValue)->internalValue(); 
}

AJValue JSC_HOST_CALL dateProtoFuncGetFullYear(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTime(exec);
    if (!gregorianDateTime)
        return jsNaN(exec);
    return jsNumber(exec, 1900 + gregorianDateTime->year);
}

AJValue JSC_HOST_CALL dateProtoFuncGetUTCFullYear(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTimeUTC(exec);
    if (!gregorianDateTime)
        return jsNaN(exec);
    return jsNumber(exec, 1900 + gregorianDateTime->year);
}

AJValue JSC_HOST_CALL dateProtoFuncToGMTString(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTimeUTC(exec);
    if (!gregorianDateTime)
        return jsNontrivialString(exec, "Invalid Date");
    DateConversionBuffer date;
    DateConversionBuffer time;
    formatDateUTCVariant(*gregorianDateTime, date);
    formatTimeUTC(*gregorianDateTime, time);
    return jsMakeNontrivialString(exec, date, " ", time);
}

AJValue JSC_HOST_CALL dateProtoFuncGetMonth(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTime(exec);
    if (!gregorianDateTime)
        return jsNaN(exec);
    return jsNumber(exec, gregorianDateTime->month);
}

AJValue JSC_HOST_CALL dateProtoFuncGetUTCMonth(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTimeUTC(exec);
    if (!gregorianDateTime)
        return jsNaN(exec);
    return jsNumber(exec, gregorianDateTime->month);
}

AJValue JSC_HOST_CALL dateProtoFuncGetDate(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTime(exec);
    if (!gregorianDateTime)
        return jsNaN(exec);
    return jsNumber(exec, gregorianDateTime->monthDay);
}

AJValue JSC_HOST_CALL dateProtoFuncGetUTCDate(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTimeUTC(exec);
    if (!gregorianDateTime)
        return jsNaN(exec);
    return jsNumber(exec, gregorianDateTime->monthDay);
}

AJValue JSC_HOST_CALL dateProtoFuncGetDay(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTime(exec);
    if (!gregorianDateTime)
        return jsNaN(exec);
    return jsNumber(exec, gregorianDateTime->weekDay);
}

AJValue JSC_HOST_CALL dateProtoFuncGetUTCDay(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTimeUTC(exec);
    if (!gregorianDateTime)
        return jsNaN(exec);
    return jsNumber(exec, gregorianDateTime->weekDay);
}

AJValue JSC_HOST_CALL dateProtoFuncGetHours(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTime(exec);
    if (!gregorianDateTime)
        return jsNaN(exec);
    return jsNumber(exec, gregorianDateTime->hour);
}

AJValue JSC_HOST_CALL dateProtoFuncGetUTCHours(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTimeUTC(exec);
    if (!gregorianDateTime)
        return jsNaN(exec);
    return jsNumber(exec, gregorianDateTime->hour);
}

AJValue JSC_HOST_CALL dateProtoFuncGetMinutes(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTime(exec);
    if (!gregorianDateTime)
        return jsNaN(exec);
    return jsNumber(exec, gregorianDateTime->minute);
}

AJValue JSC_HOST_CALL dateProtoFuncGetUTCMinutes(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTimeUTC(exec);
    if (!gregorianDateTime)
        return jsNaN(exec);
    return jsNumber(exec, gregorianDateTime->minute);
}

AJValue JSC_HOST_CALL dateProtoFuncGetSeconds(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTime(exec);
    if (!gregorianDateTime)
        return jsNaN(exec);
    return jsNumber(exec, gregorianDateTime->second);
}

AJValue JSC_HOST_CALL dateProtoFuncGetUTCSeconds(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTimeUTC(exec);
    if (!gregorianDateTime)
        return jsNaN(exec);
    return jsNumber(exec, gregorianDateTime->second);
}

AJValue JSC_HOST_CALL dateProtoFuncGetMilliSeconds(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 
    double milli = thisDateObj->internalNumber();
    if (isnan(milli))
        return jsNaN(exec);

    double secs = floor(milli / msPerSecond);
    double ms = milli - secs * msPerSecond;
    return jsNumber(exec, ms);
}

AJValue JSC_HOST_CALL dateProtoFuncGetUTCMilliseconds(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 
    double milli = thisDateObj->internalNumber();
    if (isnan(milli))
        return jsNaN(exec);

    double secs = floor(milli / msPerSecond);
    double ms = milli - secs * msPerSecond;
    return jsNumber(exec, ms);
}

AJValue JSC_HOST_CALL dateProtoFuncGetTimezoneOffset(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTime(exec);
    if (!gregorianDateTime)
        return jsNaN(exec);
    return jsNumber(exec, -gregorianDateTime->utcOffset / minutesPerHour);
}

AJValue JSC_HOST_CALL dateProtoFuncSetTime(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    double milli = timeClip(args.at(0).toNumber(exec));
    AJValue result = jsNumber(exec, milli);
    thisDateObj->setInternalValue(result);
    return result;
}

static AJValue setNewValueFromTimeArgs(ExecState* exec, AJValue thisValue, const ArgList& args, int numArgsToUse, bool inputIsUTC)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue);
    double milli = thisDateObj->internalNumber();
    
    if (args.isEmpty() || isnan(milli)) {
        AJValue result = jsNaN(exec);
        thisDateObj->setInternalValue(result);
        return result;
    }
     
    double secs = floor(milli / msPerSecond);
    double ms = milli - secs * msPerSecond;

    const GregorianDateTime* other = inputIsUTC 
        ? thisDateObj->gregorianDateTimeUTC(exec)
        : thisDateObj->gregorianDateTime(exec);
    if (!other)
        return jsNaN(exec);

    GregorianDateTime gregorianDateTime;
    gregorianDateTime.copyFrom(*other);
    if (!fillStructuresUsingTimeArgs(exec, args, numArgsToUse, &ms, &gregorianDateTime)) {
        AJValue result = jsNaN(exec);
        thisDateObj->setInternalValue(result);
        return result;
    } 
    
    AJValue result = jsNumber(exec, gregorianDateTimeToMS(exec, gregorianDateTime, ms, inputIsUTC));
    thisDateObj->setInternalValue(result);
    return result;
}

static AJValue setNewValueFromDateArgs(ExecState* exec, AJValue thisValue, const ArgList& args, int numArgsToUse, bool inputIsUTC)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue);
    if (args.isEmpty()) {
        AJValue result = jsNaN(exec);
        thisDateObj->setInternalValue(result);
        return result;
    }      
    
    double milli = thisDateObj->internalNumber();
    double ms = 0; 

    GregorianDateTime gregorianDateTime; 
    if (numArgsToUse == 3 && isnan(milli)) 
        msToGregorianDateTime(exec, 0, true, gregorianDateTime); 
    else { 
        ms = milli - floor(milli / msPerSecond) * msPerSecond; 
        const GregorianDateTime* other = inputIsUTC 
            ? thisDateObj->gregorianDateTimeUTC(exec)
            : thisDateObj->gregorianDateTime(exec);
        if (!other)
            return jsNaN(exec);
        gregorianDateTime.copyFrom(*other);
    }
    
    if (!fillStructuresUsingDateArgs(exec, args, numArgsToUse, &ms, &gregorianDateTime)) {
        AJValue result = jsNaN(exec);
        thisDateObj->setInternalValue(result);
        return result;
    } 
           
    AJValue result = jsNumber(exec, gregorianDateTimeToMS(exec, gregorianDateTime, ms, inputIsUTC));
    thisDateObj->setInternalValue(result);
    return result;
}

AJValue JSC_HOST_CALL dateProtoFuncSetMilliSeconds(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    const bool inputIsUTC = false;
    return setNewValueFromTimeArgs(exec, thisValue, args, 1, inputIsUTC);
}

AJValue JSC_HOST_CALL dateProtoFuncSetUTCMilliseconds(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    const bool inputIsUTC = true;
    return setNewValueFromTimeArgs(exec, thisValue, args, 1, inputIsUTC);
}

AJValue JSC_HOST_CALL dateProtoFuncSetSeconds(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    const bool inputIsUTC = false;
    return setNewValueFromTimeArgs(exec, thisValue, args, 2, inputIsUTC);
}

AJValue JSC_HOST_CALL dateProtoFuncSetUTCSeconds(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    const bool inputIsUTC = true;
    return setNewValueFromTimeArgs(exec, thisValue, args, 2, inputIsUTC);
}

AJValue JSC_HOST_CALL dateProtoFuncSetMinutes(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    const bool inputIsUTC = false;
    return setNewValueFromTimeArgs(exec, thisValue, args, 3, inputIsUTC);
}

AJValue JSC_HOST_CALL dateProtoFuncSetUTCMinutes(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    const bool inputIsUTC = true;
    return setNewValueFromTimeArgs(exec, thisValue, args, 3, inputIsUTC);
}

AJValue JSC_HOST_CALL dateProtoFuncSetHours(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    const bool inputIsUTC = false;
    return setNewValueFromTimeArgs(exec, thisValue, args, 4, inputIsUTC);
}

AJValue JSC_HOST_CALL dateProtoFuncSetUTCHours(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    const bool inputIsUTC = true;
    return setNewValueFromTimeArgs(exec, thisValue, args, 4, inputIsUTC);
}

AJValue JSC_HOST_CALL dateProtoFuncSetDate(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    const bool inputIsUTC = false;
    return setNewValueFromDateArgs(exec, thisValue, args, 1, inputIsUTC);
}

AJValue JSC_HOST_CALL dateProtoFuncSetUTCDate(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    const bool inputIsUTC = true;
    return setNewValueFromDateArgs(exec, thisValue, args, 1, inputIsUTC);
}

AJValue JSC_HOST_CALL dateProtoFuncSetMonth(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    const bool inputIsUTC = false;
    return setNewValueFromDateArgs(exec, thisValue, args, 2, inputIsUTC);
}

AJValue JSC_HOST_CALL dateProtoFuncSetUTCMonth(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    const bool inputIsUTC = true;
    return setNewValueFromDateArgs(exec, thisValue, args, 2, inputIsUTC);
}

AJValue JSC_HOST_CALL dateProtoFuncSetFullYear(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    const bool inputIsUTC = false;
    return setNewValueFromDateArgs(exec, thisValue, args, 3, inputIsUTC);
}

AJValue JSC_HOST_CALL dateProtoFuncSetUTCFullYear(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    const bool inputIsUTC = true;
    return setNewValueFromDateArgs(exec, thisValue, args, 3, inputIsUTC);
}

AJValue JSC_HOST_CALL dateProtoFuncSetYear(ExecState* exec, AJObject*, AJValue thisValue, const ArgList& args)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue);     
    if (args.isEmpty()) { 
        AJValue result = jsNaN(exec);
        thisDateObj->setInternalValue(result);
        return result;
    }
    
    double milli = thisDateObj->internalNumber();
    double ms = 0;

    GregorianDateTime gregorianDateTime;
    if (isnan(milli))
        // Based on ECMA 262 B.2.5 (setYear)
        // the time must be reset to +0 if it is NaN. 
        msToGregorianDateTime(exec, 0, true, gregorianDateTime);
    else {   
        double secs = floor(milli / msPerSecond);
        ms = milli - secs * msPerSecond;
        if (const GregorianDateTime* other = thisDateObj->gregorianDateTime(exec))
            gregorianDateTime.copyFrom(*other);
    }
    
    bool ok = true;
    int32_t year = args.at(0).toInt32(exec, ok);
    if (!ok) {
        AJValue result = jsNaN(exec);
        thisDateObj->setInternalValue(result);
        return result;
    }
            
    gregorianDateTime.year = (year > 99 || year < 0) ? year - 1900 : year;
    AJValue result = jsNumber(exec, gregorianDateTimeToMS(exec, gregorianDateTime, ms, false));
    thisDateObj->setInternalValue(result);
    return result;
}

AJValue JSC_HOST_CALL dateProtoFuncGetYear(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    if (!thisValue.inherits(&DateInstance::info))
        return throwError(exec, TypeError);

    DateInstance* thisDateObj = asDateInstance(thisValue); 

    const GregorianDateTime* gregorianDateTime = thisDateObj->gregorianDateTime(exec);
    if (!gregorianDateTime)
        return jsNaN(exec);

    // NOTE: IE returns the full year even in getYear.
    return jsNumber(exec, gregorianDateTime->year);
}

AJValue JSC_HOST_CALL dateProtoFuncToJSON(ExecState* exec, AJObject*, AJValue thisValue, const ArgList&)
{
    AJObject* object = thisValue.toThisObject(exec);
    if (exec->hadException())
        return jsNull();
    
    AJValue toISOValue = object->get(exec, exec->globalData().propertyNames->toISOString);
    if (exec->hadException())
        return jsNull();

    CallData callData;
    CallType callType = toISOValue.getCallData(callData);
    if (callType == CallTypeNone)
        return throwError(exec, TypeError, "toISOString is not a function");

    AJValue result = call(exec, asObject(toISOValue), callType, callData, object, exec->emptyList());
    if (exec->hadException())
        return jsNull();
    if (result.isObject())
        return throwError(exec, TypeError, "toISOString did not return a primitive value");
    return result;
}

} // namespace AJ
