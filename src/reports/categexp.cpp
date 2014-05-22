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
#include "mmOption.h"
#include "mmgraphpie.h"
#include <algorithm>
#include "model/Model_Category.h"

#define CATEGORY_SORT_BY_NAME        1
#define CATEGORY_SORT_BY_AMOUNT      2

mmReportCategoryExpenses::mmReportCategoryExpenses
(mmDateRange* date_range, const wxString& title, int type)
: mmPrintableBase(CATEGORY_SORT_BY_NAME)
, date_range_(date_range)
, title_(title)
, type_(type)
, ignoreFutureDate_(mmIniOptions::instance().ignoreFutureTransactions_)
, with_date_(false)
{
    with_date_ = date_range_->is_with_date();
}

mmReportCategoryExpenses::~mmReportCategoryExpenses()
{
    if(date_range_)
        delete date_range_;
}

void  mmReportCategoryExpenses::RefreshData()
{
    data_.clear();
    std::map<int, std::map<int, std::map<int, double> > > categoryStats;
    Model_Category::instance().getCategoryStats(categoryStats
        , date_range_
        , ignoreFutureDate_
        , false
        , with_date_);

    for (const auto& entry : categoryStats)
    {
        wxString categName = "";
        Model_Category::Data *category = Model_Category::instance().get(entry.first);
        if (category) categName = category->CATEGNAME;

        double categ_total = entry.second.at(-1).at(0);
        for (const auto &subentry : entry.second)
        {
            double subcateg_total = subentry.second.at(0);
            if ((type_ == GOES && subcateg_total > 0.0) || (type_ == COME && subcateg_total < 0.0) || (type_ == NONE && subcateg_total != 0.0))
                valueList_.push_back({ Model_Category::full_name(entry.first, subentry.first), subcateg_total });
            categ_total += subcateg_total;
        }
        if ((type_ == GOES && categ_total > 0.0) || (type_ == COME && categ_total < 0.0) || (type_ == NONE && categ_total != 0.0))
            valueListTotals_.push_back({ categName, categ_total });
    }

    data_holder line;
    int groupID = 1;
    for (const auto& category : Model_Category::instance().all(Model_Category::COL_CATEGNAME))
    {
        const wxString& sCategName = category.CATEGNAME;
        double amt = categoryStats[category.CATEGID][-1][0];
        if (type_ == GOES && amt < 0.0) amt = 0;
        if (type_ == COME && amt > 0.0) amt = 0;
        if (amt != 0.0)
            data_.push_back({ sCategName, amt, groupID });

        auto subcategories = Model_Category::sub_category(category);
        std::stable_sort(subcategories.begin(), subcategories.end(), SorterBySUBCATEGNAME());
        for (const auto& sub_category : subcategories)
        {
            wxString sFullCategName = Model_Category::full_name(category.CATEGID, sub_category.SUBCATEGID);
            amt = categoryStats[category.CATEGID][sub_category.SUBCATEGID][0];
            if (type_ == GOES && amt < 0.0) amt = 0;
            if (type_ == COME && amt > 0.0) amt = 0;
            if (amt != 0.0)
                data_.push_back({ sFullCategName, amt, groupID });
        }
        groupID++;
    }
}

