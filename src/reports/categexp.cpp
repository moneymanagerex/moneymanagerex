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

#include "htmlbuilder.h"
#include "mmOption.h"
#include <algorithm>
#include "model/Model_Category.h"

#define CATEGORY_SORT_BY_NAME        1
#define CATEGORY_SORT_BY_AMOUNT      2

mmReportCategoryExpenses::mmReportCategoryExpenses
(const wxString& title, int type)
: mmPrintableBase("mmReportCategoryExpenses")
, title_(title)
, type_(type)
{
}

mmReportCategoryExpenses::~mmReportCategoryExpenses()
{
}

bool mmReportCategoryExpenses::has_date_range()
{
    return true;
}

void  mmReportCategoryExpenses::RefreshData()
{
    data_.clear();
    wxString color;
    std::map<int, std::map<int, std::map<int, double> > > categoryStats;
    Model_Category::instance().getCategoryStats(categoryStats
        , const_cast<mmDateRange*>(m_date_range)
        , mmIniOptions::instance().ignoreFutureTransactions_
        , false
        , m_date_range->is_with_date());

    data_holder line;
    int i = 0;
    mmHTMLBuilder hb;
    int groupID = 1;
    for (const auto& category : Model_Category::instance().all(Model_Category::COL_CATEGNAME))
    {
        const wxString& sCategName = category.CATEGNAME;
        double amt = categoryStats[category.CATEGID][-1][0];
        if (type_ == GOES && amt < 0.0) amt = 0;
        if (type_ == COME && amt > 0.0) amt = 0;
        if (amt != 0.0)
            data_.push_back({ hb.getColor(i++), sCategName, amt, groupID });

        auto subcategories = Model_Category::sub_category(category);
        std::stable_sort(subcategories.begin(), subcategories.end(), SorterBySUBCATEGNAME());
        for (const auto& sub_category : subcategories)
        {
            wxString sFullCategName = Model_Category::full_name(category.CATEGID, sub_category.SUBCATEGID);
            amt = categoryStats[category.CATEGID][sub_category.SUBCATEGID][0];
            if (type_ == GOES && amt < 0.0) amt = 0;
            if (type_ == COME && amt > 0.0) amt = 0;
            if (amt != 0.0)
                data_.push_back({ hb.getColor(i++), sFullCategName, amt, groupID });
        }
        groupID++;
    }
}

wxString mmReportCategoryExpenses::getHTMLText()
{
    RefreshData();
    valueList_.clear();
    // Data is presorted by name
    std::vector<data_holder> sortedData(data_);

    std::map <int, int> group_counter;
    std::map <int, double> group_total;
    for (const auto& entry : sortedData)
    {
        group_counter[entry.categs]++;
        group_total[entry.categs] += entry.amount;
        group_total[-1] += entry.amount < 0 ? entry.amount : 0;
        group_total[-2] += entry.amount > 0 ? entry.amount : 0;
        if (type_ != NONE) valueList_.push_back({ entry.color, entry.name, entry.amount });
    }

    std::stable_sort(valueList_.begin(), valueList_.end()
        , [](const ValueTrio& x, const ValueTrio& y)
        {
        if (x.amount != y.amount)
                return fabs(x.amount) > fabs(y.amount);
            else
                return x.label < y.label;
        }
    );

    mmHTMLBuilder hb;
    hb.init();
    hb.addDivContainer();
    hb.addHeader(2, title_);
    hb.DisplayDateHeading(m_date_range->start_date(), m_date_range->end_date(), m_date_range->is_with_date());

    hb.addDivRow();
    hb.addDivCol17_67();
    // Add the graph
    hb.addDivCol25_50();
    if (type_ != NONE && !valueList_.empty())
        hb.addPieChart(valueList_, "Categories");
    hb.endDiv();

    hb.startTable();
    hb.startThead();
    hb.startTableRow();
    if (type_ != NONE) hb.addTableHeaderCell(" ");
    hb.addTableHeaderCell(_("Category"));
    hb.addTableHeaderCell(_("Amount"), true);
    hb.addTableHeaderCell(_("Total"), true);
    hb.endTableRow();
    hb.endThead();

    hb.startTbody();
    int group = 0;
    for (const auto& entry : sortedData)
    {
        group++;
        hb.startTableRow();
        if (type_ != NONE) hb.addColorMarker(entry.color);
        hb.addTableCell(entry.name);
        hb.addMoneyCell(entry.amount);
        if (group_counter[entry.categs] > 1)
            hb.addTableCell("");
        else
            hb.addMoneyCell(entry.amount);
        hb.endTableRow();

        if (group_counter[entry.categs] == group && group_counter[entry.categs] > 1)
        {
            group = 0;
            hb.startTableRow();
            if (type_ != NONE) hb.addTableCell("");
            hb.addTableCell(_("Category Total: "));
            hb.addTableCell("");
            hb.addMoneyCell(group_total[entry.categs]);
            hb.endTableRow();
        }
        if (group_counter[entry.categs] == 1 || group == 0) {
            group = 0;
        }
    }
    hb.endTbody();

    int span = (type_ != NONE) ? 4 : 3;
    hb.startTfoot();
    if (type_ == NONE)
    {
        hb.addTotalRow(_("Total Expenses:"), span, group_total[-1]);
        hb.addTotalRow(_("Total Income:"), span, group_total[-2]);
    }
    hb.addTotalRow(_("Grand Total:"), span, group_total[-1] + group_total[-2]);
    hb.endTfoot();

    hb.endTable();
    hb.endDiv();
    hb.endDiv();
    hb.endDiv();
    hb.end();

    Model_Report::outputReportFile(hb.getHTMLText());
    return "";
}

mmReportCategoryExpensesGoes::mmReportCategoryExpensesGoes
(const wxString& title)
: mmReportCategoryExpenses(title, 2)
{}

mmReportCategoryExpensesComes::mmReportCategoryExpensesComes
(const wxString& title)
: mmReportCategoryExpenses(title, 1)
{}

mmReportCategoryExpensesCategories::mmReportCategoryExpensesCategories
(const wxString& title)
: mmReportCategoryExpenses(title, 3)
{}

