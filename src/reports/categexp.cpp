/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2017 James Higley
 Copyright (C) 2021 Mark Whalley

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

void  mmReportCategoryExpenses::RefreshData()
{
    data_.clear();
    std::map<int, std::map<int, std::map<int, double> > > categoryStats;
    Model_Category::instance().getCategoryStats(categoryStats
        , accountArray_
        , const_cast<mmDateRange*>(m_date_range)
        , Option::instance().getIgnoreFutureTransactions()
        , false);

    data_holder line;
    int groupID = 1;
    for (const auto& category : Model_Category::instance().all(Model_Category::COL_CATEGNAME))
    {
        const wxString& sCategName = wxGetTranslation(category.CATEGNAME);
        double amt = categoryStats[category.CATEGID][-1][0];
        if (type_ == COME && amt < 0.0) amt = 0;
        if (type_ == GOES && amt > 0.0) amt = 0;
        if (amt != 0.0)
            data_.push_back({sCategName, amt, groupID });

        auto subcategories = Model_Category::sub_category(category);
        std::stable_sort(subcategories.begin(), subcategories.end(), SorterBySUBCATEGNAME());
        for (const auto& sub_category : subcategories)
        {
            wxString sFullCategName = Model_Category::full_name(category.CATEGID, sub_category.SUBCATEGID);
            amt = categoryStats[category.CATEGID][sub_category.SUBCATEGID][0];
            if (type_ == COME && amt < 0.0) amt = 0;
            if (type_ == GOES && amt > 0.0) amt = 0;
            if (amt != 0.0)
                data_.push_back({ sFullCategName, amt, groupID });
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
    // Grab the data   
    RefreshData();

    // Data is presorted by name
    std::vector<data_holder> sortedData(data_);

    GraphData gdExpenses, gdIncome;
    GraphSeries gsExpenses, gsIncome;

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
            {
                gsExpenses.values.push_back({ entry.amount });
                gdExpenses.labels.push_back(entry.name);
            } else if (entry.amount > 0)
            {
                gsIncome.values.push_back({ entry.amount });
                gdIncome.labels.push_back(entry.name);
            }
        }
    }
    if (!gsExpenses.values.empty())
    {
        gsExpenses.name = _("Expenses");
        gdExpenses.series.push_back(gsExpenses);
    }
    if (!gsIncome.values.empty())
    {
        gsIncome.name = _("Income");
        gdIncome.series.push_back(gsIncome);
    }

    // Build the report
    mmHTMLBuilder hb;
    hb.init();

    hb.addReportHeader(getReportTitle());
    hb.DisplayDateHeading(m_date_range->start_date(), m_date_range->end_date(), m_date_range->is_with_date());
    hb.DisplayFooter(getAccountNames());

    // Chart
    if (getChartSelection() == 0)
    {
        if (!gdExpenses.series.empty())
        {
            hb.addDivContainer("shadow"); 
            {
                gdExpenses.title = _("Expenses");
                gdExpenses.type = GraphData::PIE;
                hb.addChart(gdExpenses);
            }
            hb.endDiv();
        }
        if (!gdIncome.series.empty())
        {
            hb.addDivContainer("shadow");  
            {
                gdIncome.title = _("Income"); 
                gdIncome.type = GraphData::PIE;
                hb.addChart(gdIncome);
            }
            hb.endDiv();
        }
    }

    hb.addDivContainer("shadow"); // Table Container
    {
        hb.startTable();
        {
            hb.startThead();
            {
                hb.startTableRow();
                {
                    hb.addTableHeaderCell(_("Category"));
                    hb.addTableHeaderCell(_("Amount"), true);
                    hb.addTableHeaderCell(_("Total"), true);
                }
                hb.endTableRow();
            }
            hb.endThead();

            hb.startTbody();
            {
                int group = 0;
                for (const auto& entry : sortedData)
                {
                    group++;
                    hb.startTableRow();
                    {
                        hb.addTableCell(entry.name);
                        hb.addMoneyCell(entry.amount);
                        if (group_counter[entry.categs] > 1)
                            hb.addEmptyTableCell();
                        else
                            hb.addMoneyCell(entry.amount);
                    }
                    hb.endTableRow();

                    if (group_counter[entry.categs] == group && group_counter[entry.categs] > 1)
                    {
                        group = 0;
                        hb.startAltTableRow();
                        {
                            hb.addTableCell(_("Category Total: "));
                            hb.addEmptyTableCell();
                            hb.addMoneyCell(group_total[entry.categs]);
                        }
                        hb.endTableRow();
                    }
                    if (group_counter[entry.categs] == 1 || group == 0) 
                    {
                        group = 0;
                    }
                }
            }
            hb.endTbody();

            int span = 3;
            hb.startTfoot();
            {
                if (type_ == SUMMARY)
                {
                    hb.addTotalRow(_("Total Expenses:"), span, group_total[-1]);
                    hb.addTotalRow(_("Total Income:"), span, group_total[-2]);
                }
                hb.addTotalRow(_("Grand Total:"), span, group_total[-1] + group_total[-2]);
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
    : mmReportCategoryExpenses(wxTRANSLATE("Where the Money Goes"), TYPE::GOES)
{
    setReportParameters(Reports::WheretheMoneyGoes);
}

mmReportCategoryExpensesComes::mmReportCategoryExpensesComes()
    : mmReportCategoryExpenses(wxTRANSLATE("Where the Money Comes From"), TYPE::COME)
{
    setReportParameters(Reports::WheretheMoneyComesFrom);
}

mmReportCategoryExpensesCategories::mmReportCategoryExpensesCategories()
    : mmReportCategoryExpenses(wxTRANSLATE("Categories Summary"), TYPE::MONTHLY)
{
    m_chart_selection = 1;
    setReportParameters(Reports::CategoriesMonthly);
}

//----------------------------------------------------------------------------

mmReportCategoryOverTimePerformance::mmReportCategoryOverTimePerformance()
    : mmPrintableBase(wxTRANSLATE("Category Income/Expenses"))
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
    sd.Add(wxDateSpan::Months(m_date_selection));
    ed.Add(wxDateSpan::Months(m_date_selection));
    ed = ed.GetLastMonthDay();
    mmDateRange* date_range = new mmSpecifiedRange(sd, ed);

    //Get statistic
    std::map<int, std::map<int, std::map<int, double> > > categoryStats;
    Model_Category::instance().getCategoryStats(categoryStats
        , accountArray_
        , date_range
        , Option::instance().getIgnoreFutureTransactions());

    //Init totals
    //Type(Withdrawal/Income/Summ), month, value
    std::map<int, std::map<int, double> > totals;

    // structure for sorting of data
    struct html_data_holder { wxString name; double period[MONTHS_IN_PERIOD]; double overall; } line;
    std::vector<html_data_holder> data;
    for (const auto& category : Model_Category::instance().all(Model_Category::COL_CATEGNAME))
    {
        int categID = category.CATEGID;
        line.name = wxGetTranslation(category.CATEGNAME);
        line.overall = 0;
        unsigned month = 0;
        for (const auto &i : categoryStats[categID][-1])
        {
            double value = i.second;
            line.period[month++] = value;
            line.overall += value;
            totals[value < 0][i.first] += value;
            totals[value >= 0][i.first] += 0;
            totals[TOTAL][i.first] += value;
        }
        data.push_back(line);

        for (const auto& sub_category : Model_Category::sub_category(category))
        {
            int subcategID = sub_category.SUBCATEGID;
            line.name = Model_Category::full_name(category.CATEGID, sub_category.SUBCATEGID);
            line.overall = 0;
            month = 0;
            for (const auto &i : categoryStats[categID][subcategID])
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
    }

    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    hb.addReportHeader(getReportTitle());
    hb.DisplayDateHeading(sd, ed, true);
    hb.DisplayFooter(getAccountNames());

    const wxDateTime start_date = date_range->start_date();
    delete date_range;

    //Chart
    wxArrayString labels;
    if (getChartSelection() == 0)
    {
        GraphData gd;
        GraphSeries data_negative, data_positive;

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

            const auto label = wxString::Format("%s %i", wxGetTranslation(wxDateTime::GetEnglishMonthName(d.GetMonth())), d.GetYear());
            gd.labels.push_back(label);
        }

        data_negative.name = _("Expenses");
        data_positive.name = _("Income");
        gd.series.push_back(data_positive);
        gd.series.push_back(data_negative);

        if (!gd.series.empty())
        {
            hb.addDivContainer("shadow"); 
            {                 
                gd.type = GraphData::BAR;
                gd.colors = { mmThemeMetaColour(meta::COLOR_REPORT_CREDIT)
                                , mmThemeMetaColour(meta::COLOR_REPORT_DEBIT) }; 
                hb.addChart(gd);
            }
            hb.endDiv();
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
                    hb.addTableHeaderCell(_("Category"));

                    for (int i = 0; i < MONTHS_IN_PERIOD; i++)
                    {
                        wxDateTime d = start_date.Add(wxDateSpan::Months(i));
                        hb.addTableHeaderCell(wxGetTranslation(wxDateTime::GetEnglishMonthName(d.GetMonth()
                            , wxDateTime::Name_Abbr)) + wxString::Format("<br>%i", d.GetYear()), true);
                    }
                    hb.addTableHeaderCell(_("Overall"), true);
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
                            hb.addTableCell(entry.name);
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
                totalLabels[INCOME] = _("Incomes");
                totalLabels[EXPENSES] = _("Expenses");
                totalLabels[TOTAL] = _("Total");
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
