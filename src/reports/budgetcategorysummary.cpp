/*************************************************************************
 Copyright (C) 2012 Stefano Giorgio

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
#include "htmlbuilder.h"
#include "mmex.h"
#include "mmframe.h"
#include "model/Model_Budgetyear.h"
#include "model/Model_Budget.h"
#include "model/Model_Category.h"
#include "model/Model_Subcategory.h"
#include "reports/mmDateRange.h"

mmReportBudgetCategorySummary::mmReportBudgetCategorySummary(int budgetYearID)
: budgetYearID_(budgetYearID)
{}

mmReportBudgetCategorySummary::~mmReportBudgetCategorySummary()
{}

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
    int startDay = 1;
    int startMonth = wxDateTime::Jan;
    int endDay   = 31;
    int endMonth = wxDateTime::Dec;

    long startYear;
    wxString startYearStr = Model_Budgetyear::instance().Get(budgetYearID_);
    startYearStr.ToLong(&startYear);

    wxString headingStr = AdjustYearValues(startDay, startMonth, startYear, startYearStr);
    wxDateTime yearBegin(startDay, (wxDateTime::Month)startMonth, startYear);
    wxDateTime yearEnd(endDay, (wxDateTime::Month)endMonth, startYear);

    bool monthlyBudget = (startYearStr.length() > 5);
    if (monthlyBudget) {
        SetBudgetMonth(startYearStr, yearBegin, yearEnd);
    } else {
        AdjustDateForEndFinancialYear(yearEnd);
    }
    mmSpecifiedRange date_range(yearBegin, yearEnd);

    bool evaluateTransfer = false;
    if (mmIniOptions::instance().budgetIncludeTransfers_)
    {
        evaluateTransfer = true;
    }
    //Get statistics
    std::map<int, std::map<int, Model_Budget::PERIOD_ENUM> > budgetPeriod;
    std::map<int, std::map<int, double> > budgetAmt;
    Model_Budget::instance().getBudgetEntry(budgetYearID_, budgetPeriod, budgetAmt);
    std::map<int, std::map<int, std::map<int, double> > > categoryStats;
    Model_Category::instance().getCategoryStats(categoryStats, &date_range, mmIniOptions::instance().ignoreFutureTransactions_,
        false, true, (evaluateTransfer ? &budgetAmt : 0));

    mmHTMLBuilder hb;
    hb.init();
    hb.addDivContainer();
    bool amply = mmIniOptions::instance().budgetReportWithSummaries_;
    const wxString& headerStartupMsg = amply ? _("Budget Categories for %s") : _("Budget Category Summary for %s");

    hb.addHeader(2, wxString::Format(headerStartupMsg,  headingStr + "<br>" + _("( Estimated Vs Actual )")));
    hb.DisplayDateHeading(yearBegin, yearEnd);

    double estIncome = 0.0, estExpenses = 0.0, actIncome = 0.0, actExpenses = 0.0;

    hb.addDivRow();
    hb.addDivCol8();
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

    int categID = -1;
    double catTotalsAmt = 0.0, catTotalsEstimated = 0.0, catTotalsActual = 0.0;

    auto categs = Model_Category::all_categories();
    categs[L"\uF8FF"] = std::make_pair(-1, -1); //last alphabetical charicter
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

    double difIncome = estIncome - actIncome;
    double difExpense = estExpenses - actExpenses;

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
