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

#ifndef MM_EX_REPORTPAYEE_H_
#define MM_EX_REPORTPAYEE_H_

#include "reportbase.h"
#include "mmDateRange.h"
#include "util.h"
#include <map>
#include <vector>

class mmReportPayeeExpenses : public mmPrintableBase
{
public:
    explicit mmReportPayeeExpenses(const wxString& title = _("Payee Report"), mmDateRange* date_range = new mmAllTime());
    virtual ~mmReportPayeeExpenses();

    virtual void RefreshData();
    virtual wxString getHTMLText();

protected:
    mmDateRange* date_range_;
    void getPayeeStats(std::map<int, std::pair<double, double> > &payeeStats
        , mmDateRange* date_range, bool ignoreFuture) const;

    wxString title_;
    enum TYPE {INCOME = 0, EXPENCES, MAX};

private:
    // structure for sorting of data
    struct data_holder { wxString name; double incomes; double expences; };
    std::vector<data_holder> data_;
    std::vector<ValuePair> valueList_;
    double positiveTotal_;
    double negativeTotal_;
};

class mmReportPayeeExpensesCurrentMonth: public mmReportPayeeExpenses
{
public:
    mmReportPayeeExpensesCurrentMonth(): mmReportPayeeExpenses(
        wxString::Format(_("Payee report - %s"), _("Current Month")), new mmCurrentMonth())
    {}
};

class mmReportPayeeExpensesCurrentMonthToDate: public mmReportPayeeExpenses
{
public:
    mmReportPayeeExpensesCurrentMonthToDate(): mmReportPayeeExpenses(
        wxString::Format(_("Payee report - %s"), _("Current Month to Date")), new mmCurrentMonthToDate())
    {}
};

class mmReportPayeeExpensesLastMonth: public mmReportPayeeExpenses
{
public:
    mmReportPayeeExpensesLastMonth(): mmReportPayeeExpenses(
        wxString::Format(_("Payee report - %s"), _("Last Month")), new mmLastMonth())
    {}
};

class mmReportPayeeExpensesLast30Days: public mmReportPayeeExpenses
{
public:
    mmReportPayeeExpensesLast30Days(): mmReportPayeeExpenses(
       wxString::Format(_("Payee report - %s"), _("Last 30 Days")), new mmLast30Days())
    {}
};

class mmReportPayeeExpensesLastYear: public mmReportPayeeExpenses
{
public:
    mmReportPayeeExpensesLastYear(): mmReportPayeeExpenses(
       wxString::Format(_("Payee report - %s"), _("Last Year")), new mmLastYear())
    {}
};

class mmReportPayeeExpensesCurrentYear: public mmReportPayeeExpenses
{
public:
    mmReportPayeeExpensesCurrentYear(): mmReportPayeeExpenses(
       wxString::Format(_("Payee report - %s"), _("Current Year")), new mmCurrentYear())
    {}
};

class mmReportPayeeExpensesCurrentYearToDate : public mmReportPayeeExpenses
{
public:
    mmReportPayeeExpensesCurrentYearToDate() : mmReportPayeeExpenses(
        wxString::Format(_("Payee report - %s"), _("Current Year to Date")), new mmCurrentYearToDate())
    {}
};

class mmReportPayeeExpensesLastFinancialYear : public mmReportPayeeExpenses
{
public:
    mmReportPayeeExpensesLastFinancialYear(int day, int month): mmReportPayeeExpenses(
       wxString::Format(_("Payee report - %s"), _("Last Financial Year")), new mmLastFinancialYear(day, month))
    {}
};

class mmReportPayeeExpensesCurrentFinancialYear: public mmReportPayeeExpenses
{
public:
    mmReportPayeeExpensesCurrentFinancialYear(int day, int month): mmReportPayeeExpenses(
        wxString::Format(_("Payee report - %s"), _("Current Financial Year")), new mmCurrentFinancialYear(day, month))
    {}
};

class mmReportPayeeExpensesCurrentFinancialYearToDate : public mmReportPayeeExpenses
{
public:
    mmReportPayeeExpensesCurrentFinancialYearToDate(int day, int month) : mmReportPayeeExpenses(
        wxString::Format(_("Payee report - %s"), _("Current Financial Year to Date")), new mmCurrentFinancialYearToDate(day, month))
    {}
};

#endif //MM_EX_REPORTPAYEE_H_
