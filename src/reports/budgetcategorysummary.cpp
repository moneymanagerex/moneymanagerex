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

wxString mmReportBudgetCategorySummary::version()
{
    return "$Rev$";
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
    wxString headerStartupMsg;
    if (mmIniOptions::instance().budgetSummaryWithoutCategories_)
        headerStartupMsg = _("Budget Categories for ");
    else
        headerStartupMsg = _("Budget Category Summary for ");
    hb.addHeader(2, headerStartupMsg + headingStr + "<br>" + _("( Estimated Vs Actual )"));
    hb.DisplayDateHeading(yearBegin, yearEnd);

    double estIncome = 0.0, estExpenses = 0.0, actIncome = 0.0, actExpenses = 0.0;

    hb.startCenter();

    hb.startTable("65%");
    hb.startTableRow();
    hb.addTableHeaderCell(_("Category"));
    hb.addTableHeaderCell(_("Sub Category"));
    hb.addTableHeaderCell(_("Amount"), true);
    hb.addTableHeaderCell(_("Frequency"));
    hb.addTableHeaderCell(_("Estimated"), true);
    hb.addTableHeaderCell(_("Actual"), true);
    hb.endTableRow();

    int categID = -1;
    double catTotalsAmt = 0.0, catTotalsEstimated = 0.0, catTotalsActual = 0.0;

    for (const auto& category : Model_Category::all_categories())
    {
        double estimated = (monthlyBudget
            ? Model_Budget::getMonthlyEstimate(budgetPeriod[category.second.first][category.second.second]
                , budgetAmt[category.second.first][category.second.second])
            : Model_Budget::getYearlyEstimate(budgetPeriod[category.second.first][category.second.second]
                , budgetAmt[category.second.first][category.second.second]));
        if (estimated < 0) {
            estExpenses += estimated;
        } else {
            estIncome += estimated;
        }

        double actual = categoryStats[category.second.first][category.second.second][0];
        if (actual < 0) {
            actExpenses += actual;
        } else {
            actIncome += actual;
        }

        if (categID == -1) categID = category.second.first;
        
        catTotalsAmt += actual; //FIXME: what the amount?
        catTotalsActual += actual;
        catTotalsEstimated += estimated;
        /***************************************************************************/
        if (mmIniOptions::instance().budgetSummaryWithoutCategories_)
        {
            double amt = budgetAmt[category.second.first][category.second.second];
            hb.startTableRow();
            hb.addTableCell(category.first, false, true);
            hb.addTableCell(wxEmptyString, false, true);
            hb.addMoneyCell(amt, false);
            hb.addTableCell(wxGetTranslation(Model_Budget::all_period()[budgetPeriod[category.second.first][category.second.second]]), false, true);
            hb.addMoneyCell(estimated, false);
            hb.addMoneyCell(actual, actualAmountColour(amt, actual, estimated));
            hb.endTableRow();
        }

        /***************************************************************************
        Display a TOTALS entry for the category.
        ****************************************************************************/
        if (categID != category.second.first 
            || category.second.second == Model_Category::all_categories().rbegin()->second.second)
        {
            if (mmIniOptions::instance().budgetSummaryWithoutCategories_) {
                hb.addRowSeparator(6);
            }
            // Category, Sub Category, Period, Amount, Estimated, Actual
            hb.startTableRow();
            Model_Category::Data *c = Model_Category::instance().get(categID);
            wxString categName = "";
            if (c) categName = c->CATEGNAME;
            hb.addTableCell(categName, false, true, true, "blue");
            hb.addTableCell(wxEmptyString, false, true, true, "blue");
            hb.addTableCell(wxEmptyString, true, false, true, "blue");
            hb.addTableCell(wxEmptyString, false, true, true, "blue");
            hb.addMoneyCell(catTotalsEstimated, wxString("blue"));
            hb.addMoneyCell(catTotalsActual, this->actualAmountColour(catTotalsAmt, catTotalsActual, catTotalsEstimated, true));
            hb.endTableRow();
            hb.addRowSeparator(6);

            catTotalsAmt = catTotalsEstimated = catTotalsActual = 0.0;

        }
        categID = category.second.first;
    }

    hb.endTable();
    hb.endCenter();

    double difIncome = estIncome - actIncome;
    double difExpense = estExpenses - actExpenses;

    //Summary of Estimated Vs Actual totals
    hb.addLineBreak();
    hb.startCenter();
    hb.startTable("55%");
    hb.startTableRow();
    hb.addTableCell(_("Estimated Income:"), true, true);
    hb.addMoneyCell(estIncome, false);
    hb.addTableCell(_("Actual Income:"), true, true);
    hb.addMoneyCell(actIncome, false);
    hb.addTableCell(_("Difference Income:"), true, true);
    hb.addMoneyCell(difIncome, false);
    hb.endTableRow();

    hb.startTableRow();
    hb.addTableCell(_("Estimated Expenses:"), true, true);
    hb.addMoneyCell(estExpenses, false);
    hb.addTableCell(_("Actual Expenses:"), true, true);
    hb.addMoneyCell(actExpenses, false);
    hb.addTableCell(_("Difference Expenses:"), true, true);
    hb.addMoneyCell(difExpense, false);
    hb.endTableRow();
    hb.addRowSeparator(6);
    hb.endTable();
    hb.endCenter();

    hb.end();
    return hb.getHTMLText();
}
