/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2017 James Higley
 Copyright (C) 2021, 2022 Mark Whalley (mark@ipx.co.uk)

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
#include "images_list.h"
#include "reports/htmlbuilder.h"
#include "reports/mmDateRange.h"
#include "option.h"
#include <algorithm>
#include "model/Model_Category.h"

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

double mmReportCategoryExpenses::AppendData(const std::vector<mmReportCategoryExpenses::data_holder> &data, std::map<int64, std::map<int, double>> &categoryStats, const DB_Table_CATEGORY_V1::Data* category, int64 groupID, int level) {
    double amt = categoryStats[category->CATEGID][0];
    if (type_ == COME && amt < 0.0) amt = 0;
    if (type_ == GOES && amt > 0.0) amt = 0;
    Model_Category::Data_Set subcategories = Model_Category::sub_category(category);
    std::stable_sort(subcategories.begin(), subcategories.end(), SorterByCATEGNAME());
    std::reverse(subcategories.begin(), subcategories.end());
    double subamount = 0;
    for (const auto& subcategory : subcategories) {
        double amount = AppendData(data_, categoryStats, &subcategory, groupID, level + 1);
        if (amount != 0) data_.insert(data_.begin(), { category->CATEGID, subcategory.CATEGID, category->CATEGNAME, amount, groupID, level });
        subamount += amount;
    }
    if (amt != 0 || subamount != 0) data_.insert(data_.begin(), { category->CATEGID, -1, category->CATEGNAME, amt, groupID, level });
    return amt + subamount;
}

