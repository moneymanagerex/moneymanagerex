/*******************************************************
 Copyright (C) 2026 George Ef (george.a.ef@gmail.com)

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ********************************************************/

# pragma once

#include <wx/datetime.h>
#include <optional>
#include "primitive.h"

// DateDay represents a date without time information.
// wxWidgets does not have a dedicated type for this purpose.
// The underlying data structure is wxDateTime, with the time part set
// to noon (12:00), in order to avoid rounding errors around midnight.
// 
// DateDayN is an optional (nullable) DateDay.
// Notice that wxInvalidDateTime is not used. DateDay is always defined
// and valid, while the null value in DateDayN is std::nullopt.
//
// iso{Start,End} are helper functions for date comparisons, e.g.,
//   start.isoStart() <= isoDate <= end.isoEnd()
// where `isoDate` is a date or date+time string in ISO format, checks
// if `isoDate` is in the range (`start`..`end`) inclusive.

struct DateDay
{
protected:
    wxDateTime dateTime;

public:
    DateDay(wxDateTime dateTime_new);

public:
    static DateDay today();
    static DateDay min();
    static DateDay max();

public:
    wxDateTime getDateTime() const;
    const wxString isoDate() const;
    const wxString isoStart() const;
    const wxString isoEnd() const;
    void addSpan(wxDateSpan spanDay);

public:
    bool operator== (const DateDay& other) const;
    bool operator!= (const DateDay& other) const;
    bool operator<  (const DateDay& other) const;
    bool operator>  (const DateDay& other) const;
    bool operator<= (const DateDay& other) const;
    bool operator>= (const DateDay& other) const;

    using DateDayN = std::optional<DateDay>;
public:
    // define static methods for DateDayN
    static DateDayN dateDayN(wxDateTime dateTime_new);
    static DateDayN dateDayN(const wxString& dateStr_new);
    static wxDateTime dateTimeN(DateDayN dateN);
    static const wxString isoDateN(DateDayN dateN);
    static const wxString isoStartN(DateDayN dateN);
    static const wxString isoEndN(DateDayN dateN);
};

using DateDayN = std::optional<DateDay>;

inline DateDay DateDay::today()
{
    // get the dateTime of today, with time set to noon (12:00)
    wxDateTime today_dateTime = wxDateTime(12, 0, 0, 0);
    // the DateDay constructor anyway resets the time to noon
    return DateDay(today_dateTime);
}
inline DateDay DateDay::min()
{
    // same date as in DATE_MIN
    wxDateTime min_dateTime;
    min_dateTime.ParseISOCombined("1970-01-01T12:00:00");
    return DateDay(min_dateTime);
}
inline DateDay DateDay::max()
{
    // same date as in DATE_MAX
    wxDateTime max_dateTime;
    max_dateTime.ParseISOCombined("2999-12-31T12:00:00");
    return DateDay(max_dateTime);
}

inline wxDateTime DateDay::getDateTime() const
{
    return dateTime;
}

inline const wxString DateDay::isoDate() const
{
    return dateTime.FormatISODate();
}

// Let `isoDate` be a date or date+time string in ISO format.
// (start.isoStart() <= isoDate) checks if `isoDate` is in the
// same day as `start` or later. Notice that the strict comparison
// (start.isoStart() < isoDate) does not have the intended meaning.
inline const wxString DateDay::isoStart() const
{
    wxString dateStr = isoDate();
    return dateStr.append("");
}

// (isoDate <= end.isoEnd()) checks if `isoDate` is in the
// same day as `end` or earlier. Notice that the strict comparison
// (isoDate < end.isoEnd()) does not have the intended meaning.
inline const wxString DateDay::isoEnd() const
{
    // note: the ASCII code of "~" is greater than any character in ISO format
    wxString dateStr = isoDate();
    return dateStr.append("~");
}

inline void DateDay::addSpan(wxDateSpan spanDay)
{
    // assumption: spanDay has granularity of a day or larger
    dateTime += spanDay;
}

inline bool DateDay::operator== (const DateDay& other) const
{
    return (dateTime == other.dateTime);
}
inline bool DateDay::operator!= (const DateDay& other) const
{
    return (dateTime != other.dateTime);
}
inline bool DateDay::operator< (const DateDay& other) const
{
    return dateTime < other.dateTime;
}
inline bool DateDay::operator> (const DateDay& other) const
{
    return dateTime > other.dateTime;
}
inline bool DateDay::operator<= (const DateDay& other) const
{
    return (dateTime <= other.dateTime);
}
inline bool DateDay::operator>= (const DateDay& other) const
{
    return (dateTime >= other.dateTime);
}

inline DateDayN DateDay::dateDayN(wxDateTime dateTime_new)
{
    return dateTime_new.IsValid() ? DateDay(dateTime_new) : DateDayN();
}
inline DateDayN DateDay::dateDayN(const wxString& dateStr_new)
{
    return dateDayN(isoDateTime(dateStr_new));
}
inline wxDateTime DateDay::dateTimeN(DateDayN dateN)
{
    dateN.has_value() ? dateN.value().getDateTime() : wxInvalidDateTime;
}
inline const wxString DateDay::isoDateN(DateDayN dateN)
{
    return dateN.has_value() ? dateN.value().isoDate() : "";
}
inline const wxString DateDay::isoStartN(DateDayN dateN)
{
    return dateN.has_value() ? dateN.value().isoStart() : "";
}
inline const wxString DateDay::isoEndN(DateDayN dateN)
{
    return dateN.has_value() ? dateN.value().isoEnd() : "~";
}

