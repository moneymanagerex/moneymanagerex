/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2017 James Higley

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
#include "reports/htmlbuilder.h"
#include "reports/mmDateRange.h"
#include "option.h"
#include <algorithm>
#include "Model_Category.h"

#define CATEGORY_SORT_BY_NAME        1
#define CATEGORY_SORT_BY_AMOUNT      2

mmReportCategoryExpenses::mmReportCategoryExpenses
(const wxString& title, enum TYPE type)
    : mmPrintableBase(title)
    , type_(type)
{
}

mmReportCategoryExpenses::~mmReportCategoryExpenses()
{
}

int mmReportCategoryExpenses::report_parameters()
{
    return RepParams::DATE_RANGE | RepParams::CHART;
}

void  mmReportCategoryExpenses::RefreshData()
{
    data_.clear();
    wxString color;
    std::map<int, std::map<int, std::map<int, double> > > categoryStats;
    Model_Category::instance().getCategoryStats(categoryStats
        , const_cast<mmDateRange*>(m_date_range)
        , Option::instance().IgnoreFutureTransactions()
        , false);

    data_holder line;
    int i = 0;
    mmHTMLBuilder hb;
    int groupID = 1;
    for (const auto& category : Model_Category::instance().all(Model_Category::COL_CATEGNAME))
    {
        const wxString& sCategName = category.CATEGNAME;
        double amt = categoryStats[category.CATEGID][-1][0];
        if (type_ == COME && amt < 0.0) amt = 0;
        if (type_ == GOES && amt > 0.0) amt = 0;
        if (amt != 0.0)
            data_.push_back({ hb.getColor(i++), sCategName, amt, groupID });

        auto subcategories = Model_Category::sub_category(category);
        std::stable_sort(subcategories.begin(), subcategories.end(), SorterBySUBCATEGNAME());
        for (const auto& sub_category : subcategories)
        {
            wxString sFullCategName = Model_Category::full_name(category.CATEGID, sub_category.SUBCATEGID);
            amt = categoryStats[category.CATEGID][sub_category.SUBCATEGID][0];
            if (type_ == COME && amt < 0.0) amt = 0;
            if (type_ == GOES && amt > 0.0) amt = 0;
            if (amt != 0.0)
                data_.push_back({ hb.getColor(i++), sFullCategName, amt, groupID });
        }
        groupID++;
    }
}

bool DataSorter(const ValueTrio& x, const ValueTrio& y)
{
    if (x.amount != y.amount)
        return fabs(x.amount) > fabs(y.amount);
    else
        return x.label < y.label;
}

wxString mmReportCategoryExpenses::getHTMLText()
{
    RefreshData();
    // Data is presorted by name
    std::vector<data_holder> sortedData(data_);

    std::vector<ValueTrio> expensesList, incomeList;
    std::map <int, int> group_counter;
    std::map <int, double> group_total;
    for (const auto& entry : sortedData)
    {
        group_counter[entry.categs]++;
        group_total[entry.categs] += entry.amount;
        group_total[-1] += entry.amount < 0 ? entry.amount : 0;
        group_total[-2] += entry.amount > 0 ? entry.amount : 0;
        if (getChartSelection() == 0)
        {
            if (entry.amount < 0)
                expensesList.push_back({ entry.color, entry.name, entry.amount });
            else if (entry.amount > 0)
                incomeList.push_back({ entry.color, entry.name, entry.amount });
        }
    }

    std::stable_sort(expensesList.begin(), expensesList.end(), DataSorter);
    std::stable_sort(incomeList.begin(), incomeList.end(), DataSorter);

    mmHTMLBuilder hb;
    hb.init();
    hb.addDivContainer();
    hb.addHeader(2, title());
    hb.addDateNow();
    hb.DisplayDateHeading(m_date_range->start_date(), m_date_range->end_date(), m_date_range->is_with_date());

    hb.addDivRow();
    hb.addDivCol17_67();
    // Add the graph
    if (getChartSelection() == 0)
    {
        if (type_ == CATEGORY)
        {
            hb.addDivCol17_67();
            hb.addText("<table><tr><th style='text-align: center'>");
            hb.addText(_("Expenses"));
            hb.addText("</th><th /><th style='text-align: center'>");
            hb.addText(_("Income"));
            hb.addText("</th></tr><tr><td>");
            if (!expensesList.empty())
                hb.addPieChart(expensesList, "Expenses");
            hb.addText("</td><td /><td>");
            if (!incomeList.empty())
                hb.addPieChart(incomeList, "Income");
            hb.addText("</td></tr></table>");
            hb.endDiv();
        }
        else
        {
            hb.addDivCol25_50();
            if (!expensesList.empty())
                hb.addPieChart(expensesList, "Expenses");
            if (!incomeList.empty())
                hb.addPieChart(incomeList, "Income");
            hb.endDiv();
        }
    }

    hb.startTable();
    hb.startThead();
    hb.startTableRow();
    if (getChartSelection() == 0) hb.addTableHeaderCell(" ");
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
        if (getChartSelection() == 0) hb.addColorMarker(entry.color);
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
            if (getChartSelection() == 0) hb.addTableCell("");
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

    int span = (getChartSelection() == 0) ? 4 : 3;
    hb.startTfoot();
    if (type_ == CATEGORY)
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

    return hb.getHTMLText();
}

mmReportCategoryExpensesGoes::mmReportCategoryExpensesGoes()
    : mmReportCategoryExpenses(_("Where the Money Goes"), TYPE::GOES)
{
}

mmReportCategoryExpensesComes::mmReportCategoryExpensesComes()
    : mmReportCategoryExpenses(_("Where the Money Comes From"), TYPE::COME)
{
}

mmReportCategoryExpensesCategories::mmReportCategoryExpensesCategories()
    : mmReportCategoryExpenses(_("Categories"), TYPE::CATEGORY)
{
    m_chart_selection = 1;
}

