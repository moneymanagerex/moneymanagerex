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

// mmDateTime represents a date and time, with resolution of a second.
// The underlying data structure is wxDateTime.
// The underlying value cannot be wxInvalidDateTime.
//
// mmDateTimeN is an optional (nullable) mmDateTime.
// The underlying null value is wxInvalidDateTime.

struct mmDateTime
{
    friend struct mmDateTimeN;

protected:
    wxDateTime m_dateTime;

public:
    mmDateTime(wxDateTime dateTime);
    mmDateTime(const wxString& isoDateTime);

public:
    static const wxTimeSpan htol; // half-second tolerance
    static mmDateTime now();
    static mmDateTime from_utc(const wxString& utcDateTime);

public:
    auto getDateTime() const -> wxDateTime;
    auto isoDateTime() const -> const wxString;
    auto utcDateTime() const -> const wxString;
    void addDateSpan(wxDateSpan dateSpan);
    void addTimeSpan(wxTimeSpan timeSpan);

public:
    bool operator== (const mmDateTime& other) const;
    bool operator!= (const mmDateTime& other) const;
    bool operator<  (const mmDateTime& other) const;
    bool operator>  (const mmDateTime& other) const;
    bool operator<= (const mmDateTime& other) const;
    bool operator>= (const mmDateTime& other) const;
};

struct mmDateTimeN
{
    friend struct mmDateTime;

private:
    wxDateTime m_dateTimeN;

public:
    mmDateTimeN() = default;
    mmDateTimeN(mmDateTime dateTime);
    mmDateTimeN(wxDateTime dateTimeN);
    mmDateTimeN(const wxString& isoDateTimeN);

public:
    static mmDateTimeN from_utc(const wxString& utcDateTimeN);

public:
    bool has_value() const;
    auto value() const -> mmDateTime;
    auto value_or(mmDateTime defDateTime) const -> mmDateTime;

public:
    auto getDateTimeN() const -> wxDateTime;
    auto isoDateTimeN() const -> const wxString;
    auto utcDateTimeN() const -> const wxString;

public:
    bool operator== (const mmDateTimeN& other) const;
    bool operator!= (const mmDateTimeN& other) const;
};

inline mmDateTime mmDateTime::now()
{
    return mmDateTime(wxDateTime::Now());
}
inline mmDateTime mmDateTime::from_utc(const wxString& utcDateTime)
{
    return mmDateTime(parseDateTime(utcDateTime).FromUTC());
}

inline wxDateTime mmDateTime::getDateTime() const
{
    return m_dateTime;
}

inline const wxString mmDateTime::isoDateTime() const
{
    return m_dateTime.FormatISOCombined();
}
inline const wxString mmDateTime::utcDateTime() const
{
    return m_dateTime.ToUTC().FormatISOCombined();
}

inline void mmDateTime::addDateSpan(wxDateSpan dateSpan)
{
    // assumption: dateSpan has granularity of a day or larger
    m_dateTime += dateSpan;
}
inline void mmDateTime::addTimeSpan(wxTimeSpan timeSpan)
{
    // assumption: timeSpan has granularity of a second or larger
    m_dateTime += timeSpan;
}

// The milliseconds in both operands is set to zero, therefore
// a simple comparison of m_dateTime is sufficient.
// For more robustness we compare with a tolerance of half second.
inline bool mmDateTime::operator== (const mmDateTime& other) const
{
    return (m_dateTime < other.m_dateTime + htol && m_dateTime + htol >= other.m_dateTime);
}
inline bool mmDateTime::operator!= (const mmDateTime& other) const
{
    return (m_dateTime >= other.m_dateTime + htol || m_dateTime + htol < other.m_dateTime);
}
inline bool mmDateTime::operator< (const mmDateTime& other) const
{
    return m_dateTime + htol < other.m_dateTime;
}
inline bool mmDateTime::operator> (const mmDateTime& other) const
{
    return m_dateTime >= other.m_dateTime + htol;
}
inline bool mmDateTime::operator<= (const mmDateTime& other) const
{
    return (m_dateTime < other.m_dateTime + htol);
}
inline bool mmDateTime::operator>= (const mmDateTime& other) const
{
    return (m_dateTime + htol >= other.m_dateTime);
}

inline mmDateTimeN mmDateTimeN::from_utc(const wxString& utcDateTimeN)
{
    wxDateTime dateTimeN = parseDateTime(utcDateTimeN);
    return dateTimeN.IsValid() ? mmDateTimeN(dateTimeN.FromUTC()) : mmDateTimeN();
}

inline bool mmDateTimeN::has_value() const
{
    return m_dateTimeN.IsValid();
}
inline mmDateTime mmDateTimeN::value() const
{
    return mmDateTime(m_dateTimeN);
}
inline mmDateTime mmDateTimeN::value_or(mmDateTime defDateTime) const
{
    return m_dateTimeN.IsValid() ? mmDateTime(m_dateTimeN) : defDateTime;
}

inline wxDateTime mmDateTimeN::getDateTimeN() const
{
    return m_dateTimeN;
}
inline const wxString mmDateTimeN::isoDateTimeN() const
{
    return has_value() ? value().isoDateTime() : "";
}
inline const wxString mmDateTimeN::utcDateTimeN() const
{
    return has_value() ? value().utcDateTime() : "";
}

inline bool mmDateTimeN::operator== (const mmDateTimeN& other) const
{
    return (!has_value() && !other.has_value())
        || (has_value() && other.has_value() && value() == other.value());
}
inline bool mmDateTimeN::operator!= (const mmDateTimeN& other) const
{
    return (has_value() || other.has_value())
        && (!has_value() || !other.has_value() || value() != other.value());
}

