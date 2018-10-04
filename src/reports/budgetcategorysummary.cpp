/*************************************************************************
 Copyright (C) 2012 Stefano Giorgio
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
 *************************************************************************/

/*************************************************************************
 Renamed after extensive modifications to original file reportbudgetsetup.cpp
**************************************************************************/
#include "budgetcategorysummary.h"
#include "reports/htmlbuilder.h"
#include "mmex.h"
#include "mmframe.h"
#include "Model_Budgetyear.h"
#include "Model_Budget.h"
#include "Model_Category.h"
#include "Model_Subcategory.h"
#include "reports/mmDateRange.h"

mmReportBudgetCategorySummary::mmReportBudgetCategorySummary()
{
    m_chart_selection = 1;
}

mmReportBudgetCategorySummary::~mmReportBudgetCategorySummary()
{}

int mmReportBudgetCategorySummary::report_parameters()
{
    return RepParams::BUDGET_DATES | RepParams::CHART;
}

wxString mmReportBudgetCategorySummary::actualAmountColour(double amount, double actual, double estimated, bool total)
{
    wxString actAmtColStr = "black";
    if (total) {
        actAmtColStr = "blue";
    }

    if (amount == 0) {
        actAmtColStr = "blue";
    } else {
        if (actual < estimated) {
            actAmtColStr = "red";
        }
    }

    return actAmtColStr;
}

