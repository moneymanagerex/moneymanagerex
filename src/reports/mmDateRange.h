/*******************************************************
Copyright (C) 2006-2012     Lisheng Guan (guanlisheng@gmail.com)

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
    wxDateTime start_date_, end_date_;
    wxString title_;

public:
    mmDateRange();
    virtual ~mmDateRange();
    void destroy();
    const virtual wxDateTime start_date() const;
    const virtual wxDateTime end_date() const;
    const virtual wxDateTime today() const;
    virtual bool is_with_date() const;
    const virtual wxString title() const;
    const virtual wxString local_title() const;
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

class mmCurrentFinancialYear: public mmDateRange
{
public:
    mmCurrentFinancialYear(const int day, const int month);
};

class mmCurrentFinancialYearToDate : public mmDateRange
{
public:
    mmCurrentFinancialYearToDate(const int day, const int month);
};

class mmLastFinancialYear : public mmDateRange
{
public:
    mmLastFinancialYear(const int day, const int month);
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

#endif // MM_EX_DATE_RANGE_H_
