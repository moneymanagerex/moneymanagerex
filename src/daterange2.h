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

# pragma once

#include <wx/datetime.h>
#include <optional>
#include <unordered_map>
#include "primitive.h"
#include "dateday.h"

class DatePeriod
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
    DatePeriod(Id id_new = _A);
    DatePeriod(char label);

public:
    static wxDateSpan span(int offset, DatePeriod period);

public:
    int toInt() const;
    const wxString label() const;
    bool operator== (DatePeriod other) const;
    bool operator== (Id other_id) const;
};

using DatePeriodN = std::optional<DatePeriod>;

inline int DatePeriod::toInt() const
{
    return static_cast<int>(id);
}

inline const wxString DatePeriod::label() const
{
    return mapIdLabel[toInt()].label;
}

inline bool DatePeriod::operator== (DatePeriod other) const
{
    return id == other.id;
}
inline bool DatePeriod::operator== (DatePeriod::Id other_id) const
{
    return id == other_id;
}

// DateRange2 represents a date range relative to today.
// The range specification consists of the following:
//   two subranges
//   a reporting multiplier/period
//   a selector between calendar or financial year/quarter/month
//   a descriptive name
// Each subrange consists of a start offset/period and an end offset/period.
// To avoid instability, the date of today as well as a few user preferences,
// are cached at construction time.

class DateRange2
{
public:
    class Range
    {
        friend class DateRange2;

    protected:
        int so1 = 0;                      // start offset of first subrange
        int eo1 = 0;                      // end   offset of first subrange
        DatePeriod sp1 = DatePeriod::_A;  // start period of first subrange
        DatePeriod ep1 = DatePeriod::_A;  // end   period of first subrange
        int so2 = 0;                      // start offset of second subrange
        int eo2 = 0;                      // end   offset of second subrange
        DatePeriodN sp2 = DatePeriodN();  // start period of second subrange
        DatePeriodN ep2 = DatePeriodN();  // end   period of second subrange
        int f = 0;                        // index in first*[] (0=calendar, 1=financial)
        wxString name = "";               // specification name

    public:
        Range(
            int so1_new = 0, DatePeriod  sp1_new = DatePeriod::_A,
            int eo1_new = 0, DatePeriod  ep1_new = DatePeriod::_A,
            int so2_new = 0, DatePeriodN sp2_new = DatePeriodN(),
            int eo2_new = 0, DatePeriodN ep2_new = DatePeriodN(),
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
        static char scanToken(StringIt &buffer_i, StringIt buffer_end, int &token_o, DatePeriod &token_p);
        static const wxString offsetStr(int offset, bool show_zero = false);
        static const wxString offsetRangeStr(int so, int eo, bool show_zero = false);
    };

public:
    class Reporting
    {
        friend class DateRange2;

    protected:
        int m = 1;                      // multiplier
        DatePeriod p = DatePeriod::_A;  // reporting period

    public:
        Reporting(int m_new = 1, DatePeriod p_new = DatePeriod::_A);

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
    DateDayN sDateN;                  // the date of PERIOD::S (account statement date)
    DateDay  tDate;                   // the date of PERIOD::T (today)
    DateDayN defStartDateN;           // default start date (if range start is open)
    DateDayN defEndDateN;             // default end date (if range end is open)
    Range range;                      // range specification
    Reporting reporting;              // reporting multiplier/period

public:
    DateRange2(
        DateDayN sDateN_new = DateDayN(),
        DateDay  tDate_new = DateDay::today(),
        DateDayN defStartDateN_new = DateDayN(),
        DateDayN defEndDateN_new = DateDayN()
    );

#ifndef NDEBUG
private:
    DateRange2(
        int firstDay_new_0, int firstDay_new_1,
        wxDateTime::Month firstMonth_new_0, wxDateTime::Month firstMonth_new_1,
        wxDateTime::WeekDay firstWeekday_new,
        DateDayN sDateN_new = DateDayN(),
        DateDay  tDate_new = DateDay::today(),
        DateDayN defStartDateN_new = DateDayN(),
        DateDayN defEndDateN_new = DateDayN()
    );
#endif

public:
    void setSDateN(DateDayN sDateN_new = DateDayN());
    void setTDate(DateDay tDate_new = DateDay::today());
    void setDefStartDateN(DateDayN defStartDateN_new = DateDayN());
    void setDefEndDateN(DateDayN defEndDateN_new = DateDayN());
    void setRange(const Range &range_new);
    void setReporting(const Reporting &reporting_new);
    int getFirstDay() const;
    wxDateTime::Month getFirstMonth() const;
    wxDateTime::WeekDay getFirstWeekday() const;
    DateDayN getSDateN() const;
    DateDay  getTDate() const;
    DateDayN getDefStartDateN() const;
    DateDayN getDefEndDateN() const;
    Range getRange() const;
    Reporting getReporting() const;
    bool parseRange(const wxString &buffer, const wxString &name = "");
    bool parseReporting(const wxString &buffer);
    const wxString rangeLabel() const;
    const wxString rangeName() const;
    const wxString rangeLabelName() const;
    const wxString reportingLabel() const;
    DateDayN periodStart(DateDay date, DatePeriod period) const;
    DateDayN periodEnd(DateDay date, DatePeriod period) const;
    DateDayN rangeStart() const;
    DateDayN rangeEnd() const;
    DateDayN reportingNext() const;
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
        using value_type = DateDayN;
        using pointer = const value_type*;
        using reference = const value_type&;

