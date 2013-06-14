/*
    Author: Lisheng Guan (guanlisheng@gmail.com)
*/
#ifndef _MM_EX_DATE_RANGE_H_
#define _MM_EX_DATE_RANGE_H_

#include <wx/datetime.h>

class mmDateRange
{
protected:
    const wxDateTime today_;
    wxDateTime start_date_, end_date_;
    wxString title_;

public:
    mmDateRange();
    const virtual wxDateTime start_date() const;
    const virtual wxDateTime end_date() const;
    const virtual wxDateTime today() const;
    const virtual bool is_with_date() const;
    const virtual wxString title() const;
};

class mmCurrentMonth: public mmDateRange
{
public:
    mmCurrentMonth();
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

class mmLastFinancialYear: public mmDateRange
{
public:
    mmLastFinancialYear(const int day, const int month);
};

class mmAllTime: public mmDateRange
{
public:
    mmAllTime();
    const bool is_with_date() const;
};

#endif // _MM_EX_DATE_RANGE_H_
