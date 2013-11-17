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

#include "categexp.h"
#include "budget.h"

#include "htmlbuilder.h"
#include "util.h"
#include "mmOption.h"
#include "mmgraphpie.h"
#include <algorithm>
#include "model/Model_Category.h"

#define CATEGORY_SORT_BY_NAME        1
#define CATEGORY_SORT_BY_AMOUNT      2

mmReportCategoryExpenses::mmReportCategoryExpenses
( mmDateRange* date_range, const wxString& title, int type)
: mmPrintableBase(CATEGORY_SORT_BY_NAME)
, date_range_(date_range)
, title_(title)
, type_(type)
, ignoreFutureDate_(mmIniOptions::instance().ignoreFutureTransactions_)
{
}

mmReportCategoryExpenses::~mmReportCategoryExpenses()
{
    if(date_range_)
        delete date_range_;
}

wxString mmReportCategoryExpenses::getHTMLText()
{
    // structure for sorting of data
    struct data_holder {wxString name; double amount; int categs;} line;
    std::vector<data_holder> data;

    bool with_date = date_range_->is_with_date();
    double grandtotal = 0.0;

    std::vector<ValuePair> valueList;
    std::vector<ValuePair> valueListTotals;
    std::map<int, std::map<int, std::map<int, double> > > categoryStats;
    Model_Category::instance().getCategoryStats(categoryStats
        , date_range_
        , ignoreFutureDate_
        , false
        , with_date);

    for (const auto& category: Model_Category::instance().all(Model_Category::COL_CATEGNAME))
    {
        int categs = 0;
        double categtotal = 0.0;
        int categID = category.CATEGID;
        const wxString& sCategName = category.CATEGNAME;
        double amt = categoryStats[categID][-1][0];
        if (type_ == GOES && amt < 0.0) amt = 0;
        if (type_ == COME && amt > 0.0) amt = 0;

        categtotal += amt;
        grandtotal += amt;

        if (amt != 0)
        {
            ValuePair vp;
            vp.label = sCategName;
            vp.amount = amt;
            valueList.push_back(vp);

            line.name = sCategName;
            line.amount = amt;
            line.categs = 0;
            data.push_back(line);
        }

        Model_Subcategory::Data_Set subcategories = Model_Category::sub_category(category);
        std::stable_sort(subcategories.begin(), subcategories.end(), SorterBySUBCATEGNAME());
        for (const auto& sub_category : subcategories)
        {
            int subcategID = sub_category.SUBCATEGID;

            wxString sFullCategName = category.CATEGNAME + " : " + sub_category.SUBCATEGNAME;
            amt = categoryStats[categID][subcategID][0];

            if (type_ == GOES && amt < 0.0) amt = 0;
            if (type_ == COME && amt > 0.0) amt = 0;

            categtotal += amt;
            grandtotal += amt;

            if (amt != 0)
            {
                categs++;
                ValuePair vp;
                vp.label = sFullCategName;
                vp.amount = amt;
                valueList.push_back(vp);

                line.name = sFullCategName;
                line.amount = amt;
                line.categs = 0;
                data.push_back(line);
            }
        }

        if (categtotal != 0)
        {
            ValuePair vp_total;
            vp_total.label = category.CATEGNAME;
            vp_total.amount = categtotal;
            valueListTotals.push_back(vp_total);
        }

        if (categs > 1)
        {
            line.name = _("Category Total: ");
            line.amount = categtotal;
            line.categs = categs;
            data.push_back(line);
        }
        else if (categs > 0)
        {
            // Insert place holder to add a seperator
            line.name = "";
            line.amount = 0;
            line.categs = categs;
            data.push_back(line);
        }
    }

    // Data is presorted by name
    if (CATEGORY_SORT_BY_AMOUNT == sortColumn_)
    {
        std::stable_sort(data.begin(), data.end()
            , [] (const data_holder& x, const data_holder& y)
            {
                if (x.amount != y.amount) return x.amount < y.amount;
                else return x.name < y.name;
            }
        );
    }

    mmHTMLBuilder hb;
    hb.init();
    hb.addHeader(2, title_);

    hb.DisplayDateHeading(date_range_->start_date(), date_range_->end_date(), with_date);

    hb.startCenter();

    // Add the graph
    mmGraphPie ggtotal;
    hb.addImage(ggtotal.getOutputFileName());
    ggtotal.init(valueListTotals);
    ggtotal.Generate(wxEmptyString);
    mmGraphPie gg;
    hb.addImage(gg.getOutputFileName());
    gg.init(valueList);
    gg.Generate(wxEmptyString);

    hb.startTable("60%");
    hb.startTableRow();
    if(CATEGORY_SORT_BY_NAME == sortColumn_)
        hb.addTableHeaderCell(_("Category"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", CATEGORY_SORT_BY_NAME), _("Category"));
    if(CATEGORY_SORT_BY_AMOUNT == sortColumn_)
        hb.addTableHeaderCell(_("Amount"), true);
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", CATEGORY_SORT_BY_AMOUNT), _("Amount"), true);
    hb.endTableRow();

    bool endSeparator = false;
    for (const auto& entry : data)
    {
        endSeparator = false;
        if (entry.categs > 0)
        {
            if(CATEGORY_SORT_BY_NAME == sortColumn_)
            {
                if (entry.name != "")
                {
                    hb.addRowSeparator(0);
                    hb.startTableRow();
                    hb.addTableCell(entry.name, false, true, true, "GRAY");
                    hb.addMoneyCell(entry.amount, "GRAY");
                    hb.endTableRow();
                }
                hb.addRowSeparator(2);
                endSeparator = true;
            }
        }
        else
        {
            hb.startTableRow();
            hb.addTableCell(entry.name, false, true);
            hb.addMoneyCell(entry.amount);
            hb.endTableRow();
        }
    }

    if (!endSeparator)
        hb.addRowSeparator(2);
    hb.addTotalRow(_("Grand Total: "), 1, grandtotal);

    hb.endTable();
    hb.endCenter();
    hb.end();

    return hb.getHTMLText();
}

mmReportCategoryExpensesGoes::mmReportCategoryExpensesGoes
( mmDateRange* date_range, const wxString& title)
: mmReportCategoryExpenses(date_range, title, 2)
{}

mmReportCategoryExpensesGoesCurrentMonth::mmReportCategoryExpensesGoesCurrentMonth
( )
: mmReportCategoryExpensesGoes(new mmCurrentMonth()
, wxString::Format(_("Where the Money Goes - %s"), _("Current Month")))
{}

mmReportCategoryExpensesGoesCurrentMonthToDate::mmReportCategoryExpensesGoesCurrentMonthToDate
( )
: mmReportCategoryExpensesGoes(new mmCurrentMonthToDate()
, wxString::Format(_("Where the Money Goes - %s"), _("Current Month to Date")))
{}

mmReportCategoryExpensesGoesLastMonth::mmReportCategoryExpensesGoesLastMonth
( )
: mmReportCategoryExpensesGoes(new mmLastMonth()
, wxString::Format(_("Where the Money Goes - %s"), _("Last Month")))
{}

mmReportCategoryExpensesGoesLast30Days::mmReportCategoryExpensesGoesLast30Days
( )
: mmReportCategoryExpensesGoes(new mmLast30Days()
, wxString::Format(_("Where the Money Goes - %s"), _("Last 30 Days")))
{}

mmReportCategoryExpensesGoesLastYear::mmReportCategoryExpensesGoesLastYear
( )
: mmReportCategoryExpensesGoes(new mmLastYear()
,  wxString::Format(_("Where the Money Goes - %s"), _("Last Year")))
{}

mmReportCategoryExpensesGoesCurrentYear::mmReportCategoryExpensesGoesCurrentYear()
: mmReportCategoryExpensesGoes(new mmCurrentYear()
, wxString::Format(_("Where the Money Goes - %s"), _("Current Year")))
{}

mmReportCategoryExpensesGoesCurrentYearToDate::mmReportCategoryExpensesGoesCurrentYearToDate
( )
: mmReportCategoryExpensesGoes(new mmCurrentYearToDate()
, wxString::Format(_("Where the Money Goes - %s"), _("Current Year to Date")))
{}

mmReportCategoryExpensesGoesLastFinancialYear::mmReportCategoryExpensesGoesLastFinancialYear
( const int day, const int month)
: mmReportCategoryExpensesGoes(new mmLastFinancialYear(day, month)
, wxString::Format(_("Where the Money Goes - %s"), _("Last Financial Year")))
{}

mmReportCategoryExpensesGoesCurrentFinancialYear::mmReportCategoryExpensesGoesCurrentFinancialYear
( const int day, const int month)
: mmReportCategoryExpensesGoes(new mmCurrentFinancialYear(day, month)
, wxString::Format(_("Where the Money Goes - %s"), _("Current Financial Year")))
{}

mmReportCategoryExpensesComes::mmReportCategoryExpensesComes
(mmDateRange* date_range, const wxString& title)
: mmReportCategoryExpenses(date_range, title, 1)
{}

mmReportCategoryExpensesComesCurrentMonth::mmReportCategoryExpensesComesCurrentMonth
( )
: mmReportCategoryExpensesComes(new  mmCurrentMonth()
, wxString::Format(_("Where the Money Comes From - %s"), _("Current Month")))
{}

mmReportCategoryExpensesComesCurrentMonthToDate::mmReportCategoryExpensesComesCurrentMonthToDate
( )
: mmReportCategoryExpensesComes(new mmCurrentMonthToDate()
, wxString::Format(_("Where the Money Comes From - %s"), _("Current Month to Date")))
{}

mmReportCategoryExpensesComesLastMonth::mmReportCategoryExpensesComesLastMonth
( )
: mmReportCategoryExpensesComes(new mmLastMonth()
,  wxString::Format(_("Where the Money Comes From - %s"), _("Last Month")))
{}

mmReportCategoryExpensesComesLast30Days::mmReportCategoryExpensesComesLast30Days
( )
: mmReportCategoryExpensesComes(new mmLast30Days()
, wxString::Format(_("Where the Money Comes From - %s"), _("Last Month")))
{}

mmReportCategoryExpensesComesLastYear::mmReportCategoryExpensesComesLastYear
( )
: mmReportCategoryExpensesComes(new mmLastYear()
, wxString::Format(_("Where the Money Comes From - %s"), _("Last Year")))
{}

mmReportCategoryExpensesComesCurrentYear::mmReportCategoryExpensesComesCurrentYear
( )
: mmReportCategoryExpensesComes(new mmCurrentYear()
, wxString::Format(_("Where the Money Comes From - %s"), _("Current Year")))
{}

mmReportCategoryExpensesComesCurrentYearToDate::mmReportCategoryExpensesComesCurrentYearToDate
( )
: mmReportCategoryExpensesComes(new mmCurrentYearToDate()
, wxString::Format(_("Where the Money Comes From - %s"), _("Current Year to Date")))
{}

mmReportCategoryExpensesComesLastFinancialYear::mmReportCategoryExpensesComesLastFinancialYear
( int day, int month)
: mmReportCategoryExpensesComes(new mmLastFinancialYear(day, month)
, wxString::Format(_("Where the Money Comes From - %s"), _("Last Financial Year")))
{}

mmReportCategoryExpensesComesCurrentFinancialYear::mmReportCategoryExpensesComesCurrentFinancialYear
( int day, int month)
: mmReportCategoryExpensesComes(new mmCurrentFinancialYear(day, month)
, wxString::Format(_("Where the Money Comes From - %s"), _("Current Financial Year")))
{}

mmReportCategoryExpensesCategories::mmReportCategoryExpensesCategories
( mmDateRange* date_range, const wxString& title)
: mmReportCategoryExpenses(date_range, title, 0)
{}

mmReportCategoryExpensesCategoriesCurrentMonth::mmReportCategoryExpensesCategoriesCurrentMonth
( )
: mmReportCategoryExpensesCategories(new mmCurrentMonth()
, wxString::Format(_("Categories - %s"), _("Current Month")))
{}

mmReportCategoryExpensesCategoriesCurrentMonthToDate::mmReportCategoryExpensesCategoriesCurrentMonthToDate
( )
: mmReportCategoryExpensesCategories(new mmCurrentMonthToDate()
, wxString::Format(_("Categories - %s"), _("Current Month to Date")))
{}

mmReportCategoryExpensesCategoriesLastMonth::mmReportCategoryExpensesCategoriesLastMonth()
: mmReportCategoryExpensesCategories(new mmLastMonth()
, wxString::Format(_("Categories - %s"), _("Last Month")))
{}

mmReportCategoryExpensesCategoriesLast30Days::mmReportCategoryExpensesCategoriesLast30Days
( )
: mmReportCategoryExpensesCategories(new mmLast30Days()
, wxString::Format(_("Categories - %s"), _("Last 30 Days")))
{}

mmReportCategoryExpensesCategoriesLastYear::mmReportCategoryExpensesCategoriesLastYear()
: mmReportCategoryExpensesCategories(new mmLastYear()
, wxString::Format(_("Categories - %s"), _("Last Year")))
{}

mmReportCategoryExpensesCategoriesCurrentYear::mmReportCategoryExpensesCategoriesCurrentYear
( )
: mmReportCategoryExpensesCategories(new mmCurrentYear()
, wxString::Format(_("Categories - %s"), _("Current Year")))
{}

mmReportCategoryExpensesCategoriesCurrentYearToDate::mmReportCategoryExpensesCategoriesCurrentYearToDate
( )
: mmReportCategoryExpensesCategories(new mmCurrentYearToDate()
, wxString::Format(_("Categories - %s"), _("Current Year to Date")))
{}

mmReportCategoryExpensesCategoriesLastFinancialYear::mmReportCategoryExpensesCategoriesLastFinancialYear
( int day, int month)
: mmReportCategoryExpensesCategories(new mmLastFinancialYear(day, month)
, wxString::Format(_("Categories - %s"), _("Last Financial Year")))
{}

mmReportCategoryExpensesCategoriesCurrentFinancialYear::mmReportCategoryExpensesCategoriesCurrentFinancialYear
( int day, int month)
: mmReportCategoryExpensesCategories(new mmCurrentFinancialYear(day, month)
, wxString::Format(_("Categories - %s"), _("Last Financial Year")))
{}
