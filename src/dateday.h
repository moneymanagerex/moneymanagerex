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
#include "primitive.h"

// DateDay represents a date without time information.
// wxWidgets does not have a dedicated type for this purpose.
// The underlying data structure is wxDateTime, with the time part set
// to noon (12:00), in order to avoid rounding errors around midnight.
// The special value wxInvalidDateTime means "undefined" (depending on
// the context, it may mean, e.g., "missing", or "null", or "invalid").
//
// dateISO{Start,End} are helper functions for date comparisons, e.g.,
//   start.dateStartISO() <= date_str <= end.dateEndISO()
// where `date_str` is a date or date+time string in ISO format, checks
// if `date_str` is in the range (`start`..`end`), where each boundary
// is inclusive if it is defined and open if it is undefined.

struct DateDay
{
protected:
    wxDateTime dateTime;

public:
    DateDay();
    DateDay(wxDateTime dateTime_new);

public:
    static DateDay today();

public:
    bool isDefined() const;
    wxDateTime getDateTime(wxDateTime defaultDateTime = wxDateTime()) const;
    const wxString dateISO() const;
    const wxString dateStartISO() const;
    const wxString dateEndISO() const;
    void addSpan(wxDateSpan span);

    // partial operators (the undefined value is not comparable)
    bool operator== (const DateDay& other) const;
    bool operator!= (const DateDay& other) const;
    bool operator<  (const DateDay& other) const;
    bool operator>  (const DateDay& other) const;
    bool operator<= (const DateDay& other) const;
    bool operator>= (const DateDay& other) const;
};

inline DateDay DateDay::today()
{
    // get the dateTime of today, with time set to noon (12:00)
    wxDateTime today_dateTime = wxDateTime(12, 0, 0, 0);
    // the DateDay constructor anyway resets the time to noon
    return DateDay(today_dateTime);
}

// isDefined is better name than isValid.
// depending on the context, wxInvalidDateTime may be a valid value.
inline bool DateDay::isDefined() const
{
    return dateTime.IsValid();
}

inline wxDateTime DateDay::getDateTime(wxDateTime defaultDateTime) const
{
    return dateTime.IsValid() ? dateTime : defaultDateTime;
}

inline const wxString DateDay::dateISO() const
{
    // an undefined date is represented with an empty string
    return dateTime.IsValid() ? dateTime.FormatISODate() : "";
}

// Let `date_str` be a date or date+time string in ISO format.
// (start.dateStartISO() <= date_str) is true iff:
//   `start` is defined and the date part in `date_str` is greater than or equal to `start`, or
//   `start` is undefined
// Notice that (start.dateStartISO() < date_str) does not have the intended meaning.
inline const wxString DateDay::dateStartISO() const
{
    wxString dateStr = dateISO();
    return dateStr.append("");
}

// (date_str <= end.dateEndISO()) is true iff:
//   `end` is defined and the date part in `date_str` is less than or equal to `end`, or
//   `end` is undefined
// Notice that (date_str < end.dateEndISO()) does not have the intended meaning.
inline const wxString DateDay::dateEndISO() const
{
    // note: the ASCII code of "~" is greater than any character in ISO format
    wxString dateStr = dateISO();
    return dateStr.append("~");
}

inline void DateDay::addSpan(wxDateSpan span)
{
    if (isDefined())
        dateTime += span;
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
    return (isDefined() && other.isDefined() && dateTime < other.dateTime);
}

inline bool DateDay::operator> (const DateDay& other) const
{
    return (isDefined() && other.isDefined() && dateTime > other.dateTime);
}

inline bool DateDay::operator<= (const DateDay& other) const
{
    return (*this == other) || (*this < other);
}

inline bool DateDay::operator>= (const DateDay& other) const
{
    return (*this == other) || (*this > other);
}

