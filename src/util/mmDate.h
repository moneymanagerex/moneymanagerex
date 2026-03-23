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
#include "mmDateTime.h"

// mmDate represents a date without time information.
// wxWidgets does not have a dedicated type for this purpose.
// The underlying data structure is wxDateTime, with the time part set
// to noon (12:00), in order to avoid rounding errors around midnight.
// The underlying value cannot be wxInvalidDateTime.
//
// mmDateN is an optional (nullable) mmDate.
// The underlying null value is wxInvalidDateTime.
//
// iso{Start,End} are helper functions for date comparisons, e.g.,
//   start.isoStart() <= isoDate <= end.isoEnd()
// where `isoDate` is a date or date+time string in ISO format, checks
// if `isoDate` is in the range (`start`..`end`) inclusive.

struct mmDate
{
    friend struct mmDateN;

protected:
    wxDateTime m_dateTime;

public:
    mmDate(wxDateTime dateTime);
    mmDate(mmDateTime dateTime);
    mmDate(const wxString& isoDateTime);

public:
    static const wxTimeSpan htol; // half-day tolerance
    static mmDate today();
    static mmDate min();
    static mmDate max();

public:
    auto getDateTime() const -> wxDateTime;
    auto isoDate() const -> const wxString;
    auto isoStart() const -> const wxString;
    auto isoEnd() const -> const wxString;
    void addDateSpan(wxDateSpan dateSpan);
    auto plusDateSpan(wxDateSpan dateSpan) -> mmDate;
    auto minusDateSpan(wxDateSpan dateSpan) -> mmDate;
    int  daysSince(const mmDate& other) const;
    int  daysUntil(const mmDate& other) const;

public:
    bool operator== (const mmDate& other) const;
    bool operator!= (const mmDate& other) const;
    bool operator<  (const mmDate& other) const;
    bool operator>  (const mmDate& other) const;
    bool operator<= (const mmDate& other) const;
    bool operator>= (const mmDate& other) const;
};

struct mmDateN
{
    friend struct mmDate;

private:
    wxDateTime m_dateTimeN;

public:
    mmDateN() = default;
    mmDateN(mmDate dateDay);
    mmDateN(wxDateTime dateTimeN);
    mmDateN(mmDateTimeN dateTimeN);
    mmDateN(const wxString& isoDateTimeN);

public:
    bool has_value() const;
    auto value() const -> mmDate;
    auto value_or(mmDate defDateDay) const -> mmDate;

public:
    auto getDateTimeN() const -> wxDateTime;
    auto isoDateN() const -> const wxString;
    auto isoStartN() const -> const wxString;
    auto isoEndN() const -> const wxString;

public:
    bool operator== (const mmDateN& other) const;
    bool operator!= (const mmDateN& other) const;
};

inline mmDate mmDate::today()
{
    // get the dateTime of today, with time set to noon (12:00)
    wxDateTime today_dateTime = wxDateTime(12, 0, 0, 0);
    // the mmDate constructor anyway resets the time to noon
    return mmDate(today_dateTime);
}
inline mmDate mmDate::min()
{
    // same date as in DATE_MIN
    wxDateTime min_dateTime;
    min_dateTime.ParseISOCombined("1970-01-01T12:00:00");
    return mmDate(min_dateTime);
}
inline mmDate mmDate::max()
{
    // same date as in DATE_MAX
    // TODO: the datetime is 12 hours larger than DATE_MAX; check for overflow
    wxDateTime max_dateTime;
    max_dateTime.ParseISOCombined("2999-12-31T12:00:00");
    return mmDate(max_dateTime);
}

inline wxDateTime mmDate::getDateTime() const
{
    return m_dateTime;
}

inline const wxString mmDate::isoDate() const
{
    return m_dateTime.FormatISODate();
}

// Let `isoDate` be a date or date+time string in ISO format.
// (start.isoStart() <= isoDate) checks if `isoDate` is in the
// same day as `start` or later. Notice that the strict comparison
// (start.isoStart() < isoDate) does not have the intended meaning.
inline const wxString mmDate::isoStart() const
{
    wxString dateStr = isoDate();
    return dateStr.append("");
}

// (isoDate <= end.isoEnd()) checks if `isoDate` is in the
// same day as `end` or earlier. Notice that the strict comparison
// (isoDate < end.isoEnd()) does not have the intended meaning.
inline const wxString mmDate::isoEnd() const
{
    // note: the ASCII code of "~" is greater than any character in ISO format
    wxString dateStr = isoDate();
    return dateStr.append("~");
}

inline void mmDate::addDateSpan(wxDateSpan dateSpan)
{
    // assumption: dateSpan has granularity of a day or larger
    m_dateTime += dateSpan;
}
inline mmDate mmDate::plusDateSpan(wxDateSpan dateSpan)
{
    return mmDate(m_dateTime + dateSpan);
}
inline mmDate mmDate::minusDateSpan(wxDateSpan dateSpan)
{
    return mmDate(m_dateTime - dateSpan);
}

inline int mmDate::daysSince(const mmDate& other) const
{
    wxTimeSpan dt = m_dateTime.Subtract(other.m_dateTime) + htol;
    return dt.IsPositive() ? dt.GetDays() : dt.GetDays() - 1;
}
inline int mmDate::daysUntil(const mmDate& other) const
{
    wxTimeSpan dt = other.m_dateTime.Subtract(m_dateTime) + htol;
    return dt.IsPositive() ? dt.GetDays() : dt.GetDays() - 1;
}

// The time in both operands is set to noon, therefore
// a simple comparison of m_dateTime is sufficient.
// For more robustness we compare with a tolerance of half day.
inline bool mmDate::operator== (const mmDate& other) const
{
    return (m_dateTime < other.m_dateTime + htol && m_dateTime + htol >= other.m_dateTime);
}
inline bool mmDate::operator!= (const mmDate& other) const
{
    return (m_dateTime >= other.m_dateTime + htol || m_dateTime + htol < other.m_dateTime);
}
inline bool mmDate::operator< (const mmDate& other) const
{
    return m_dateTime + htol < other.m_dateTime;
}
inline bool mmDate::operator> (const mmDate& other) const
{
    return m_dateTime >= other.m_dateTime + htol;
}
inline bool mmDate::operator<= (const mmDate& other) const
{
    return (m_dateTime < other.m_dateTime + htol);
}
inline bool mmDate::operator>= (const mmDate& other) const
{
    return (m_dateTime + htol >= other.m_dateTime);
}

inline bool mmDateN::has_value() const
{
    return m_dateTimeN.IsValid();
}
inline mmDate mmDateN::value() const
{
    return mmDate(m_dateTimeN);
}
inline mmDate mmDateN::value_or(mmDate defDateDay) const
{
    return m_dateTimeN.IsValid() ? mmDate(m_dateTimeN) : defDateDay;
}

inline wxDateTime mmDateN::getDateTimeN() const
{
    return m_dateTimeN;
}
inline const wxString mmDateN::isoDateN() const
{
    return has_value() ? value().isoDate() : "";
}
inline const wxString mmDateN::isoStartN() const
{
    return has_value() ? value().isoStart() : "";
}
inline const wxString mmDateN::isoEndN() const
{
    return has_value() ? value().isoEnd() : "~";
}

inline bool mmDateN::operator== (const mmDateN& other) const
{
    return (!has_value() && !other.has_value())
        || (has_value() && other.has_value() && value() == other.value());
}
inline bool mmDateN::operator!= (const mmDateN& other) const
{
    return (has_value() || other.has_value())
        && (!has_value() || !other.has_value() || value() != other.value());
}