wxString mmReportBudgetCategorySummary::getHTMLText()
{
    unsigned short startDay = 1;
    long startMonth, startYear;

    wxString value = Model_Budgetyear::instance().Get(m_date_selection);
    wxString budget_month, budget_year = value;

    wxRegEx pattern("^([0-9]{4})(|-[0-9]{2})$");
    if (pattern.Matches(value))
    {
        budget_year = pattern.GetMatch(value, 1);
        budget_month = pattern.GetMatch(value, 2);
        budget_month.Replace("-", "");
    }

    if (!budget_year.ToLong(&startYear)) {
        startYear = static_cast<long>(wxDateTime::Today().GetYear());
        budget_year = wxString::Format("%ld", startYear);
    }

    if (!budget_month.ToLong(&startMonth))
        startMonth = static_cast<long>(wxDateTime::Jan);
    else
        startMonth--;

    wxDateTime yearBegin(startDay, (wxDateTime::Month)startMonth
        , static_cast<int>(startYear));
    wxDateTime yearEnd = yearBegin;

    bool monthlyBudget = (!budget_month.empty());
    if (monthlyBudget)
        yearEnd.Add(wxDateSpan::Month()).Subtract(wxDateSpan::Day());
    else
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
        , &date_range, Option::instance().IgnoreFutureTransactions()
        , false, (evaluateTransfer ? &budgetAmt : nullptr));

    auto categs = Model_Category::all_categories();
    categs[L"\uF8FF"] = std::make_pair(-1, -1); //last alphabetical charicter
    int categID = -1;
    mmHTMLBuilder hb;
    //---------------------------------
    const wxString& headingStr = AdjustYearValues(static_cast<int>(startDay)
        , startMonth, startYear, budget_year);
    hb.init();
    hb.addDivContainer();
    bool amply = Option::instance().BudgetReportWithSummaries();
    const wxString& headerStartupMsg = amply 
        ? _("Budget Categories for %s") : _("Budget Category Summary for %s");

    hb.addHeader(2, wxString::Format(headerStartupMsg
        ,  headingStr + "<br>" + _("( Estimated Vs Actual )")));
    hb.addDateNow();
    hb.DisplayDateHeading(yearBegin, yearEnd);

    double estIncome = 0.0, estExpenses = 0.0, actIncome = 0.0, actExpenses = 0.0;

    hb.addDivRow();
    hb.addDivCol17_67();
    // Add the graph
    if (getChartSelection() == 0)
    {
        hb.addDivCol25_50();
        std::vector<ValueTrio> actValueList, estValueList;
        for (const auto& category : categs)
        {
            if (categID != category.second.first && categID != -1)
            {
                Model_Category::Data *c = Model_Category::instance().get(categID);
                wxString categName = "Categories";
                if (c) categName = c->CATEGNAME;
                if ((actValueList.size() > 2) && (estValueList.size() > 2) && (getChartSelection() == 0))
                    hb.addRadarChart(actValueList, estValueList, categName);

                actValueList.clear();
                estValueList.clear();
            }
            if (category.second.first != -1)
            {
                double estimated = (monthlyBudget
                    ? Model_Budget::getMonthlyEstimate(budgetPeriod[category.second.first][category.second.second]
                        , budgetAmt[category.second.first][category.second.second])
                    : Model_Budget::getYearlyEstimate(budgetPeriod[category.second.first][category.second.second]
                        , budgetAmt[category.second.first][category.second.second]));
                double actual = categoryStats[category.second.first][category.second.second][0];

                ValueTrio vt;
                vt.label = category.first;
                vt.amount = actual;
                vt.color = hb.getColor(0);
                actValueList.push_back(vt);
                vt.amount = estimated;
                estValueList.push_back(vt);
            }
            categID = category.second.first;
        }
        hb.endDiv();
    }
    hb.startTable();
    hb.startThead();
    hb.startTableRow();
    hb.addTableHeaderCell(_("Category"));
    hb.addTableHeaderCell(_("Amount"), true);
    hb.addTableHeaderCell(_("Frequency"));
    hb.addTableHeaderCell(_("Estimated"), true);
    hb.addTableHeaderCell(_("Actual"), true);
    hb.endTableRow();
    hb.endThead();

    categID = -1;
    double catTotalsAmt = 0.0, catTotalsEstimated = 0.0, catTotalsActual = 0.0;

    for (const auto& category : categs)
    {
        double estimated = (monthlyBudget
            ? Model_Budget::getMonthlyEstimate(budgetPeriod[category.second.first][category.second.second]
                , budgetAmt[category.second.first][category.second.second])
            : Model_Budget::getYearlyEstimate(budgetPeriod[category.second.first][category.second.second]
                , budgetAmt[category.second.first][category.second.second]));

        if (estimated < 0)
            estExpenses += estimated;
        else
            estIncome += estimated;

        double actual = categoryStats[category.second.first][category.second.second][0];
        if (actual < 0)
            actExpenses += actual;
        else
            actIncome += actual;

        /***************************************************************************
        Display a TOTALS entry for the category.
        ****************************************************************************/
        if (categID != category.second.first && categID != -1)
        {
            // Category, Period, Amount, Estimated, Actual
            amply ? hb.startTableRow(mmColors::listFutureDateColor.GetAsString()) : hb.startTableRow();
            Model_Category::Data *c = Model_Category::instance().get(categID);
            wxString categName = "";
            if (c) categName = c->CATEGNAME;
            hb.addTableCell(categName);
            hb.addTableCell(wxEmptyString);
            hb.addTableCell(wxEmptyString);
            hb.addMoneyCell(catTotalsEstimated);
            hb.addMoneyCell(catTotalsActual);
            hb.endTableRow();

            catTotalsAmt = catTotalsEstimated = catTotalsActual = 0.0;
        }
        
        catTotalsAmt += actual;
        catTotalsActual += actual;
        catTotalsEstimated += estimated;

        /***************************************************************************/
        if (amply && category.second.first != -1)
        {
            double amt = budgetAmt[category.second.first][category.second.second];
            hb.startTableRow();
            hb.addTableCell(category.first);
            hb.addMoneyCell(amt);
            hb.addTableCell(wxGetTranslation(Model_Budget::all_period()[budgetPeriod[category.second.first][category.second.second]]));
            hb.addMoneyCell(estimated);
            hb.addMoneyCell(actual);
            hb.endTableRow();
        }

        categID = category.second.first;
    }

    hb.endTable();

    double difIncome = actIncome - estIncome;
    double difExpense = actExpenses - estExpenses;

    //Summary of Estimated Vs Actual totals
    hb.startTfoot();
    hb.startTable();
    hb.startTotalTableRow();
    hb.addTableCell(_("Estimated Income:"));
    hb.addMoneyCell(estIncome);
    hb.addTableCell(_("Actual Income:"));
    hb.addMoneyCell(actIncome);
    hb.addTableCell(_("Difference Income:"));
    hb.addMoneyCell(difIncome);
    hb.endTableRow();

    hb.startTotalTableRow();
    hb.addTableCell(_("Estimated Expenses:"));
    hb.addMoneyCell(estExpenses);
    hb.addTableCell(_("Actual Expenses:"));
    hb.addMoneyCell(actExpenses);
    hb.addTableCell(_("Difference Expenses:"));
    hb.addMoneyCell(difExpense);
    hb.endTableRow();
    hb.endTfoot();

    hb.endTable();
    hb.endDiv();
    hb.endDiv();
    hb.endDiv();
    hb.end();

    return hb.getHTMLText();
}
