/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#ifndef _MM_EX_REPORTCATEGEXP_H_
#define _MM_EX_REPORTCATEGEXP_H_

#include "reportbase.h"
#include "mmDateRange.h"

class mmReportCategoryExpenses : public mmPrintableBase
{
public:
    mmReportCategoryExpenses(mmDateRange* date_range, const wxString& title, int type);
    virtual ~mmReportCategoryExpenses();

    wxString getHTMLText();
    wxString version() { return "$Rev$"; }

public:
    enum TYPE { NONE = 0, GOES, COME, CATEGORY};

protected:
    mmDateRange* date_range_;
    bool ignoreFutureDate_;
    wxString title_;
    int type_;
};

class mmReportCategoryExpensesGoes: public mmReportCategoryExpenses
{
public:
    mmReportCategoryExpensesGoes(mmDateRange* date_range = new mmAllTime()
    , const wxString& title = _("Where the Money Goes"));
};

class mmReportCategoryExpensesGoesCurrentMonth: public mmReportCategoryExpensesGoes
{
public:
    mmReportCategoryExpensesGoesCurrentMonth();
};

class mmReportCategoryExpensesGoesCurrentMonthToDate: public mmReportCategoryExpensesGoes
{
public:
    mmReportCategoryExpensesGoesCurrentMonthToDate();
};

class mmReportCategoryExpensesGoesLastMonth: public mmReportCategoryExpensesGoes
{
public:
    mmReportCategoryExpensesGoesLastMonth();
};

class mmReportCategoryExpensesGoesLast30Days: public mmReportCategoryExpensesGoes
{
public:
    mmReportCategoryExpensesGoesLast30Days();
};

class mmReportCategoryExpensesGoesLastYear: public mmReportCategoryExpensesGoes
{
public:
    mmReportCategoryExpensesGoesLastYear();
};

class mmReportCategoryExpensesGoesCurrentYear: public mmReportCategoryExpensesGoes
{
public:
    mmReportCategoryExpensesGoesCurrentYear();
};

class mmReportCategoryExpensesGoesCurrentYearToDate: public mmReportCategoryExpensesGoes
{
public:
    mmReportCategoryExpensesGoesCurrentYearToDate();
};

class mmReportCategoryExpensesGoesLastFinancialYear: public mmReportCategoryExpensesGoes
{
public:
    mmReportCategoryExpensesGoesLastFinancialYear(const int day, const int month);
};

class mmReportCategoryExpensesGoesCurrentFinancialYear: public mmReportCategoryExpensesGoes
{
public:
    mmReportCategoryExpensesGoesCurrentFinancialYear(const int day, const int month);
};

class mmReportCategoryExpensesComes: public mmReportCategoryExpenses
{
public:
    mmReportCategoryExpensesComes(mmDateRange* date_range = new mmAllTime()
    , const wxString& title = _("Where the Money Comes From"));
};

class mmReportCategoryExpensesComesCurrentMonth: public mmReportCategoryExpensesComes
{
public:
    mmReportCategoryExpensesComesCurrentMonth();
};

class mmReportCategoryExpensesComesCurrentMonthToDate: public mmReportCategoryExpensesComes
{
public:
    mmReportCategoryExpensesComesCurrentMonthToDate();
};

class mmReportCategoryExpensesComesLastMonth: public mmReportCategoryExpensesComes
{
public:
    mmReportCategoryExpensesComesLastMonth();
};

class mmReportCategoryExpensesComesLast30Days: public mmReportCategoryExpensesComes
{
public:
    mmReportCategoryExpensesComesLast30Days();
};

class mmReportCategoryExpensesComesLastYear: public mmReportCategoryExpensesComes
{
public:
    mmReportCategoryExpensesComesLastYear();
};

class mmReportCategoryExpensesComesCurrentYear: public mmReportCategoryExpensesComes
{
public:
    mmReportCategoryExpensesComesCurrentYear();
};

class mmReportCategoryExpensesComesCurrentYearToDate: public mmReportCategoryExpensesComes
{
public:
    mmReportCategoryExpensesComesCurrentYearToDate();
};

class mmReportCategoryExpensesComesLastFinancialYear: public mmReportCategoryExpensesComes
{
public:
    mmReportCategoryExpensesComesLastFinancialYear(int day, int month);
};

class mmReportCategoryExpensesComesCurrentFinancialYear: public mmReportCategoryExpensesComes
{
public:
    mmReportCategoryExpensesComesCurrentFinancialYear(int day, int month);
};

class mmReportCategoryExpensesCategories: public mmReportCategoryExpenses
{
public:
    mmReportCategoryExpensesCategories(mmDateRange* date_range = new mmAllTime()
    , const wxString& title = _("Categories"));
};

class mmReportCategoryExpensesCategoriesCurrentMonth: public mmReportCategoryExpensesCategories
{
public:
    mmReportCategoryExpensesCategoriesCurrentMonth();
};

class mmReportCategoryExpensesCategoriesCurrentMonthToDate: public mmReportCategoryExpensesCategories
{
public:
    mmReportCategoryExpensesCategoriesCurrentMonthToDate();
};

class mmReportCategoryExpensesCategoriesLastMonth: public mmReportCategoryExpensesCategories
{
public:
    mmReportCategoryExpensesCategoriesLastMonth();
};

class mmReportCategoryExpensesCategoriesLast30Days: public mmReportCategoryExpensesCategories
{
public:
    mmReportCategoryExpensesCategoriesLast30Days();
};

class mmReportCategoryExpensesCategoriesLastYear: public mmReportCategoryExpensesCategories
{
public:
    mmReportCategoryExpensesCategoriesLastYear();
};

class mmReportCategoryExpensesCategoriesCurrentYear: public mmReportCategoryExpensesCategories
{
public:
    mmReportCategoryExpensesCategoriesCurrentYear();
};

class mmReportCategoryExpensesCategoriesCurrentYearToDate: public mmReportCategoryExpensesCategories
{
public:
    mmReportCategoryExpensesCategoriesCurrentYearToDate();
};

class mmReportCategoryExpensesCategoriesLastFinancialYear: public mmReportCategoryExpensesCategories
{
public:
    mmReportCategoryExpensesCategoriesLastFinancialYear(int day, int month);
};

class mmReportCategoryExpensesCategoriesCurrentFinancialYear: public mmReportCategoryExpensesCategories
{
public:
    mmReportCategoryExpensesCategoriesCurrentFinancialYear(int day, int month);
};

#endif // _MM_EX_REPORTCATEGEXP_H_
