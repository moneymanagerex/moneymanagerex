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

#ifndef MM_EX_DATE_RANGE_H_
#define MM_EX_DATE_RANGE_H_

#include <wx/datetime.h>

class mmDateRange
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
    mmDateRange();
    virtual ~mmDateRange();
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

class mmCurrentMonth: public mmDateRange
{
public:
    mmCurrentMonth();
};

class mmToday: public mmDateRange
{
public:
    mmToday();
};

class mmCurrentMonthToDate: public mmDateRange
{
public:
    mmCurrentMonthToDate();
};

class mmLastMonth: public mmDateRange
{
public:
    mmLastMonth();
};

class mmLastNDays : public mmDateRange
{
public:
    mmLastNDays(int days);
    void SetRange(int days);
};

class mmLast30Days: public mmDateRange
{
public:
    mmLast30Days();
};

class mmLast90Days: public mmDateRange
{
public:
    mmLast90Days();
};

class mmLast3Months: public mmDateRange
{
public:
    mmLast3Months();
};

class mmLast12Months: public mmDateRange
{
public:
    mmLast12Months();
};

class mmCurrentYear: public mmDateRange
{
public:
    mmCurrentYear();
};

class mmCurrentYearToDate: public mmDateRange
{
public:
    mmCurrentYearToDate();
};

class mmLastYear: public mmDateRange
{
public:
    mmLastYear();
};

class mmLastYearBefore: public mmDateRange
{
public:
    mmLastYearBefore();
};

class mmCurrentFinancialYear: public mmDateRange
{
public:
    mmCurrentFinancialYear();
};

class mmCurrentFinancialYearToDate : public mmDateRange
{
public:
    mmCurrentFinancialYearToDate();
};

class mmLastFinancialYear : public mmDateRange
{
public:
    mmLastFinancialYear();
};

class mmAllTime: public mmDateRange
{
public:
    mmAllTime();
    bool is_with_date() const;
};

class mmSpecifiedRange : public mmDateRange
{
public:
    mmSpecifiedRange(const wxDateTime& start, const wxDateTime& end);
};

class mmLast365Days: public mmDateRange
{
public:
    mmLast365Days();
};

class mmSinseToday : public mmDateRange
{
public:
    mmSinseToday();
};

class mmSinse30days : public mmDateRange
{
public:
    mmSinse30days();
};

class mmSinse90days : public mmDateRange
{
public:
    mmSinse90days();
};

class mmSinseCurrentYear : public mmDateRange
{
public:
    mmSinseCurrentYear();
};

class mmSinseCurrentFinancialYear : public mmCurrentFinancialYear
{
public:
    mmSinseCurrentFinancialYear();
};


inline const wxDateTime mmDateRange::start_date() const { return this->start_date_; }
inline const wxDateTime mmDateRange::end_date() const { return this->end_date_; }
inline const wxDateTime mmDateRange::future_date() const { return this->future_; }
inline const wxDateTime mmDateRange::today() const { return this->today_; }
inline bool mmDateRange::is_with_date() const { return true; }
inline const wxString mmDateRange::title() const { return title_; }
inline void mmDateRange::start_date(wxDateTime& start_date) { this->start_date_ = start_date; }
inline void mmDateRange::end_date(wxDateTime& end_date) { this->end_date_ = end_date; }
inline void mmDateRange::set_end_date(wxDateTime v) { end_date_ = v; }
inline int mmDateRange::startDay() const { return this->startDay_; }
inline bool mmDateRange::isFutureIgnored() const { return this->futureIgnored_; }


#endif // MM_EX_DATE_RANGE_H_