wxString mmReportCategoryExpenses::getHTMLText()
{
    // Data is presorted by name
    std::vector<data_holder> sortedData(data_);
    if (CATEGORY_SORT_BY_AMOUNT == sortColumn_)
    {
        std::stable_sort(sortedData.begin(), sortedData.end()
            , [] (const data_holder& x, const data_holder& y)
            {
                if (x.amount != y.amount) return x.amount < y.amount;
                else return x.name < y.name;
            }
        );
    }

    std::map <int, int> group_counter;
    std::map <int, double> group_total;
    for (const auto& entry : sortedData)
    {
        wxLogDebug("%s %s %s", entry.name, wxString() << entry.categs, wxString() << entry.amount);
        group_counter[entry.categs]++;
        group_total[entry.categs] += entry.amount;
        group_total[-1] += entry.amount < 0 ? entry.amount : 0;
        group_total[-2] += entry.amount > 0 ? entry.amount : 0;
    }

    mmHTMLBuilder hb;
    hb.init();
    hb.addHeader(2, title_);
    hb.DisplayDateHeading(date_range_->start_date(), date_range_->end_date(), with_date_);
    hb.startCenter();

    // Add the graph
    hb.startTable("100%");
    hb.startTableRow();
    hb.startTableCell("50%");
    mmGraphPie ggtotal;
    hb.addImage(ggtotal.getOutputFileName());
    ggtotal.init(valueListTotals_);
    ggtotal.Generate(_("Categories"));
    hb.endTableCell();
    hb.startTableCell("50%");
    mmGraphPie gg;
    hb.addImage(gg.getOutputFileName());
    gg.init(valueList_);
    gg.Generate(_("Subcategories"));
    hb.endTableCell();
    hb.endTableRow();
    hb.endTable();

    hb.startTable("60%");
    hb.startTableRow();
    if(CATEGORY_SORT_BY_NAME == sortColumn_)
        hb.addTableHeaderCell(_("Category"));
    else
        hb.addTableHeaderCellLink(wxString::Format("sort:%d", CATEGORY_SORT_BY_NAME), _("Category"));

    if(CATEGORY_SORT_BY_AMOUNT == sortColumn_)
        hb.addTableHeaderCell(_("Amount"), true);
    else
        hb.addTableHeaderCellLink(wxString::Format("sort:%d", CATEGORY_SORT_BY_AMOUNT), _("Amount"), true);

    hb.addTableHeaderCell(_("Total"), true);
    hb.endTableRow();

    int group = 1;
    for (const auto& entry : sortedData)
    {
        if (group != entry.categs)
        {
            if (group_counter[group] > 1)
            {
                hb.startTableRow();
                hb.addTableCell(_("Category Total: "), false, true, true, "GRAY");
                hb.addTableCell("");
                hb.addMoneyCell(group_total[group], "GRAY");
                hb.endTableRow();
            }
            hb.addRowSeparator(3);
        }
        group = entry.categs;
        hb.startTableRow();
        hb.addTableCell(entry.name, false, true);
        hb.addMoneyCell(entry.amount);
        if (group_counter[entry.categs] > 1)
            hb.addTableCell("");
        else
            hb.addMoneyCell(entry.amount);
        hb.endTableRow();
    }

    hb.addRowSeparator(3);
    if (type_ == NONE)
    {
        hb.addTotalRow(_("Total Expences: "), 3, group_total[-1]);
        hb.addTotalRow(_("Total Income: "), 3, group_total[-2]);
    }
    hb.addTotalRow(_("Grand Total: "), 3, group_total[-1] + group_total[-2]);

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

mmReportCategoryExpensesGoesCurrentFinancialYearToDate::mmReportCategoryExpensesGoesCurrentFinancialYearToDate
(const int day, const int month)
: mmReportCategoryExpensesGoes(new mmCurrentFinancialYearToDate(day, month)
, wxString::Format(_("Where the Money Goes - %s"), _("Current Financial Year to Date")))
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
, wxString::Format(_("Where the Money Comes From - %s"), _("Last 30 Days")))
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

mmReportCategoryExpensesComesCurrentFinancialYearToDate::mmReportCategoryExpensesComesCurrentFinancialYearToDate
(int day, int month)
: mmReportCategoryExpensesComes(new mmCurrentFinancialYearToDate(day, month)
, wxString::Format(_("Where the Money Comes From - %s"), _("Current Financial Year to Date")))
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
, wxString::Format(_("Categories - %s"), _("Current Financial Year")))
{}

mmReportCategoryExpensesCategoriesCurrentFinancialYearToDate::mmReportCategoryExpensesCategoriesCurrentFinancialYearToDate
(int day, int month)
: mmReportCategoryExpensesCategories(new mmCurrentFinancialYearToDate(day, month)
, wxString::Format(_("Categories - %s"), _("Current Financial Year to Date")))
{}
