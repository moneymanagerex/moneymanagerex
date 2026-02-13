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

#include "base/defs.h"
#include <optional>
#include <wx/datetime.h>

#include "_primitive.h"

// mmDateDay represents a date without time information.
// wxWidgets does not have a dedicated type for this purpose.
// The underlying data structure is wxDateTime, with the time part set
// to noon (12:00), in order to avoid rounding errors around midnight.
// The underlying value cannot be wxInvalidDateTime.
//
// mmDateDayN is an optional (nullable) mmDateDay.
// The underlying null value is wxInvalidDateTime.
//
// iso{Start,End} are helper functions for date comparisons, e.g.,
//   start.isoStart() <= isoDate <= end.isoEnd()
// where `isoDate` is a date or date+time string in ISO format, checks
// if `isoDate` is in the range (`start`..`end`) inclusive.

struct mmDateDay
{
    friend struct mmDateDayN;

protected:
    wxDateTime dateTime;

public:
    mmDateDay(wxDateTime dateTime_new);

public:
    static const wxTimeSpan htol; // half-day tolerance
    static mmDateDay today();
    static mmDateDay min();
    static mmDateDay max();

public:
    wxDateTime getDateTime() const;
    const wxString isoDate() const;
    const wxString isoStart() const;
    const wxString isoEnd() const;
    void addSpan(wxDateSpan spanDay);

public:
    bool operator== (const mmDateDay& other) const;
    bool operator!= (const mmDateDay& other) const;
    bool operator<  (const mmDateDay& other) const;
    bool operator>  (const mmDateDay& other) const;
    bool operator<= (const mmDateDay& other) const;
    bool operator>= (const mmDateDay& other) const;
};

struct mmDateDayN
{
    friend struct mmDateDay;

private:
    wxDateTime dateTimeN;

public:
    mmDateDayN() = default;
    mmDateDayN(mmDateDay dateDay_new);
    mmDateDayN(wxDateTime dateTimeN_new);
    mmDateDayN(const wxString& isoDateN_new);

public:
    bool has_value() const;
    mmDateDay value() const;
    mmDateDay value_or(mmDateDay defDateDay) const;

public:
    wxDateTime getDateTimeN() const;
    const wxString isoDateN() const;
    const wxString isoStartN() const;
    const wxString isoEndN() const;

public:
    bool operator== (const mmDateDayN& other) const;
    bool operator!= (const mmDateDayN& other) const;
};

inline mmDateDay mmDateDay::today()
{
    // get the dateTime of today, with time set to noon (12:00)
    wxDateTime today_dateTime = wxDateTime(12, 0, 0, 0);
    // the mmDateDay constructor anyway resets the time to noon
    return mmDateDay(today_dateTime);
}
inline mmDateDay mmDateDay::min()
{
    // same date as in DATE_MIN
    wxDateTime min_dateTime;
    min_dateTime.ParseISOCombined("1970-01-01T12:00:00");
    return mmDateDay(min_dateTime);
}
inline mmDateDay mmDateDay::max()
{
    // same date as in DATE_MAX
    // TODO: the datetime is 12 hours larger than DATE_MAX; check for overflow
    wxDateTime max_dateTime;
    max_dateTime.ParseISOCombined("2999-12-31T12:00:00");
    return mmDateDay(max_dateTime);
}

inline wxDateTime mmDateDay::getDateTime() const
{
    return dateTime;
}

inline const wxString mmDateDay::isoDate() const
{
    return dateTime.FormatISODate();
}

// Let `isoDate` be a date or date+time string in ISO format.
// (start.isoStart() <= isoDate) checks if `isoDate` is in the
// same day as `start` or later. Notice that the strict comparison
// (start.isoStart() < isoDate) does not have the intended meaning.
inline const wxString mmDateDay::isoStart() const
{
    wxString dateStr = isoDate();
    return dateStr.append("");
}

// (isoDate <= end.isoEnd()) checks if `isoDate` is in the
// same day as `end` or earlier. Notice that the strict comparison
// (isoDate < end.isoEnd()) does not have the intended meaning.
inline const wxString mmDateDay::isoEnd() const
{
    // note: the ASCII code of "~" is greater than any character in ISO format
    wxString dateStr = isoDate();
    return dateStr.append("~");
}

inline void mmDateDay::addSpan(wxDateSpan spanDay)
{
    // assumption: spanDay has granularity of a day or larger
    dateTime += spanDay;
}

// the dateTime in both operands is set to noon, therefore
// a simple comparison of dateTime is sufficient;
// for more robustness we compare with a tolerance of half day.
inline bool mmDateDay::operator== (const mmDateDay& other) const
{
    return (dateTime < other.dateTime + htol && dateTime + htol >= other.dateTime);
}
inline bool mmDateDay::operator!= (const mmDateDay& other) const
{
    return (dateTime >= other.dateTime + htol || dateTime + htol < other.dateTime);
}
inline bool mmDateDay::operator< (const mmDateDay& other) const
{
    return dateTime + htol < other.dateTime;
}
inline bool mmDateDay::operator> (const mmDateDay& other) const
{
    return dateTime >= other.dateTime + htol;
}
inline bool mmDateDay::operator<= (const mmDateDay& other) const
{
    return (dateTime < other.dateTime + htol);
}
inline bool mmDateDay::operator>= (const mmDateDay& other) const
{
    return (dateTime + htol >= other.dateTime);
}

inline bool mmDateDayN::has_value() const
{
    return dateTimeN.IsValid();
}
inline mmDateDay mmDateDayN::value() const
{
    return mmDateDay(dateTimeN);
}
inline mmDateDay mmDateDayN::value_or(mmDateDay defDateDay) const
{
    return dateTimeN.IsValid() ? mmDateDay(dateTimeN) : defDateDay;
}

inline wxDateTime mmDateDayN::getDateTimeN() const
{
    return dateTimeN;
}
inline const wxString mmDateDayN::isoDateN() const
{
    return has_value() ? value().isoDate() : "";
}
inline const wxString mmDateDayN::isoStartN() const
{
    return has_value() ? value().isoStart() : "";
}
inline const wxString mmDateDayN::isoEndN() const
{
    return has_value() ? value().isoEnd() : "~";
}

inline bool mmDateDayN::operator== (const mmDateDayN& other) const
{
    return (!has_value() && !other.has_value())
        || (has_value() && other.has_value() && value() == other.value());
}
inline bool mmDateDayN::operator!= (const mmDateDayN& other) const
{
    return (has_value() || other.has_value())
        && (!has_value() || !other.has_value() || value() != other.value());
}

