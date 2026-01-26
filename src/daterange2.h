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
//
// getISO{Start,End} are helper functions for date comparisons, e.g.,
//   start_date.getISOStart() <= date_str <= end_date.getISOEnd()
// checks if date_str (a date or date+time string in ISO format) is in the
// range (start_date..end_date), where the boundary {start,end}_date is
// inclusive if it is defined and open if it is undefined.

struct DateDay
{
public:
    enum PERIOD_ID
    {
        PERIOD_ID_A = 0, // all
        PERIOD_ID_Y,     // year = 4 quarters = 12 months
        PERIOD_ID_Q,     // quarter = 3 months
        PERIOD_ID_M,     // month
        PERIOD_ID_W,     // week = 7 days
        PERIOD_ID_T,     // today
        PERIOD_ID_S,     // statement date
        PERIOD_ID_size,
        PERIOD_ID_none = PERIOD_ID_size
    };

    typedef struct { PERIOD_ID id; wxString label; } PERIOD_INFO_t;
    typedef std::unordered_map<char, PERIOD_ID> PERIOD_LABEL_ID_t;
    static const PERIOD_INFO_t PERIOD_INFO[];
    static const PERIOD_LABEL_ID_t PERIOD_LABEL_ID;

private:
    static PERIOD_LABEL_ID_t make_period_label_id();

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
    const wxString getISO(const wxString defaultISO = "") const;
    const wxString getISOStart() const;
    const wxString getISOEnd() const;
    void addSpan(wxDateSpan span);
    void addOffset(int offset, DateDay::PERIOD_ID period);
};

inline DateDay DateDay::today()
{
    // get the dateTime of today, with time set to noon (12:00)
    wxDateTime today_dateTime = wxDateTime(12, 0, 0, 0);
    // the DateDay constructor anyway resets the time to noon
    return DateDay(today_dateTime);
}

inline bool DateDay::isDefined() const
{
    return dateTime.IsValid();
}

inline wxDateTime DateDay::getDateTime(wxDateTime defaultDateTime) const
{
    return dateTime.IsValid() ? dateTime : defaultDateTime;
}

inline const wxString DateDay::getISO(const wxString defaultISO) const
{
    return dateTime.IsValid() ? dateTime.FormatISODate() : defaultISO;
}

inline const wxString DateDay::getISOStart() const
{
    wxString dateISO = getISO();
    return dateISO.append("");
}

inline const wxString DateDay::getISOEnd() const
{
    // note: the ASCII code of "~" is larger than any character in ISO format
    wxString dateISO = getISO();
    return dateISO.append("~");
}

inline void DateDay::addSpan(wxDateSpan span)
{
    if (isDefined())
        dateTime += span;
}

// DateRange2 represents a date range relative to today.
// The range specification consists of two subranges, a selector between
// calendar or financial year/quarter/month, and a descriptive name.
// Each subrange consists of a start offset/period and an end offset/period.
// To avoid instability, the date of today as well as a few user preferences,
// are cached at construction time.

class DateRange2
{
public:
    class Spec
    {
        friend class DateRange2;

    protected:
        int so1 = 0;                                      // start offset of first subrange
        int eo1 = 0;                                      // end   offset of first subrange
        DateDay::PERIOD_ID sp1 = DateDay::PERIOD_ID_A;    // start period of first subrange
        DateDay::PERIOD_ID ep1 = DateDay::PERIOD_ID_A;    // end   period of first subrange
        int so2 = 0;                                      // start offset of second subrange
        int eo2 = 0;                                      // end   offset of second subrange
        DateDay::PERIOD_ID sp2 = DateDay::PERIOD_ID_none; // start period of second subrange
        DateDay::PERIOD_ID ep2 = DateDay::PERIOD_ID_none; // end   period of second subrange
        int f = 0;                                        // index (0=calendar, 1=financial)
        wxString name = "";                               // specification name

    public:
        Spec(
            int so1_new = 0, DateDay::PERIOD_ID sp1_new = DateDay::PERIOD_ID_A,
            int eo1_new = 0, DateDay::PERIOD_ID ep1_new = DateDay::PERIOD_ID_A,
            int so2_new = 0, DateDay::PERIOD_ID sp2_new = DateDay::PERIOD_ID_none,
            int eo2_new = 0, DateDay::PERIOD_ID ep2_new = DateDay::PERIOD_ID_none,
            int f_new = 0, wxString name_new = ""
        );

