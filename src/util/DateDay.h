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

#pragma once

#include "defs.h"
#include <optional>
#include <wx/datetime.h>

#include "primitive.h"

// DateDay represents a date without time information.
// wxWidgets does not have a dedicated type for this purpose.
// The underlying data structure is wxDateTime, with the time part set
// to noon (12:00), in order to avoid rounding errors around midnight.
// The underlying value cannot be wxInvalidDateTime.
//
// DateDayN is an optional (nullable) DateDay.
// The underlying null value is wxInvalidDateTime.
//
// iso{Start,End} are helper functions for date comparisons, e.g.,
//   start.isoStart() <= isoDate <= end.isoEnd()
// where `isoDate` is a date or date+time string in ISO format, checks
// if `isoDate` is in the range (`start`..`end`) inclusive.

struct DateDay
{
    friend struct DateDayN;

protected:
    wxDateTime dateTime;

public:
    DateDay(wxDateTime dateTime_new);

public:
    static const wxTimeSpan htol; // half-day tolerance
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
};

struct DateDayN
{
    friend struct DateDay;

private:
    wxDateTime dateTimeN;

public:
    DateDayN() = default;
    DateDayN(DateDay dateDay_new);
    DateDayN(wxDateTime dateTimeN_new);
    DateDayN(const wxString& isoDateN_new);

public:
    bool has_value() const;
    DateDay value() const;
    DateDay value_or(DateDay defDateDay) const;

public:
    wxDateTime getDateTimeN() const;
    const wxString isoDateN() const;
    const wxString isoStartN() const;
    const wxString isoEndN() const;

public:
    bool operator== (const DateDayN& other) const;
    bool operator!= (const DateDayN& other) const;
};

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
    // TODO: the datetime is 12 hours larger than DATE_MAX; check for overflow
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

// the dateTime in both operands is set to noon, therefore
// a simple comparison of dateTime is sufficient;
// for more robustness we compare with a tolerance of half day.
inline bool DateDay::operator== (const DateDay& other) const
{
    return (dateTime < other.dateTime + htol && dateTime + htol >= other.dateTime);
}
inline bool DateDay::operator!= (const DateDay& other) const
{
    return (dateTime >= other.dateTime + htol || dateTime + htol < other.dateTime);
}
inline bool DateDay::operator< (const DateDay& other) const
{
    return dateTime + htol < other.dateTime;
}
inline bool DateDay::operator> (const DateDay& other) const
{
    return dateTime >= other.dateTime + htol;
}
inline bool DateDay::operator<= (const DateDay& other) const
{
    return (dateTime < other.dateTime + htol);
}
inline bool DateDay::operator>= (const DateDay& other) const
{
    return (dateTime + htol >= other.dateTime);
}

inline bool DateDayN::has_value() const
{
    return dateTimeN.IsValid();
}
inline DateDay DateDayN::value() const
{
    return DateDay(dateTimeN);
}
inline DateDay DateDayN::value_or(DateDay defDateDay) const
{
    return dateTimeN.IsValid() ? DateDay(dateTimeN) : defDateDay;
}

inline wxDateTime DateDayN::getDateTimeN() const
{
    return dateTimeN;
}
inline const wxString DateDayN::isoDateN() const
{
    return has_value() ? value().isoDate() : "";
}
inline const wxString DateDayN::isoStartN() const
{
    return has_value() ? value().isoStart() : "";
}
inline const wxString DateDayN::isoEndN() const
{
    return has_value() ? value().isoEnd() : "~";
}

inline bool DateDayN::operator== (const DateDayN& other) const
{
    return (!has_value() && !other.has_value())
        || (has_value() && other.has_value() && value() == other.value());
}
inline bool DateDayN::operator!= (const DateDayN& other) const
{
    return (has_value() || other.has_value())
        && (!has_value() || !other.has_value() || value() != other.value());
}

