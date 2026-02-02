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
        _S,         // day (statement date)
        _min = _A,  // (min value)
        _max = _S,  // (max value)
    };

    typedef struct { Id id; const wxString label; } MapIdLabel;
    typedef std::unordered_map<char, Id> MapLabelId;
    static const MapIdLabel mapIdLabel[];
    static const MapLabelId mapLabelId;

private:
    static MapLabelId makeLabelId();

public:
    static wxDateSpan span(int offset, DatePeriod period);

private:
    Id id;

public:
    DatePeriod(Id id_new = _A);
    DatePeriod(char label);

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
        DatePeriodN sp2 = std::nullopt;   // start period of second subrange
        DatePeriodN ep2 = std::nullopt;   // end   period of second subrange
        int f = 0;                        // index in first*[] (0=calendar, 1=financial)
        wxString name = "";               // specification name

    public:
        Range(
            int so1_new = 0, DatePeriod  sp1_new = DatePeriod::_A,
            int eo1_new = 0, DatePeriod  ep1_new = DatePeriod::_A,
            int so2_new = 0, DatePeriodN sp2_new = std::nullopt,
            int eo2_new = 0, DatePeriodN ep2_new = std::nullopt,
            int f_new = 0, wxString name_new = ""
        );

    public:
        void setName(const wxString &name_new);
        bool parseLabel(StringIt &buffer_i, StringIt buffer_end);
        void parseName(StringIt &buffer_i, StringIt buffer_end);
        bool parseLabelName(const wxString &buffer, const wxString &name_new = "");
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
    DateDay date_s;                   // the date of PERIOD::S (MAY be undefined)
    DateDay date_t;                   // the date of PERIOD::T (MUST be defined)
    DateDay default_start;            // default start date (if range start is open)
    DateDay default_end;              // default end date (if range end is open)
    Range range;                      // range specification
    Reporting reporting;              // reporting multiplier/period

public:
    DateRange2(
        DateDay date_s_new = DateDay(), DateDay date_t_new = DateDay(),
        DateDay default_start_new = DateDay(), DateDay default_end_new = DateDay()
    );

#ifndef NDEBUG
private:
    DateRange2(
        int firstDay_new_0, int firstDay_new_1,
        wxDateTime::Month firstMonth_new_0, wxDateTime::Month firstMonth_new_1,
        wxDateTime::WeekDay firstWeekday_new,
        DateDay date_t_new = DateDay(), DateDay date_s_new = DateDay(),
        DateDay default_start_new = DateDay(), DateDay default_end_new = DateDay()
    );
#endif

public:
    void setDateS(DateDay date = DateDay());
    void setDateT(DateDay date = DateDay());
    void setDefaultStart(DateDay date = DateDay());
    void setDefaultEnd(DateDay date = DateDay());
    void setRange(const Range &range_new);
    void setReporting(const Reporting &reporting_new);
    DateDay getDateS() const;
    DateDay getDateT() const;
    DateDay getDefaultStart() const;
    DateDay getDefaultEnd() const;
    Range getRange() const;
    Reporting getReporting() const;
    bool parseRange(const wxString &buffer, const wxString &name = "");
    bool parseReporting(const wxString &buffer);
    const wxString rangeLabel() const;
    const wxString rangeName() const;
    const wxString rangeLabelName() const;
    const wxString reportingLabel() const;
    DateDay periodStart(DateDay date, DatePeriod period) const;
    DateDay periodEnd(DateDay date, DatePeriod period) const;
    DateDay rangeStart() const;
    DateDay rangeEnd() const;
    DateDay reportingNext() const;
    const wxString rangeStartISO() const;
    const wxString rangeEndISO() const;
    const wxString reportingNextISO() const;
    const wxString checkingTooltip() const;
    const wxString reportingTooltip() const;

public:
    struct ReportingIterator
    {
        using iterator_category = std::input_iterator_tag;
        using difference_type = int;
        using value_type = DateDay;
        using pointer = const value_type*;
        using reference = const value_type&;

        friend class DateRange2;

    private:
        const DateRange2* a;
        int count;
        DateDay next, last;

    public:
        ReportingIterator(const DateRange2* a_new);

    public:
        const DateDay& operator*();
        const DateDay* operator->();
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

inline void DateRange2::setDateS(DateDay date)
{
    // date_s MAY be undefined (not applicable)
    date_s = date;
}

inline void DateRange2::setDateT(DateDay date)
{
    // date_t MUST be defined
    if (date.isDefined())
        date_t = date;
    else {
        date_t = DateDay::today();
    }
}

inline void DateRange2::setDefaultStart(DateDay date)
{
    default_start = date;
}

inline void DateRange2::setDefaultEnd(DateDay date)
{
    default_end = date;
}

inline void DateRange2::setRange(const DateRange2::Range &range_new)
{
    range = range_new;
}

inline void DateRange2::setReporting(const DateRange2::Reporting &reporting_new)
{
    reporting = reporting_new;
}

inline DateDay DateRange2::getDateS() const
{
    return date_s;
}

inline DateDay DateRange2::getDateT() const
{
    return date_t;
}

inline DateDay DateRange2::getDefaultStart() const
{
    return default_start;
}

inline DateDay DateRange2::getDefaultEnd() const
{
    return default_end;
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

inline const wxString DateRange2::rangeStartISO() const
{
    return rangeStart().dateStartISO();
}

inline const wxString DateRange2::rangeEndISO() const
{
    return rangeEnd().dateEndISO();
}

inline const wxString DateRange2::reportingNextISO() const
{
    return reportingNext().dateEndISO();
}

inline const DateDay& DateRange2::ReportingIterator::operator*()
{
    return next;
}
inline const DateDay* DateRange2::ReportingIterator::operator->()
{
    return &next;
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