    public:
        bool parseLabel(StringIt &str_i, StringIt str_end);
        void parseName(StringIt &str_i, StringIt str_end);
        void setName(const wxString &name_new);
        bool parseSpec(const wxString &str, const wxString &name_new = "");
        bool hasPeriodS() const;
        const wxString getLabel() const;
        const wxString getName() const;
        const wxString getLabelName() const;
        const wxString checkingName() const;
        const wxString checkingDescription() const;

    private:
        static void scanWhiteSpace(StringIt &str_i, StringIt str_end);
        static char scanToken(StringIt &str_i, StringIt str_end, int &token_o, DateDay::PERIOD_ID &token_p);
        static const wxString offsetStr(int offset, bool show_zero = false);
        static const wxString offsetRangeStr(int so, int eo, bool show_zero = false);
    };

public:
    DateRange2(DateDay date_s_new = DateDay(), DateDay date_t_new = DateDay());

protected:
    int firstDay[2];                  // first day in PERIOD_ID_[YQM] (1..28)
    wxDateTime::Month firstMonth[2];  // first month in PERIOD_ID_[YQ] (0..11)
    wxDateTime::WeekDay firstWeekday; // first weekday in PERIOD_ID_W (0=Sun, 1=Mon)
    DateDay date_s;                   // the date of PERIOD_ID_S (MAY be undefined)
    DateDay date_t;                   // the date of PERIOD_ID_T (MUST be defined)
    Spec spec;                        // range specification

public:
    void setDateS(DateDay date = DateDay());
    void setDateT(DateDay date = DateDay());
    DateDay getDateS() const;
    DateDay getDateT() const;
    void setSpec(const Spec &spec_new);
    bool parseSpec(const wxString &str, const wxString &name = "");
    Spec getSpec() const;
    const wxString getLabel() const;
    const wxString getName() const;
    const wxString getLabelName() const;
    DateDay periodStart(DateDay date, DateDay::PERIOD_ID period) const;
    DateDay periodEnd(DateDay date, DateDay::PERIOD_ID period) const;
    DateDay checkingStart() const;
    DateDay checkingEnd() const;
    DateDay reportingStart() const;
    DateDay reportingEnd() const;
    const wxString checkingISOStart() const;
    const wxString checkingISOEnd() const;
    const wxString reportingISOStart() const;
    const wxString reportingISOEnd() const;
    const wxString checkingTooltip() const;
    const wxString reportingTooltip() const;

#ifndef NDEBUG
private:
    DateRange2(
        int firstDay_new_0, int firstDay_new_1,
        wxDateTime::Month firstMonth_new_0, wxDateTime::Month firstMonth_new_1,
        wxDateTime::WeekDay firstWeekday_new,
        DateDay date_t_new = DateDay(), DateDay date_s_new = DateDay()
    );
public:
    static bool debug();
#endif
};

inline void DateRange2::Spec::setName(const wxString &name_new)
{
    name = name_new;
}

inline bool DateRange2::Spec::hasPeriodS() const
{
    return
        sp1 == DateDay::PERIOD_ID_S || ep1 == DateDay::PERIOD_ID_S ||
        sp2 == DateDay::PERIOD_ID_S || ep2 == DateDay::PERIOD_ID_S;
}

inline const wxString DateRange2::Spec::offsetStr(int offset, bool show_zero)
{
    return (offset != 0) ? wxString::Format("%+d", offset) : show_zero ? "0" : "";
}

inline const wxString DateRange2::Spec::getName() const
{
    return name;
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

inline void DateRange2::setDateS(DateDay date)
{
    // date_s MAY be undefined (not applicable)
    date_s = date;
}

inline DateDay DateRange2::getDateT() const
{
    return date_t;
}

inline DateDay DateRange2::getDateS() const
{
    return date_s;
}

inline void DateRange2::setSpec(const DateRange2::Spec &spec_new)
{
    spec = spec_new;
}

inline DateRange2::Spec DateRange2::getSpec() const
{
    return spec;
}

inline const wxString DateRange2::getLabel() const
{
    return spec.getLabel();
}

inline const wxString DateRange2::getName() const
{
    return spec.getName();
}

inline const wxString DateRange2::getLabelName() const
{
    return spec.getLabelName();
}

inline const wxString DateRange2::checkingISOStart() const
{
    return checkingStart().getISOStart();
}

inline const wxString DateRange2::checkingISOEnd() const
{
    return checkingEnd().getISOEnd();
}

inline const wxString DateRange2::reportingISOStart() const
{
    return reportingStart().getISOStart();
}

inline const wxString DateRange2::reportingISOEnd() const
{
   return reportingEnd().getISOEnd();
}

