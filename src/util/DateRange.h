/*******************************************************
Copyright (C) 2006-2012     Lisheng Guan (guanlisheng@gmail.com)
Copyright (C) 2014 - 2022   Nikolay Akimov
Copyright (C) 2021          Mark Whalley (mark@ipx.co.uk)

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
#include <wx/datetime.h>

class DateRange
{
protected:
    const wxDateTime today_;
    const wxDateTime today_end_;
    const wxDateTime future_;
    wxDateTime start_date_, end_date_;
    int startDay_;
    wxString title_;
    bool futureIgnored_;

public:
    DateRange();
    virtual ~DateRange();
    void destroy();
    const virtual wxDateTime start_date() const;
    const virtual wxDateTime end_date() const;
    const wxDateTime future_date() const;
    const virtual wxDateTime today() const;
    virtual bool is_with_date() const;
    const virtual wxString title() const;
    const virtual wxString local_title() const;
    void virtual start_date(wxDateTime& start_date);
    void virtual end_date(wxDateTime& end_date);
    void set_end_date(wxDateTime v);
    void findEndOfMonth();
    void findBeginOfMonth();
    int startDay() const;
    bool isFutureIgnored() const;
    static const wxDateTime getDayEnd(wxDateTime dt)
    {
        return wxDateTime(dt.GetDay(), dt.GetMonth(), dt.GetYear(), 23, 59, 59, 999);
    };
};

class mmCurrentMonth: public DateRange
{
public:
    mmCurrentMonth();
};

class mmToday: public DateRange
{
public:
    mmToday();
};

class mmCurrentMonthToDate: public DateRange
{
public:
    mmCurrentMonthToDate();
};

class mmLastMonth: public DateRange
{
public:
    mmLastMonth();
};

class mmLastNDays : public DateRange
{
public:
    mmLastNDays(int days);
    void SetRange(int days);
};

class mmLast30Days: public DateRange
{
public:
    mmLast30Days();
};

class mmLast90Days: public DateRange
{
public:
    mmLast90Days();
};

class mmLast3Months: public DateRange
{
public:
    mmLast3Months();
};

class mmLast12Months: public DateRange
{
public:
    mmLast12Months();
};

class mmCurrentYear: public DateRange
{
public:
    mmCurrentYear();
};

class mmCurrentYearToDate: public DateRange
{
public:
    mmCurrentYearToDate();
};

class mmLastYear: public DateRange
{
public:
    mmLastYear();
};

class mmLastYearBefore: public DateRange
{
public:
    mmLastYearBefore();
};

class mmCurrentFinancialYear: public DateRange
{
public:
    mmCurrentFinancialYear();
};

class mmCurrentFinancialYearToDate : public DateRange
{
public:
    mmCurrentFinancialYearToDate();
};

class mmLastFinancialYear : public DateRange
{
public:
    mmLastFinancialYear();
};

class mmAllTime: public DateRange
{
public:
    mmAllTime();
    bool is_with_date() const;
};

class mmSpecifiedRange : public DateRange
{
public:
    mmSpecifiedRange(const wxDateTime& start, const wxDateTime& end);
};

class mmLast365Days: public DateRange
{
public:
    mmLast365Days();
};

class mmSinseToday : public DateRange
{
public:
    mmSinseToday();
};

class mmSinse30days : public DateRange
{
public:
    mmSinse30days();
};

class mmSinse90days : public DateRange
{
public:
    mmSinse90days();
};

class mmSinseCurrentYear : public DateRange
{
public:
    mmSinseCurrentYear();
};

class mmSinseCurrentFinancialYear : public mmCurrentFinancialYear
{
public:
    mmSinseCurrentFinancialYear();
};


inline const wxDateTime DateRange::start_date() const { return this->start_date_; }
inline const wxDateTime DateRange::end_date() const { return this->end_date_; }
inline const wxDateTime DateRange::future_date() const { return this->future_; }
inline const wxDateTime DateRange::today() const { return this->today_; }
inline bool DateRange::is_with_date() const { return true; }
inline const wxString DateRange::title() const { return title_; }
inline void DateRange::start_date(wxDateTime& start_date) { this->start_date_ = start_date; }
inline void DateRange::end_date(wxDateTime& end_date) { this->end_date_ = end_date; }
inline void DateRange::set_end_date(wxDateTime v) { end_date_ = v; }
inline int DateRange::startDay() const { return this->startDay_; }
inline bool DateRange::isFutureIgnored() const { return this->futureIgnored_; }


