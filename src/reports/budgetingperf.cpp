/*******************************************************
Copyright (C) 2006-2012 Nikolay Akimov
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

#include "budgetingperf.h"
#include "reports/htmlbuilder.h"
#include "mmex.h"
#include "mmframe.h"
#include "Model_Budgetyear.h"
#include "Model_Budget.h"
#include "Model_Category.h"
#include "Model_Subcategory.h"
#include "reports/mmDateRange.h"

mmReportBudgetingPerformance::mmReportBudgetingPerformance()
{
}

mmReportBudgetingPerformance::~mmReportBudgetingPerformance()
{}

void mmReportBudgetingPerformance::DisplayRow(mmHTMLBuilder &hb
    , double estimated, double actual, const wxString& catName
    , const std::map<int, double>& stats, bool bTotalRow)
{
    if ((estimated != 0.0) || (actual != 0.0))
    {
        if (bTotalRow)
            hb.startTotalTableRow();
        else
            hb.startTableRow();
        hb.addTableCell(catName);

        const double est = estimated / 12.0;
        for (const auto &i : stats)
        {
            const auto estVal = Model_Currency::toString(est, Model_Currency::GetBaseCurrency());
            hb.startTableCell(" style='text-align:right;' nowrap");
            hb.addText(estVal);
            hb.endTableCell();

            const auto val = Model_Currency::toString(i.second, Model_Currency::GetBaseCurrency());
            hb.startTableCell(wxString::Format(" style='text-align:right;%s' nowrap"
                , (i.second - est < 0) ? "color:#FF0000;" : ""));
            hb.addText(val);
            hb.endTableCell();
        }

        // year end
        const auto estVal = Model_Currency::toString(estimated, Model_Currency::GetBaseCurrency());
        hb.startTableCell(" style='text-align:right;' nowrap");
        hb.addText(estVal);
        hb.endTableCell();

        const auto val = Model_Currency::toString(actual, Model_Currency::GetBaseCurrency());
        hb.startTableCell(wxString::Format(" style='text-align:right;%s' nowrap"
            , (actual - estimated < 0) ? "color:#FF0000;" : "color:#009900;"));
        hb.addText(val);
        hb.endTableCell();

        if (((estimated < 0) && (actual < 0)) ||
            ((estimated > 0) && (actual > 0)))
        {
            double percent = (fabs(actual) / fabs(estimated)) * 100.0;
            hb.addTableCell(wxString::Format("%.0f", percent));
        }
        else
        {
            hb.addTableCell("-");
        }
        hb.endTableRow();
    }
}

int mmReportBudgetingPerformance::report_parameters()
{
    return RepParams::ONLY_YEARS;
}

wxString mmReportBudgetingPerformance::getHTMLText()
{
    unsigned short startDay = 1;
    wxDate::Month startMonth = wxDateTime::Jan;
    long startYear;

    wxString value = Model_Budgetyear::instance().Get(m_date_selection);
    wxString budget_year = value;

    wxRegEx pattern("^([0-9]{4})$");
    if (pattern.Matches(value))
        budget_year = pattern.GetMatch(value, 1);

    if (!budget_year.ToLong(&startYear)) {
        startYear = static_cast<long>(wxDateTime::Today().GetYear());
        budget_year = wxString::Format("%ld", startYear);
    }

    wxDateTime yearBegin(startDay, startMonth
        , static_cast<int>(startYear));
    wxDateTime yearEnd = yearBegin;
    yearEnd.Add(wxDateSpan::Year()).Subtract(wxDateSpan::Day());

    // Readjust dates by the Budget Offset Option
    Option::instance().BudgetDateOffset(yearBegin);
    Option::instance().BudgetDateOffset(yearEnd);
    mmSpecifiedRange date_range(yearBegin, yearEnd);

    bool evaluateTransfer = false;
    if (Option::instance().BudgetIncludeTransfers())
    {
        evaluateTransfer = true;
    }
    //Get statistics
    std::map<int, std::map<int, Model_Budget::PERIOD_ENUM> > budgetPeriod;
    std::map<int, std::map<int, double> > budgetAmt;
    Model_Budget::instance().getBudgetEntry(m_date_selection, budgetPeriod, budgetAmt);
    std::map<int, std::map<int, std::map<int, double> > > categoryStats;
    Model_Category::instance().getCategoryStats(categoryStats
        , &date_range, Option::instance().IgnoreFutureTransactions(),
        true, (evaluateTransfer ? &budgetAmt : nullptr));
    //Init totals
    const auto &allCategories = Model_Category::instance().all(Model_Category::COL_CATEGNAME);
    const auto &allSubcategories = Model_Subcategory::instance().all(Model_Subcategory::COL_SUBCATEGNAME);
    std::map<int, std::map<int, double> > totals;
    double monthlyEst = 0, monthlyAct = 0;
    std::map<int, double> monthlyActual;
    for (const auto& category : allCategories)
    {
        totals[category.CATEGID][-1] = 0;
        for (const auto &subcategory : allSubcategories)
        {
            if (subcategory.CATEGID == category.CATEGID)
                totals[category.CATEGID][subcategory.SUBCATEGID] = 0;
        }
    }

    for (const auto& category : allCategories)
    {
        for (const auto &i : categoryStats[category.CATEGID][-1])
        {
            totals[category.CATEGID][-1] += categoryStats[category.CATEGID][-1][i.first];
            for (const Model_Subcategory::Data& subcategory : allSubcategories)
            {
                if (subcategory.CATEGID == category.CATEGID)
                    totals[category.CATEGID][subcategory.SUBCATEGID] 
                    += categoryStats[category.CATEGID][subcategory.SUBCATEGID][i.first];
            }
        }
    }

    const wxString& headingStr = AdjustYearValues(static_cast<int>(startDay)
        , startMonth, startYear, budget_year);
    mmHTMLBuilder hb;
    hb.init();
    hb.addDivContainer();
    hb.addHeader(2, wxString::Format(_("Budget Performance for %s"), headingStr));
    hb.addDateNow();
    hb.DisplayDateHeading(yearBegin, yearEnd);

    hb.startTable();
    hb.startThead();
    hb.startTableRow();
    hb.addTableHeaderCell(_("Category"));

    for (int yidx = 0; yidx < 12; yidx++)
    {
        int m = startMonth + yidx;
        if (m >= 12) m -= 12;
        hb.addTableHeaderCell(wxGetTranslation(wxDateTime::GetEnglishMonthName(wxDateTime::Month(m)
            , wxDateTime::Name_Abbr)), false, true, 2, true);
    }
    hb.addTableHeaderCell(_("Total"), false, true, 3, true);
    hb.endTableRow();
    hb.endThead();
    hb.startTableRow();
    hb.addEmptyTableCell();
    for (int yidx = 0; yidx < 12; yidx++)
    {
        hb.addTableCell(_("Est."), false, true);
        hb.addTableCell(_("Act."), false, true);
    }
    hb.addTableCell(_("Est."), false, true);
    hb.addTableCell(_("Act."), false, true);
    hb.addTableCell(_("%"), false, true);
    hb.endTableRow();

    hb.startTbody();
    for (const Model_Category::Data& category : allCategories)
    {
        // Set the estimated amount for the year
        double estimated = Model_Budget::getYearlyEstimate(budgetPeriod[category.CATEGID][-1]
            , budgetAmt[category.CATEGID][-1]);

        // set the actual amount for the year
        double actual = totals[category.CATEGID][-1];

        monthlyEst += estimated;
        monthlyAct += actual;
        for (const auto &i : categoryStats[category.CATEGID][-1])
            monthlyActual[i.first] += i.second;

        DisplayRow(hb, estimated, actual, category.CATEGNAME, categoryStats[category.CATEGID][-1]);

        for (const Model_Subcategory::Data& subcategory : allSubcategories)
        {
            if (subcategory.CATEGID != category.CATEGID) continue;

            // Set the estimated amount for the year
            estimated = Model_Budget::getYearlyEstimate(budgetPeriod[category.CATEGID][subcategory.SUBCATEGID]
                , budgetAmt[category.CATEGID][subcategory.SUBCATEGID]);

            // set the actual abount for the year
            actual = totals[category.CATEGID][subcategory.SUBCATEGID];

            monthlyEst += estimated;
            monthlyAct += actual;
            for (const auto &i : categoryStats[category.CATEGID][subcategory.SUBCATEGID])
                monthlyActual[i.first] += i.second;

            DisplayRow(hb, estimated, actual, category.CATEGNAME + ": " 
                + subcategory.SUBCATEGNAME, categoryStats[category.CATEGID][subcategory.SUBCATEGID]);
        }
    }
    hb.endTbody();
    hb.startTfoot();
    DisplayRow(hb, monthlyEst, monthlyAct, _("Monthly Total"), monthlyActual, true);
    hb.endTfoot();

    hb.endTable();
    hb.endDiv();
    hb.end();

    return hb.getHTMLText();
}
