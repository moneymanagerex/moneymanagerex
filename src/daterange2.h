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
#include <unordered_map>
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
    enum PERIOD_ID
    {
        PERIOD_ID_A = 0, // all
        PERIOD_ID_Y,     // year = 4 quarters = 12 months
        PERIOD_ID_Q,     // quarter = 3 months
        PERIOD_ID_M,     // month
        PERIOD_ID_W,     // week = 7 days
        PERIOD_ID_T,     // day (today)
        PERIOD_ID_S,     // day (statement date)
        PERIOD_ID_size,
        PERIOD_ID_none = PERIOD_ID_size
    };

    typedef struct { PERIOD_ID id; wxString label; } PERIOD_INFO_t;
    typedef std::unordered_map<char, PERIOD_ID> PERIOD_LABEL_ID_t;
    static const PERIOD_INFO_t PERIOD_INFO[];
    static const PERIOD_LABEL_ID_t PERIOD_LABEL_ID;
    static wxDateSpan span(int offset, PERIOD_ID period);

private:
    static PERIOD_LABEL_ID_t make_period_label_id();

public:
    class Range
    {
        friend class DateRange2;

    protected:
        int so1 = 0;                    // start offset of first subrange
        int eo1 = 0;                    // end   offset of first subrange
        PERIOD_ID sp1 = PERIOD_ID_A;    // start period of first subrange
        PERIOD_ID ep1 = PERIOD_ID_A;    // end   period of first subrange
        int so2 = 0;                    // start offset of second subrange
        int eo2 = 0;                    // end   offset of second subrange
        PERIOD_ID sp2 = PERIOD_ID_none; // start period of second subrange
        PERIOD_ID ep2 = PERIOD_ID_none; // end   period of second subrange
        int f = 0;                      // index in first*[] (0=calendar, 1=financial)
        wxString name = "";             // specification name

    public:
        Range(
            int so1_new = 0, PERIOD_ID sp1_new = PERIOD_ID_A,
            int eo1_new = 0, PERIOD_ID ep1_new = PERIOD_ID_A,
            int so2_new = 0, PERIOD_ID sp2_new = PERIOD_ID_none,
            int eo2_new = 0, PERIOD_ID ep2_new = PERIOD_ID_none,
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
        static char scanToken(StringIt &buffer_i, StringIt buffer_end, int &token_o, PERIOD_ID &token_p);
        static const wxString offsetStr(int offset, bool show_zero = false);
        static const wxString offsetRangeStr(int so, int eo, bool show_zero = false);
    };

public:
    class Reporting
    {
        friend class DateRange2;

    protected:
        int m = 0;                    // multiplier
        PERIOD_ID p = PERIOD_ID_none; // reporting period

    public:
        Reporting(int m_new = 0, PERIOD_ID p_new = PERIOD_ID_none);

    public:
        bool parseLabel(StringIt &buffer_i, StringIt buffer_end);
        const wxString getLabel() const;

    private:
        static const wxString multiplierStr(int m, bool show_one = false);
    };

protected:
    int firstDay[2];                  // first day in PERIOD_ID_[YQM] (1..28)
    wxDateTime::Month firstMonth[2];  // first month in PERIOD_ID_[YQ] (0..11)
    wxDateTime::WeekDay firstWeekday; // first weekday in PERIOD_ID_W (0=Sun, 1=Mon)
    DateDay date_s;                   // the date of PERIOD_ID_S (MAY be undefined)
    DateDay date_t;                   // the date of PERIOD_ID_T (MUST be defined)
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
    DateDay periodStart(DateDay date, PERIOD_ID period) const;
    DateDay periodEnd(DateDay date, PERIOD_ID period) const;
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
        sp1 == PERIOD_ID_S || ep1 == PERIOD_ID_S ||
        sp2 == PERIOD_ID_S || ep2 == PERIOD_ID_S;
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

