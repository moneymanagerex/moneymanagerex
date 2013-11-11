/*******************************************************
 Copyright (C) 2006-2012

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

#ifndef _MM_EX_REPORTINCEXP_H_
#define _MM_EX_REPORTINCEXP_H_

#include "reportbase.h"
#include "mmDateRange.h"

class mmReportIncomeExpenses : public mmPrintableBaseSpecificAccounts
{
public:
    mmReportIncomeExpenses(mmDateRange* date_range = new mmCurrentMonth());
    virtual ~mmReportIncomeExpenses();
    virtual wxString getHTMLText();
    wxString version() { return "$Rev$"; }

protected:
    mmDateRange* date_range_;
    wxString title_;
    virtual wxString title() const;

};

class mmReportIncomeExpensesAllTime: public mmReportIncomeExpenses
{
public:
    mmReportIncomeExpensesAllTime(): mmReportIncomeExpenses(new mmAllTime())
    {
    }
};

class mmReportIncomeExpensesCurrentMonth: public mmReportIncomeExpenses
{
public:
    mmReportIncomeExpensesCurrentMonth(): mmReportIncomeExpenses(new mmCurrentMonth())
    {
    }
};

class mmReportIncomeExpensesCurrentMonthToDate: public mmReportIncomeExpenses
{
public:
    mmReportIncomeExpensesCurrentMonthToDate(): mmReportIncomeExpenses(new mmCurrentMonthToDate())
    {
    }
};

class mmReportIncomeExpensesLastMonth: public mmReportIncomeExpenses
{
public:
    mmReportIncomeExpensesLastMonth(): mmReportIncomeExpenses(new mmLastMonth())
    {
    }
};

class mmReportIncomeExpensesLast30Days: public mmReportIncomeExpenses
{
public:
    mmReportIncomeExpensesLast30Days(): mmReportIncomeExpenses(new mmLast30Days())
    {
    }
};

class mmReportIncomeExpensesSpecificAccounts : public mmReportIncomeExpenses
{
public:
    mmReportIncomeExpensesSpecificAccounts(mmDateRange* date_range = new mmCurrentMonth()) : mmReportIncomeExpenses(date_range)
    {
    }
    virtual wxString getHTMLText()
    {
        getSpecificAccounts();
        return mmReportIncomeExpenses::getHTMLText();
    }
};

class mmReportIncomeExpensesAllTimeSpecificAccounts : public mmReportIncomeExpensesSpecificAccounts
{
public:
    mmReportIncomeExpensesAllTimeSpecificAccounts() : mmReportIncomeExpensesSpecificAccounts(new mmAllTime())
    {
    }
};

class mmReportIncomeExpensesCurrentMonthSpecificAccounts : public mmReportIncomeExpensesSpecificAccounts
{
public:
    mmReportIncomeExpensesCurrentMonthSpecificAccounts() : mmReportIncomeExpensesSpecificAccounts(new mmCurrentMonth())
    {
    }
};

class mmReportIncomeExpensesCurrentMonthToDateSpecificAccounts : public mmReportIncomeExpensesSpecificAccounts
{
public:
    mmReportIncomeExpensesCurrentMonthToDateSpecificAccounts() : mmReportIncomeExpensesSpecificAccounts(new mmCurrentMonthToDate())
    {
    }
};

class mmReportIncomeExpensesLastMonthSpecificAccounts : public mmReportIncomeExpensesSpecificAccounts
{
public:
    mmReportIncomeExpensesLastMonthSpecificAccounts() : mmReportIncomeExpensesSpecificAccounts(new mmLastMonth())
    {
    }
};

class mmReportIncomeExpensesLast30DaysSpecificAccounts : public mmReportIncomeExpensesSpecificAccounts
{
public:
    mmReportIncomeExpensesLast30DaysSpecificAccounts() : mmReportIncomeExpensesSpecificAccounts(new mmLast30Days())
    {
    }
};

/////////////////////////////////////////////////////////////////////////////////////
class mmReportIncomeExpensesMonthly : public mmPrintableBaseSpecificAccounts
{
public:
    mmReportIncomeExpensesMonthly(int day_ = 1
                                         , int month_ = 1
                                                 , mmDateRange* date_range = new mmCurrentMonth());
    virtual ~mmReportIncomeExpensesMonthly();
    virtual wxString getHTMLText();
protected:

    mmDateRange* date_range_;
    int day_;
    int month_;
    wxString title_;
    virtual wxString title() const
    {
        return wxString::Format(title_, date_range_->title());
    }
};

class mmReportIncomeExpensesCurrentYear: public mmReportIncomeExpensesMonthly
{
public:
    mmReportIncomeExpensesCurrentYear(): mmReportIncomeExpensesMonthly(1, 1, new mmCurrentYear())
    {
    }
};

class mmReportIncomeExpensesLastYear: public mmReportIncomeExpensesMonthly
{
public:
    mmReportIncomeExpensesLastYear(): mmReportIncomeExpensesMonthly(1, 1, new mmLastYear())
    {
    }
};

class mmReportIncomeExpensesCurrentFinancialYear: public mmReportIncomeExpensesMonthly
{
public:
    mmReportIncomeExpensesCurrentFinancialYear(const int day, const int month)
        : mmReportIncomeExpensesMonthly(day, month, new mmCurrentFinancialYear(day, month))
    {
    }
};

class mmReportIncomeExpensesLastFinancialYear: public mmReportIncomeExpensesMonthly
{
public:
    mmReportIncomeExpensesLastFinancialYear(const int day, const int month)
        : mmReportIncomeExpensesMonthly(day, month, new mmLastFinancialYear(day, month))
    {
    }
};

class mmReportIncomeExpensesMonthlySpecificAccounts : public mmReportIncomeExpensesMonthly
{
public:
    mmReportIncomeExpensesMonthlySpecificAccounts(int day = 1, int month = 1
        , mmDateRange* date_range = new mmCurrentMonth()) : mmReportIncomeExpensesMonthly(day, month, date_range)
    {
    }
    virtual wxString getHTMLText()
    {
        getSpecificAccounts();
        return mmReportIncomeExpensesMonthly::getHTMLText();
    }
};

class mmReportIncomeExpensesCurrentYearSpecificAccounts : public mmReportIncomeExpensesMonthlySpecificAccounts
{
public:
    mmReportIncomeExpensesCurrentYearSpecificAccounts() : mmReportIncomeExpensesMonthlySpecificAccounts(1, 1, new mmCurrentYear())
    {
    }
};

class mmReportIncomeExpensesLastYearSpecificAccounts : public mmReportIncomeExpensesMonthlySpecificAccounts
{
public:
    mmReportIncomeExpensesLastYearSpecificAccounts() : mmReportIncomeExpensesMonthlySpecificAccounts(1, 1, new mmLastYear())
    {
    }
};

class mmReportIncomeExpensesCurrentFinancialYearSpecificAccounts : public mmReportIncomeExpensesMonthlySpecificAccounts
{
public:
    mmReportIncomeExpensesCurrentFinancialYearSpecificAccounts(const int day, const int month)
        : mmReportIncomeExpensesMonthlySpecificAccounts(day, month, new mmCurrentFinancialYear(day, month))
    {
    }
};

class mmReportIncomeExpensesLastFinancialYearSpecificAccounts : public mmReportIncomeExpensesMonthlySpecificAccounts
{
public:
    mmReportIncomeExpensesLastFinancialYearSpecificAccounts(const int day, const int month)
        : mmReportIncomeExpensesMonthlySpecificAccounts(day, month, new mmLastFinancialYear(day, month))
    {
    }
};

#endif // _MM_EX_REPORTINCEXP_H_

