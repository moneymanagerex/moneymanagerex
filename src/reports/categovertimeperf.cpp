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

//----------------------------------------------------------------------------
#include "categovertimeperf.h"
#include "reports/htmlbuilder.h"
#include "util.h"
#include "option.h"
#include "reports/mmDateRange.h"
#include "Model_Category.h"
#include <algorithm>
#include <map>

#define CATEGORY_SORT_BY_NAME       1
#define CATEGORY_SORT_BY_OVERALL    2
#define CATEGORY_SORT_BY_PERIOD     3 // must be last sort value

//----------------------------------------------------------------------------

mmReportCategoryOverTimePerformance::mmReportCategoryOverTimePerformance()
    : mmPrintableBase(_("Category Income/Expenses"))
{
    m_date_range = new mmLast12Months();
}
//----------------------------------------------------------------------------
mmReportCategoryOverTimePerformance::~mmReportCategoryOverTimePerformance()
{
    delete m_date_range;
}

wxString mmReportCategoryOverTimePerformance::getHTMLText()
{
    const int MONTHS_IN_PERIOD = 12; // including current month

    //Get statistic
    std::map<int, std::map<int, std::map<int, double> > > categoryStats;
    Model_Category::instance().getCategoryStats(categoryStats
        , const_cast<mmDateRange*>(m_date_range)
        , Option::instance().IgnoreFutureTransactions());

    //Init totals
    //Type(Withdrawal/Income/Summ), month, value
    std::map<int, std::map<int, double> > totals;

    // structure for sorting of data (month is used for sorting of period values and is not directly displayed)
    struct data_holder {wxString name; double period[MONTHS_IN_PERIOD]; double overall; double month;} line;
    std::vector<data_holder> data;
    for (const auto& category: Model_Category::instance().all(Model_Category::COL_CATEGNAME))
    {
        int categID = category.CATEGID;
        line.name = category.CATEGNAME;
        line.overall = 0;
        int month = 0;
        for (const auto &i : categoryStats[categID][-1])
        {
            double value = i.second;
            line.period[month++] = value;
            line.overall += value;
            totals[value<0][i.first] += value;
            totals[value>=0][i.first] += 0;
            totals[TOTAL][i.first] += value;
        }
        data.push_back(line);
        for (const auto& sub_category: Model_Category::sub_category(category))
        {
            int subcategID = sub_category.SUBCATEGID;
            line.name = category.CATEGNAME + " : " + sub_category.SUBCATEGNAME;
            line.overall = 0;
            month = 0;
            for (const auto &i : categoryStats[categID][subcategID])
            {
                double value = i.second;
                line.period[month++] = value;
                line.overall += value;
                totals[value<0][i.first] += value;
                totals[value>=0][i.first] += 0;
                totals[TOTAL][i.first] += value;
            }
            data.push_back(line);
        }
    }

    mmHTMLBuilder hb;
    hb.init();
    hb.addDivContainer();
    hb.addHeader(2, title());
    hb.addDateNow();
    hb.addLineBreak();

    hb.startSortTable();

    //Add header
    hb.startThead();
    hb.startTableRow();
    hb.addTableHeaderCell(_("Category"));
    wxDateTime start_date = m_date_range->start_date();
    for (int i = 0; i < MONTHS_IN_PERIOD; i++)
    {
        wxDateTime d = wxDateTime(start_date).Add(wxDateSpan::Months(i));
        hb.addTableHeaderCell(wxGetTranslation(wxDateTime::GetEnglishMonthName(d.GetMonth(), wxDateTime::Name_Abbr)) + wxString::Format("<br>%i", d.GetYear()), true);
    }
    hb.addTableHeaderCell(_("Overall"), true);
    hb.endTableRow();
    hb.endThead();
    
    hb.startTbody();
    //Begin of table
    for (const auto& entry : data)
    {
        if (entry.overall != 0.0) {
            hb.startTableRow();
            hb.addTableCell(entry.name);
            for (int i = 0; i < MONTHS_IN_PERIOD; i++)
                hb.addMoneyCell(entry.period[i]);
            hb.addMoneyCell(entry.overall);
            hb.endTableRow();
        }
    }
    hb.endTbody();

    //Totals
    hb.startTfoot();
    std::map<int, wxString> totalLabels;
    totalLabels[INCOME] = _("Incomes");
    totalLabels[EXPENSES] = _("Expenses");
    totalLabels[TOTAL] = _("Total");
    for (const auto& print_totals : totals)
    {
        hb.startTotalTableRow();
        hb.addTableCell(totalLabels[print_totals.first]);
        double overall = 0;
        for (const auto& range : totals[print_totals.first])
        {
            double amount = range.second;
            overall += amount;
            hb.addMoneyCell(amount);
        }
        hb.addMoneyCell(overall);
        hb.endTableRow();
    }
    hb.endTfoot();
    hb.endTable();
    hb.endDiv();
    hb.end();

    return hb.getHTMLText();
}
//----------------------------------------------------------------------------
