/*******************************************************
 Copyright (C) 2025 George Ef (george.a.ef@gmail.com)

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
#include <wx/datetime.h>
#include <optional>
#include <unordered_map>

#include "_primitive.h"
#include "mmDateDay.h"

class mmDatePeriod
{
public:
    enum Id
    {
        _A = 0,     // all
        _Y,         // year = 4 quarters = 12 months
        _Q,         // quarter = 3 months
        _M,         // month
        _W,         // week = 7 days
        _T,         // day (today)
        _S,         // day (account statement date)
        _min = _A,  // (min value)
        _max = _S,  // (max value)
    };

    typedef struct { Id id; const wxString label; } MapIdLabel;
    typedef std::unordered_map<char, Id> MapLabelId;
    static const MapIdLabel mapIdLabel[];
    static const MapLabelId mapLabelId;

private:
    static MapLabelId makeLabelId();

private:
    Id id;

public:
    mmDatePeriod(Id id_new = _A);
    mmDatePeriod(char label);

public:
    static wxDateSpan span(int offset, mmDatePeriod period);

public:
    int toInt() const;
    const wxString label() const;
    bool operator== (mmDatePeriod other) const;
    bool operator== (Id other_id) const;
};

using mmDatePeriodN = std::optional<mmDatePeriod>;

inline int mmDatePeriod::toInt() const
{
    return static_cast<int>(id);
}

inline const wxString mmDatePeriod::label() const
{
    return mapIdLabel[toInt()].label;
}

inline bool mmDatePeriod::operator== (mmDatePeriod other) const
{
    return id == other.id;
}
inline bool mmDatePeriod::operator== (mmDatePeriod::Id other_id) const
{
    return id == other_id;
}

// mmDateRange2 represents a date range relative to today.
// The range specification consists of the following:
//   two subranges
//   a reporting multiplier/period
//   a selector between calendar or financial year/quarter/month
//   a descriptive name
// Each subrange consists of a start offset/period and an end offset/period.
// To avoid instability, the date of today as well as a few user preferences,
// are cached at construction time.

class mmDateRange2
{
public:
    class Range
    {
        friend class mmDateRange2;

    protected:
        int so1 = 0;                          // start offset of first subrange
        int eo1 = 0;                          // end   offset of first subrange
        mmDatePeriod sp1 = mmDatePeriod::_A;  // start period of first subrange
        mmDatePeriod ep1 = mmDatePeriod::_A;  // end   period of first subrange
        int so2 = 0;                          // start offset of second subrange
        int eo2 = 0;                          // end   offset of second subrange
        mmDatePeriodN sp2 = mmDatePeriodN();  // start period of second subrange
        mmDatePeriodN ep2 = mmDatePeriodN();  // end   period of second subrange
        int f = 0;                            // index in first*[] (0=calendar, 1=financial)
        wxString name = "";                   // specification name

    public:
        Range(
            int so1_new = 0, mmDatePeriod  sp1_new = mmDatePeriod::_A,
            int eo1_new = 0, mmDatePeriod  ep1_new = mmDatePeriod::_A,
            int so2_new = 0, mmDatePeriodN sp2_new = mmDatePeriodN(),
            int eo2_new = 0, mmDatePeriodN ep2_new = mmDatePeriodN(),
            int f_new = 0, wxString name_new = ""
        );

    public:
        void setName(const wxString &name_new);
        bool parseLabel(StringIt &buffer_i, StringIt buffer_end);
        void parseName(StringIt &buffer_i, StringIt buffer_end);
        bool parseLabelName(const wxString &buffer, const wxString &name_new = "");
        int getF() const;
        const wxString getLabel() const;
        const wxString getName() const;
        const wxString getLabelName() const;
        const wxString checkingName() const;
        const wxString checkingDescription() const;
        bool hasPeriodS() const;

    private:
        static void scanWhiteSpace(StringIt &buffer_i, StringIt buffer_end);
        static char scanToken(StringIt &buffer_i, StringIt buffer_end, int &token_o, mmDatePeriod &token_p);
        static const wxString offsetStr(int offset, bool show_zero = false);
        static const wxString offsetRangeStr(int so, int eo, bool show_zero = false);
    };

public:
    class Reporting
    {
        friend class mmDateRange2;

    protected:
        int m;           // multiplier
        mmDatePeriod p;  // reporting period

    public:
        Reporting(int m_new = 1, mmDatePeriod p_new = mmDatePeriod::_A);

    public:
        bool parseLabel(StringIt &buffer_i, StringIt buffer_end);
        const wxString getLabel() const;

    private:
        static const wxString multiplierStr(int m, bool show_one = false);
    };

protected:
    int firstDay[2];                  // first day in PERIOD::[YQM] (1..28)
    wxDateTime::Month firstMonth[2];  // first month in PERIOD::[YQ] (0..11)
    wxDateTime::WeekDay firstWeekday; // first weekday in PERIOD::W (0=Sun, 1=Mon)
    mmDateDayN sDateN;                // the date of PERIOD::S (account statement date)
    mmDateDay  tDate;                 // the date of PERIOD::T (today)
    mmDateDayN defStartDateN;         // default start date (if range start is open)
    mmDateDayN defEndDateN;           // default end date (if range end is open)
    Range range;                      // range specification
    Reporting reporting;              // reporting multiplier/period

public:
    mmDateRange2(
        mmDateDayN sDateN_new = mmDateDayN(),
        mmDateDay  tDate_new = mmDateDay::today(),
        mmDateDayN defStartDateN_new = mmDateDayN(),
        mmDateDayN defEndDateN_new = mmDateDayN()
    );

#ifndef NDEBUG
private:
    mmDateRange2(
        int firstDay_new_0, int firstDay_new_1,
        wxDateTime::Month firstMonth_new_0, wxDateTime::Month firstMonth_new_1,
        wxDateTime::WeekDay firstWeekday_new,
        mmDateDayN sDateN_new = mmDateDayN(),
        mmDateDay  tDate_new = mmDateDay::today(),
        mmDateDayN defStartDateN_new = mmDateDayN(),
        mmDateDayN defEndDateN_new = mmDateDayN()
    );
#endif

public:
    void setSDateN(mmDateDayN sDateN_new = mmDateDayN());
    void setTDate(mmDateDay tDate_new = mmDateDay::today());
    void setDefStartDateN(mmDateDayN defStartDateN_new = mmDateDayN());
    void setDefEndDateN(mmDateDayN defEndDateN_new = mmDateDayN());
    void setRange(const Range &range_new);
    void setReporting(const Reporting &reporting_new);
    int getFirstDay() const;
    wxDateTime::Month getFirstMonth() const;
    wxDateTime::WeekDay getFirstWeekday() const;
    mmDateDayN getSDateN() const;
    mmDateDay  getTDate() const;
    mmDateDayN getDefStartDateN() const;
    mmDateDayN getDefEndDateN() const;
    Range getRange() const;
    Reporting getReporting() const;
    bool parseRange(const wxString &buffer, const wxString &name = "");
    bool parseReporting(const wxString &buffer);
    const wxString rangeLabel() const;
    const wxString rangeName() const;
    const wxString rangeLabelName() const;
    const wxString reportingLabel() const;
    mmDateDayN periodStart(mmDateDay date, mmDatePeriod period) const;
    mmDateDayN periodEnd(mmDateDay date, mmDatePeriod period) const;
    mmDateDayN rangeStart() const;
    mmDateDayN rangeEnd() const;
    mmDateDayN reportingNext() const;
    const wxString rangeStartIsoStartN() const;
    const wxString rangeEndIsoEndN() const;
    const wxString reportingNextIsoEndN() const;
    const wxString checkingTooltip() const;
    const wxString reportingTooltip() const;

public:
    struct ReportingIterator
    {
        using iterator_category = std::input_iterator_tag;
        using difference_type = int;
        using value_type = mmDateDayN;
        using pointer = const value_type*;
        using reference = const value_type&;

        friend class mmDateRange2;

    private:
        const mmDateRange2* a;
        int count;
        mmDateDayN nextDateN, lastDateN;

    public:
        ReportingIterator(const mmDateRange2* a_new);

    public:
        const mmDateDayN& operator*();
        const mmDateDayN* operator->();
        ReportingIterator& operator++();
        ReportingIterator operator++(int);
        bool operator== (const ReportingIterator& other);
        bool operator!= (const ReportingIterator& other);
        bool operator== (int other_count);
        bool operator!= (int other_count);

    private:
        void increment();
    };

    ReportingIterator cbegin() const;
    int cend() const;

#ifndef NDEBUG
public:
    static bool debug();
#endif
};

inline void mmDateRange2::Range::setName(const wxString &name_new)
{
    name = name_new;
}

inline int mmDateRange2::Range::getF() const
{
    return f;
}

inline const wxString mmDateRange2::Range::getName() const
{
    return name;
}

inline bool mmDateRange2::Range::hasPeriodS() const
{
    return
        sp1 == mmDatePeriod::_S || ep1 == mmDatePeriod::_S ||
        sp2 == mmDatePeriod::_S || ep2 == mmDatePeriod::_S;
}

inline const wxString mmDateRange2::Range::offsetStr(int offset, bool show_zero)
{
    return (offset != 0) ? wxString::Format("%+d", offset) : show_zero ? "0" : "";
}
inline const wxString mmDateRange2::Reporting::multiplierStr(int m, bool show_one)
{
    return (m != 1 || show_one) ? wxString::Format("%+d", m) : "";
}

inline void mmDateRange2::setSDateN(mmDateDayN sDateN_new)
{
    sDateN = sDateN_new;
}
inline void mmDateRange2::setTDate(mmDateDay tDate_new)
{
    tDate = tDate_new;
}
inline void mmDateRange2::setDefStartDateN(mmDateDayN defStartDateN_new)
{
    defStartDateN = defStartDateN_new;
}
inline void mmDateRange2::setDefEndDateN(mmDateDayN defEndDateN_new)
{
    defEndDateN = defEndDateN_new;
}
inline void mmDateRange2::setRange(const mmDateRange2::Range &range_new)
{
    range = range_new;
}
inline void mmDateRange2::setReporting(const mmDateRange2::Reporting &reporting_new)
{
    reporting = reporting_new;
}

inline int mmDateRange2::getFirstDay() const
{
    return firstDay[range.f];
}
inline wxDateTime::Month mmDateRange2::getFirstMonth() const
{
    return firstMonth[range.f];
}
inline wxDateTime::WeekDay mmDateRange2::getFirstWeekday() const
{
    return firstWeekday;
}
inline mmDateDayN mmDateRange2::getSDateN() const
{
    return sDateN;
}
inline mmDateDay mmDateRange2::getTDate() const
{
    return tDate;
}
inline mmDateDayN mmDateRange2::getDefStartDateN() const
{
    return defStartDateN;
}
inline mmDateDayN mmDateRange2::getDefEndDateN() const
{
    return defEndDateN;
}
inline mmDateRange2::Range mmDateRange2::getRange() const
{
    return range;
}
inline mmDateRange2::Reporting mmDateRange2::getReporting() const
{
    return reporting;
}

inline const wxString mmDateRange2::rangeLabel() const
{
    return range.getLabel();
}
inline const wxString mmDateRange2::rangeName() const
{
    return range.getName();
}
inline const wxString mmDateRange2::rangeLabelName() const
{
    return range.getLabelName();
}
inline const wxString mmDateRange2::reportingLabel() const
{
    return reporting.getLabel();
}

inline const wxString mmDateRange2::rangeStartIsoStartN() const
{
    return rangeStart().isoStartN();
}
inline const wxString mmDateRange2::rangeEndIsoEndN() const
{
    return rangeEnd().isoEndN();
}
inline const wxString mmDateRange2::reportingNextIsoEndN() const
{
    return reportingNext().isoEndN();
}

inline const mmDateDayN& mmDateRange2::ReportingIterator::operator*()
{
    return nextDateN;
}
inline const mmDateDayN* mmDateRange2::ReportingIterator::operator->()
{
    return &nextDateN;
}
inline mmDateRange2::ReportingIterator& mmDateRange2::ReportingIterator::operator++()
{
    increment();
    return *this;
}
inline mmDateRange2::ReportingIterator mmDateRange2::ReportingIterator::operator++(int)
{
    mmDateRange2::ReportingIterator tmp = *this;
    ++(*this);
    return tmp;
}
inline bool mmDateRange2::ReportingIterator::operator== (const ReportingIterator& other)
{
    return count == other.count;
}
inline bool mmDateRange2::ReportingIterator::operator!= (const ReportingIterator& other)
{
    return count != other.count;
}
inline bool mmDateRange2::ReportingIterator::operator== (int other_count)
{
    return count == other_count;
}
inline bool mmDateRange2::ReportingIterator::operator!= (int other_count)
{
    return count != other_count;
}

inline mmDateRange2::ReportingIterator mmDateRange2::cbegin() const
{
    return ReportingIterator(this);
}

inline int mmDateRange2::cend() const
{
    return -1;
}