void  mmReportCategoryExpenses::RefreshData()
{
    data_.clear();
    std::map<int64, std::map<int, double> > categoryStats;
    Model_Category::instance().getCategoryStats(categoryStats
        , accountArray_
        , const_cast<mmDateRange*>(m_date_range)
        , Option::instance().getIgnoreFutureTransactions()
        , false);

    data_holder line;
    int groupID = 0;
    Model_Category::Data_Set categories = Model_Category::instance().find(Model_Category::PARENTID(-1));
    std::stable_sort(categories.begin(), categories.end(), SorterByCATEGNAME());
    std::reverse(categories.begin(), categories.end());
    for (const auto& category : categories)
    {
        double amt = categoryStats[category.CATEGID][0];
        if (type_ == COME && amt < 0.0) amt = 0;
        if (type_ == GOES && amt > 0.0) amt = 0;
        
        auto subcategories = Model_Category::sub_category(category);
        std::stable_sort(subcategories.begin(), subcategories.end(), SorterByCATEGNAME());
        std::reverse(subcategories.begin(), subcategories.end());
        double subamount = 0;
        for (const auto& sub_category : subcategories)
        {
            double amount = AppendData(data_, categoryStats, &sub_category, category.CATEGID, 1);
            if (amount != 0) data_.insert(data_.begin(), { category.CATEGID, sub_category.CATEGID, category.CATEGNAME, amount, category.CATEGID, 0 });
            subamount += amount;
        }
        if (amt != 0 || subamount != 0) data_.insert(data_.begin(), { category.CATEGID, -1, category.CATEGNAME, amt, category.CATEGID, 0 });

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
    // Grab the data   
    RefreshData();

    // Data is presorted by name
    std::vector<data_holder> sortedData(data_);

    GraphData gdExpenses, gdIncome;
    GraphSeries gsExpenses, gsIncome;
    std::vector<std::pair<wxString, double>> expense_vector;
    std::vector<std::pair<wxString, double>> income_vector;
    std::map <int64, int> group_counter;
    std::map <int64, std::map<int64, double>> group_total;
    
    for (const auto& entry : sortedData)
    {
        if (entry.subCatID != -1 || entry.level == 0) group_counter[entry.catID] = 1;
        group_total[entry.categs][entry.catID] += entry.amount;
        if (entry.subCatID == -1) {
            group_total[-1][0] += entry.amount < 0 ? entry.amount : 0;
            group_total[-2][0] += entry.amount > 0 ? entry.amount : 0;

            if (getChartSelection() == 0)
            {
                if (entry.amount < 0)
                {
                    expense_vector.emplace_back(Model_Category::full_name(entry.catID), entry.amount);
                }
                else if (entry.amount > 0)
                {
                    income_vector.emplace_back(Model_Category::full_name(entry.catID), entry.amount);
                }
            }
        }
    }

    if (!expense_vector.empty())
    {
        std::sort(expense_vector.begin(), expense_vector.end());
        std::stable_sort(expense_vector.begin(), expense_vector.end(), [](const std::pair<wxString, double>& left, const std::pair<wxString, double>& right) {
            return left.second < right.second;});
        for (const auto& i : expense_vector)
        {
            gsExpenses.values.push_back({ i.second });
            gdExpenses.labels.push_back(i.first);
        }
        gsExpenses.name = _t("Expenses");
        gdExpenses.series.push_back(gsExpenses);
    }
    if (!income_vector.empty())
    {
        std::sort(income_vector.begin(), income_vector.end());
        std::stable_sort(income_vector.begin(), income_vector.end(), [](const std::pair<wxString, double>& left, const std::pair<wxString, double>& right) {
            return left.second > right.second; });
        for (const auto& i : income_vector)
        {
            gsIncome.values.push_back({ i.second });
            gdIncome.labels.push_back(i.first);
        }
        gsIncome.name = _t("Income");
        gdIncome.series.push_back(gsIncome);
    }

    // Build the report
    mmHTMLBuilder hb;
    hb.init();

    hb.addReportHeader(getReportTitle(), m_date_range->startDay(), m_date_range->isFutureIgnored());
    hb.DisplayDateHeading(m_date_range->start_date(), m_date_range->end_date(), m_date_range->is_with_date());
    hb.DisplayFooter(getAccountNames());
    // Prime the filter
    m_filter.clear();
    m_filter.setDateRange(m_date_range->start_date(), m_date_range->end_date());
    m_filter.setAccountList(accountArray_);

    // Chart
    if (getChartSelection() == 0)
    {
        if (!gdExpenses.series.empty())
        {

            gdExpenses.title = _t("Expenses");
            gdExpenses.type = GraphData::PIE;
            hb.addChart(gdExpenses);

        }
        if (!gdIncome.series.empty())
        {

            gdIncome.title = _t("Income");
            gdIncome.type = GraphData::PIE;
            hb.addChart(gdIncome);

        }
    }

    hb.addDivContainer("shadow"); // Table Container
    {
        hb.addDivContainer();
        hb.addText(wxString::Format("<button onclick=\"collapseAllToggles()\">%s</button>", _t("Collapse All")));
        hb.addText(wxString::Format("<button onclick=\"expandAllToggles()\">%s</button>", _t("Expand All")));
        //hb.addText(wxString::Format("<button onclick=\"expandAllToggles()\" style=\"float: right;\">%s</button>", _t("Export")));
        hb.endDiv();

        hb.startTable();
        {
            hb.startThead();
            {
                hb.startTableRow();
                {
                    hb.addTableHeaderCell(_t("Category"));
                    hb.addTableHeaderCell(_t("Amount"), "text-right");
                    hb.addTableHeaderCell(_t("Total"), "text-right");
                }
                hb.endTableRow();
            }
            hb.endThead();

            hb.startTbody();
            {
                int lastLevel = 0;
                std::map<int, wxString> levelParent;
                wxString row_id = "";
                wxString row_pid = "";
                for (const auto& entry : sortedData)
                {
                    if (entry.subCatID != -1) continue;

                    if (entry.level == 0) {
                        row_id = wxString::Format("%lld", entry.catID) + ".";
                        row_pid = "0.";
                    }
                    else if (entry.level > lastLevel) {
                        row_pid = row_id;
                        row_id = row_id + wxString::Format("%lld", entry.catID) + ".";
                    }
                    else if (entry.level == lastLevel)
                        row_id = row_pid + wxString::Format("%lld", entry.catID) + ".";
                    else {
                        row_id = levelParent[entry.level] + wxString::Format("%lld", entry.catID) + ".";
                        row_pid = levelParent[entry.level];
                    }

                    wxString indent = "";
                    for (int i = 0; i < entry.level; i++) {
                        indent = indent.Prepend("&nbsp;&nbsp;&nbsp;&nbsp;");
                    }

                    if(group_counter[entry.catID]){
                        hb.startTableRow("toggle' data-row-id='" + row_id + "' data-row-pid='" + row_pid);
                        hb.addTableCell(wxString::Format(indent + "<a>+&nbsp;%s</a>", entry.name));
                        hb.addEmptyTableCell();
                        hb.addMoneyCell(group_total[entry.categs][entry.catID]);
                        hb.endTableRow();
                    }
                    if (entry.amount != 0) {
                        if (group_counter[entry.catID]) {
                            indent.Append("&nbsp;&nbsp;&nbsp;&nbsp;");
                        }
                        hb.startTableRow("xtoggle' data-row-id='" + row_id + "' data-row-pid='" + (group_counter[entry.catID] ? row_id : row_pid));
                        hb.addTableCell(wxString::Format(indent +"<a href=\"viewtrans:%lld\" target=\"_blank\">%s</a>", entry.catID, entry.name));
                        hb.addMoneyCell(entry.amount);
                        hb.addEmptyTableCell();
                        hb.endTableRow();
                    }
                    levelParent[entry.level] = row_pid;
                    lastLevel = entry.level;
                }
            }
            hb.endTbody();

            int span = 3;
            hb.startTfoot();
            {
                if (type_ == SUMMARY)
                {
                    hb.addTotalRow(_t("Total Expenses:"), span, group_total[-1][0]);
                    hb.addTotalRow(_t("Total Income:"), span, group_total[-2][0]);
                }
                hb.addTotalRow(_t("Grand Total:"), span, group_total[-1][0] + group_total[-2][0]);
            }
            hb.endTfoot();
        }
        hb.endTable();
    }
    hb.endDiv();

    hb.end();

    wxLogDebug("======= mmReportCategoryExpenses:getHTMLText =======");
    wxLogDebug("%s", hb.getHTMLText());

    return hb.getHTMLText();
}

mmReportCategoryExpensesGoes::mmReportCategoryExpensesGoes()
    : mmReportCategoryExpenses(_n("Where the Money Goes"), TYPE::GOES)
{
    setReportParameters(Reports::WheretheMoneyGoes);
}

mmReportCategoryExpensesComes::mmReportCategoryExpensesComes()
    : mmReportCategoryExpenses(_n("Where the Money Comes From"), TYPE::COME)
{
    setReportParameters(Reports::WheretheMoneyComesFrom);
}

mmReportCategoryExpensesCategories::mmReportCategoryExpensesCategories()
    : mmReportCategoryExpenses(_n("Categories Summary"), TYPE::MONTHLY)
{
    m_chart_selection = 1;
    setReportParameters(Reports::CategoriesMonthly);
}

//----------------------------------------------------------------------------

mmReportCategoryOverTimePerformance::mmReportCategoryOverTimePerformance()
    : mmPrintableBase(_n("Category Income/Expenses"))
{
    m_date_range = new mmLast12Months();
    setReportParameters(Reports::CategoryOverTimePerformance);
}
//----------------------------------------------------------------------------
mmReportCategoryOverTimePerformance::~mmReportCategoryOverTimePerformance()
{
    delete m_date_range;
}

wxString mmReportCategoryOverTimePerformance::getHTMLText()
{
    // Grab the data
    const int MONTHS_IN_PERIOD = 12; // including current month

    wxDate sd = m_date_range->start_date();
    wxDate ed = m_date_range->end_date();
    sd.Add(wxDateSpan::Months(m_date_selection.GetValue()));
    ed.Add(wxDateSpan::Months(m_date_selection.GetValue()));
    mmDateRange* date_range = new mmSpecifiedRange(sd, ed);

    //Get statistic
    std::map<int64, std::map<int, double> > categoryStats;
    Model_Category::instance().getCategoryStats(categoryStats
        , accountArray_
        , date_range
        , Option::instance().getIgnoreFutureTransactions());

    //Init totals
    //Type(Withdrawal/Income/Summ), month, value
    std::map<int, std::map<int, double> > totals;

    // structure for sorting of data
    struct html_data_holder { int64 catID; int64 subCatID; wxString name; double period[MONTHS_IN_PERIOD]; double overall; } line;
    std::vector<html_data_holder> data;
    std::map<wxString, int64> categories = Model_Category::all_categories();
    for (const auto& category : categories)
    {
        int64 categID = category.second;
        line.catID = categID;
        line.subCatID = -1;
        line.name = category.first;
        line.overall = 0;
        unsigned month = 0;
        for (const auto& i : categoryStats[categID])
        {
            double value = i.second;
            line.period[month++] = value;
            line.overall += value;
            totals[value < 0][i.first] += value;
            totals[value >= 0][i.first] += 0;
            totals[TOTAL][i.first] += value;
        }
        data.push_back(line);
    }

    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    hb.addReportHeader(getReportTitle(), m_date_range->startDay(), m_date_range->isFutureIgnored());
    hb.DisplayDateHeading(sd, ed, true);
    hb.DisplayFooter(getAccountNames());
    // Prime the filter
    m_filter.clear();
    m_filter.setDateRange(sd, ed);
    m_filter.setAccountList(accountArray_);

    const wxDateTime start_date = date_range->start_date();
    delete date_range;

    //Chart
    wxArrayString labels;
    if (getChartSelection() == 0)
    {
        GraphData gd;
        GraphSeries data_negative, data_positive, data_difference;

        for (int i = 0; i < MONTHS_IN_PERIOD; i++)
        {
            wxDateTime d = start_date.Add(wxDateSpan::Months(i));

            double val_negative = 0;
            double val_positive = 0;
            for (const auto& entry : data)
            {
                if (entry.period[i] < 0)
                    val_negative += -entry.period[i];
                else
                    val_positive += entry.period[i];
            }

            data_negative.values.push_back(val_negative);
            data_positive.values.push_back(val_positive);
            data_difference.values.push_back(val_positive - val_negative);

            const auto label = wxString::Format("%s %i", wxGetTranslation(wxDateTime::GetEnglishMonthName(d.GetMonth())), d.GetYear());
            gd.labels.push_back(label);
        }

        data_difference.name = _t("Difference");
        data_negative.name = _t("Expenses");
        data_positive.name = _t("Income");

        data_difference.type = "line";
        data_positive.type = "column";
        data_negative.type = "column";

        gd.series.push_back(data_difference);
        gd.series.push_back(data_positive);
        gd.series.push_back(data_negative);

        if (!gd.series.empty())
        {
            gd.type = GraphData::BARLINE;
            gd.colors = { mmThemeMetaColour(meta::COLOR_REPORT_DELTA)
                            , mmThemeMetaColour(meta::COLOR_REPORT_CREDIT)
                            , mmThemeMetaColour(meta::COLOR_REPORT_DEBIT) };
            hb.addChart(gd);
        }
    }

    hb.addDivContainer("shadow");
    {
        hb.startSortTable();
        {

            //Add header
            hb.startThead();
            {
                hb.startTableRow();
                {
                    hb.addTableHeaderCell(_t("Category"));

                    for (int i = 0; i < MONTHS_IN_PERIOD; i++)
                    {
                        wxDateTime d = start_date.Add(wxDateSpan::Months(i));
                        hb.addTableHeaderCell(wxGetTranslation(wxDateTime::GetEnglishMonthName(d.GetMonth()
                            , wxDateTime::Name_Abbr)) + wxString::Format("<br>%i", d.GetYear()), "text-right");
                    }
                    hb.addTableHeaderCell(_t("Overall"), "text-right");
                }
                hb.endTableRow();
            }
            hb.endThead();

            hb.startTbody();
            {
                //Begin of table
                for (const auto& entry : data)
                {
                    if (entry.overall != 0.0)
                    {
                        hb.startTableRow();
                        {
                            hb.addTableCellLink(wxString::Format("viewtrans:%lld", entry.catID)
                                , entry.name);
                            for (int i = 0; i < MONTHS_IN_PERIOD; i++)
                                hb.addMoneyCell(entry.period[i]);
                            hb.addMoneyCell(entry.overall);
                        }
                        hb.endTableRow();
                    }
                }
            }
            hb.endTbody();

            //Totals
            hb.startTfoot();
            {
                std::map<int, wxString> totalLabels;
                totalLabels[INCOME] = _t("Incomes");
                totalLabels[EXPENSES] = _t("Expenses");
                totalLabels[TOTAL] = _t("Total");
                for (const auto& print_totals : totals)
                {
                    hb.startTotalTableRow();
                    {
                        hb.addTableCell(totalLabels[print_totals.first]);
                        double overall = 0;
                        for (const auto& range : totals[print_totals.first])
                        {
                            double amount = range.second;
                            overall += amount;
                            hb.addMoneyCell(amount);
                        }
                        hb.addMoneyCell(overall);
                    }
                    hb.endTableRow();
                }
            }
            hb.endTfoot();
        }
        hb.endTable();
    }
    hb.endDiv();

    hb.end();

    wxLogDebug("======= mmReportCategoryOverTimePerformance:getHTMLText =======");
    wxLogDebug("%s", hb.getHTMLText());

    return hb.getHTMLText();
}
