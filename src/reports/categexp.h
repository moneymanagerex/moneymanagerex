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

#ifndef MM_EX_REPORTCATEGEXP_H_
#define MM_EX_REPORTCATEGEXP_H_

#include "reportbase.h"
#include "mmDateRange.h"
#include "util.h"
#include <vector>

class mmReportCategoryExpenses : public mmPrintableBase
{
public:
    mmReportCategoryExpenses(mmDateRange* date_range, const wxString& title, int type);
    virtual ~mmReportCategoryExpenses();

    virtual void RefreshData();
    virtual bool has_date_range();
    virtual wxString getHTMLText();

public:
    enum TYPE { NONE = 0, GOES, COME, CATEGORY};

protected:
    bool ignoreFutureDate_;
    wxString title_;
    int type_;

private:
    // structure for sorting of data
    struct data_holder { wxString color;  wxString name; double amount; int categs; };
    std::vector<data_holder> data_;
    std::vector<ValueTrio> valueList_;
};

class mmReportCategoryExpensesGoes: public mmReportCategoryExpenses
{
public:
    explicit mmReportCategoryExpensesGoes(mmDateRange* date_range = new mmAllTime()
    , const wxString& title = _("Where the Money Goes"));
};

class mmReportCategoryExpensesComes : public mmReportCategoryExpenses
{
public:
    explicit mmReportCategoryExpensesComes(mmDateRange* date_range = new mmAllTime()
    , const wxString& title = _("Where the Money Comes From"));
};

class mmReportCategoryExpensesCategories : public mmReportCategoryExpenses
{
public:
    explicit mmReportCategoryExpensesCategories(mmDateRange* date_range = new mmAllTime()
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

class mmReportCategoryExpensesCategoriesCurrentFinancialYearToDate : public mmReportCategoryExpensesCategories
{
public:
    mmReportCategoryExpensesCategoriesCurrentFinancialYearToDate(int day, int month);
};

#endif // MM_EX_REPORTCATEGEXP_H_
