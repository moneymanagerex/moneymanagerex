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
    mmReportCategoryExpenses(mmCoreDB* core
                             , mmDateRange* date_range, const wxString& title, int type);

    wxString getHTMLText();

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
    mmReportCategoryExpensesGoes(mmCoreDB* core
                                 , mmDateRange* date_range = new mmAllTime()
    , const wxString& title = _("Where the Money Goes"));
};

class mmReportCategoryExpensesGoesCurrentMonth: public mmReportCategoryExpensesGoes
{
public:
    mmReportCategoryExpensesGoesCurrentMonth(mmCoreDB* core);
};

class mmReportCategoryExpensesGoesCurrentMonthToDate: public mmReportCategoryExpensesGoes
{
public:
    mmReportCategoryExpensesGoesCurrentMonthToDate(mmCoreDB* core);
};

class mmReportCategoryExpensesGoesLastMonth: public mmReportCategoryExpensesGoes
{
public:
    mmReportCategoryExpensesGoesLastMonth(mmCoreDB* core);
};

class mmReportCategoryExpensesGoesLast30Days: public mmReportCategoryExpensesGoes
{
public:
    mmReportCategoryExpensesGoesLast30Days(mmCoreDB* core);
};

class mmReportCategoryExpensesGoesLastYear: public mmReportCategoryExpensesGoes
{
public:
    mmReportCategoryExpensesGoesLastYear(mmCoreDB* core);
};

class mmReportCategoryExpensesGoesCurrentYear: public mmReportCategoryExpensesGoes
{
public:
    mmReportCategoryExpensesGoesCurrentYear(mmCoreDB* core);
};

class mmReportCategoryExpensesGoesCurrentYearToDate: public mmReportCategoryExpensesGoes
{
public:
    mmReportCategoryExpensesGoesCurrentYearToDate(mmCoreDB* core);
};

class mmReportCategoryExpensesGoesLastFinancialYear: public mmReportCategoryExpensesGoes
{
public:
    mmReportCategoryExpensesGoesLastFinancialYear(mmCoreDB* core
            , const int day, const int month);
};

class mmReportCategoryExpensesGoesCurrentFinancialYear: public mmReportCategoryExpensesGoes
{
public:
    mmReportCategoryExpensesGoesCurrentFinancialYear(mmCoreDB* core
            , const int day, const int month);
};

class mmReportCategoryExpensesComes: public mmReportCategoryExpenses
{
public:
    mmReportCategoryExpensesComes(mmCoreDB* core
                                  , mmDateRange* date_range = new mmAllTime()
    , const wxString& title = _("Where the Money Comes From"));
};

class mmReportCategoryExpensesComesCurrentMonth: public mmReportCategoryExpensesComes
{
public:
    mmReportCategoryExpensesComesCurrentMonth(mmCoreDB* core);
};

class mmReportCategoryExpensesComesCurrentMonthToDate: public mmReportCategoryExpensesComes
{
public:
    mmReportCategoryExpensesComesCurrentMonthToDate(mmCoreDB* core);
};

class mmReportCategoryExpensesComesLastMonth: public mmReportCategoryExpensesComes
{
public:
    mmReportCategoryExpensesComesLastMonth(mmCoreDB* core);
};

class mmReportCategoryExpensesComesLast30Days: public mmReportCategoryExpensesComes
{
public:
    mmReportCategoryExpensesComesLast30Days(mmCoreDB* core);
};

class mmReportCategoryExpensesComesLastYear: public mmReportCategoryExpensesComes
{
public:
    mmReportCategoryExpensesComesLastYear(mmCoreDB* core);
};

class mmReportCategoryExpensesComesCurrentYear: public mmReportCategoryExpensesComes
{
public:
    mmReportCategoryExpensesComesCurrentYear(mmCoreDB* core);
};

class mmReportCategoryExpensesComesCurrentYearToDate: public mmReportCategoryExpensesComes
{
public:
    mmReportCategoryExpensesComesCurrentYearToDate(mmCoreDB* core);
};

class mmReportCategoryExpensesComesLastFinancialYear: public mmReportCategoryExpensesComes
{
public:
    mmReportCategoryExpensesComesLastFinancialYear(mmCoreDB* core, int day, int month);
};

class mmReportCategoryExpensesComesCurrentFinancialYear: public mmReportCategoryExpensesComes
{
public:
    mmReportCategoryExpensesComesCurrentFinancialYear(mmCoreDB* core, int day, int month);
};

class mmReportCategoryExpensesCategories: public mmReportCategoryExpenses
{
public:
    mmReportCategoryExpensesCategories(mmCoreDB* core
                                       , mmDateRange* date_range = new mmAllTime()
    , const wxString& title = _("Categories"));
};

class mmReportCategoryExpensesCategoriesCurrentMonth: public mmReportCategoryExpensesCategories
{
public:
    mmReportCategoryExpensesCategoriesCurrentMonth(mmCoreDB* core);
};

class mmReportCategoryExpensesCategoriesCurrentMonthToDate: public mmReportCategoryExpensesCategories
{
public:
    mmReportCategoryExpensesCategoriesCurrentMonthToDate(mmCoreDB* core);
};

class mmReportCategoryExpensesCategoriesLastMonth: public mmReportCategoryExpensesCategories
{
public:
    mmReportCategoryExpensesCategoriesLastMonth(mmCoreDB* core);
};

class mmReportCategoryExpensesCategoriesLast30Days: public mmReportCategoryExpensesCategories
{
public:
    mmReportCategoryExpensesCategoriesLast30Days(mmCoreDB* core);
};

class mmReportCategoryExpensesCategoriesLastYear: public mmReportCategoryExpensesCategories
{
public:
    mmReportCategoryExpensesCategoriesLastYear(mmCoreDB* core);
};

class mmReportCategoryExpensesCategoriesCurrentYear: public mmReportCategoryExpensesCategories
{
public:
    mmReportCategoryExpensesCategoriesCurrentYear(mmCoreDB* core);
};

class mmReportCategoryExpensesCategoriesCurrentYearToDate: public mmReportCategoryExpensesCategories
{
public:
    mmReportCategoryExpensesCategoriesCurrentYearToDate(mmCoreDB* core);
};

class mmReportCategoryExpensesCategoriesLastFinancialYear: public mmReportCategoryExpensesCategories
{
public:
    mmReportCategoryExpensesCategoriesLastFinancialYear(mmCoreDB* core, int day, int month);
};

class mmReportCategoryExpensesCategoriesCurrentFinancialYear: public mmReportCategoryExpensesCategories
{
public:
    mmReportCategoryExpensesCategoriesCurrentFinancialYear(mmCoreDB* core, int day, int month);
};

#endif // _MM_EX_REPORTCATEGEXP_H_