        friend class DateRange2;

    private:
        const DateRange2* a;
        int count;
        DateDayN nextDateN, lastDateN;

    public:
        ReportingIterator(const DateRange2* a_new);

    public:
        const DateDayN& operator*();
        const DateDayN* operator->();
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

inline void DateRange2::Range::setName(const wxString &name_new)
{
    name = name_new;
}

inline int DateRange2::Range::getF() const
{
    return f;
}

inline const wxString DateRange2::Range::getName() const
{
    return name;
}

inline bool DateRange2::Range::hasPeriodS() const
{
    return
        sp1 == DatePeriod::_S || ep1 == DatePeriod::_S ||
        sp2 == DatePeriod::_S || ep2 == DatePeriod::_S;
}

inline const wxString DateRange2::Range::offsetStr(int offset, bool show_zero)
{
    return (offset != 0) ? wxString::Format("%+d", offset) : show_zero ? "0" : "";
}
inline const wxString DateRange2::Reporting::multiplierStr(int m, bool show_one)
{
    return (m != 1 || show_one) ? wxString::Format("%+d", m) : "";
}

inline void DateRange2::setSDateN(DateDayN sDateN_new)
{
    sDateN = sDateN_new;
}
inline void DateRange2::setTDate(DateDay tDate_new)
{
    tDate = tDate_new;
}
inline void DateRange2::setDefStartDateN(DateDayN defStartDateN_new)
{
    defStartDateN = defStartDateN_new;
}
inline void DateRange2::setDefEndDateN(DateDayN defEndDateN_new)
{
    defEndDateN = defEndDateN_new;
}
inline void DateRange2::setRange(const DateRange2::Range &range_new)
{
    range = range_new;
}
inline void DateRange2::setReporting(const DateRange2::Reporting &reporting_new)
{
    reporting = reporting_new;
}

inline int DateRange2::getFirstDay() const
{
    return firstDay[range.f];
}
inline wxDateTime::Month DateRange2::getFirstMonth() const
{
    return firstMonth[range.f];
}
inline wxDateTime::WeekDay DateRange2::getFirstWeekday() const
{
    return firstWeekday;
}
inline DateDayN DateRange2::getSDateN() const
{
    return sDateN;
}
inline DateDay DateRange2::getTDate() const
{
    return tDate;
}
inline DateDayN DateRange2::getDefStartDateN() const
{
    return defStartDateN;
}
inline DateDayN DateRange2::getDefEndDateN() const
{
    return defEndDateN;
}
inline DateRange2::Range DateRange2::getRange() const
{
    return range;
}
inline DateRange2::Reporting DateRange2::getReporting() const
{
    return reporting;
}

inline const wxString DateRange2::rangeLabel() const
{
    return range.getLabel();
}
inline const wxString DateRange2::rangeName() const
{
    return range.getName();
}
inline const wxString DateRange2::rangeLabelName() const
{
    return range.getLabelName();
}
inline const wxString DateRange2::reportingLabel() const
{
    return reporting.getLabel();
}

inline const wxString DateRange2::rangeStartIsoStartN() const
{
    return rangeStart().isoStartN();
}
inline const wxString DateRange2::rangeEndIsoEndN() const
{
    return rangeEnd().isoEndN();
}
inline const wxString DateRange2::reportingNextIsoEndN() const
{
    return reportingNext().isoEndN();
}

inline const DateDayN& DateRange2::ReportingIterator::operator*()
{
    return nextDateN;
}
inline const DateDayN* DateRange2::ReportingIterator::operator->()
{
    return &nextDateN;
}
inline DateRange2::ReportingIterator& DateRange2::ReportingIterator::operator++()
{
    increment();
    return *this;
}
inline DateRange2::ReportingIterator DateRange2::ReportingIterator::operator++(int)
{
    DateRange2::ReportingIterator tmp = *this;
    ++(*this);
    return tmp;
}
inline bool DateRange2::ReportingIterator::operator== (const ReportingIterator& other)
{
    return count == other.count;
}
inline bool DateRange2::ReportingIterator::operator!= (const ReportingIterator& other)
{
    return count != other.count;
}
inline bool DateRange2::ReportingIterator::operator== (int other_count)
{
    return count == other_count;
}
inline bool DateRange2::ReportingIterator::operator!= (int other_count)
{
    return count != other_count;
}

inline DateRange2::ReportingIterator DateRange2::cbegin() const
{
    return ReportingIterator(this);
}

inline int DateRange2::cend() const
{
    return -1;
}

