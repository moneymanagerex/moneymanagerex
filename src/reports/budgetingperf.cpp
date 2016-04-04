/*******************************************************
Copyright (C) 2006-2012

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
#include "htmlbuilder.h"
#include "mmex.h"
#include "mmframe.h"
#include "model/Model_Budgetyear.h"
#include "model/Model_Budget.h"
#include "model/Model_Category.h"
#include "model/Model_Subcategory.h"
#include "reports/mmDateRange.h"

mmReportBudgetingPerformance::mmReportBudgetingPerformance(int budgetYearID)
: budgetYearID_(budgetYearID)
{}

mmReportBudgetingPerformance::~mmReportBudgetingPerformance()
{}

const wxString mmReportBudgetingPerformance::DisplayEstimateMonths(double estimated)
{
    mmHTMLBuilder hb;
    double amount = estimated / 12;
    const auto val = Model_Currency::toString(amount, Model_Currency::GetBaseCurrency());
    for (int yidx = 0; yidx < 12; yidx++)
    {
        hb.addTableCell(val, true);
    }

    return hb.getHTMLText();
}

const wxString  mmReportBudgetingPerformance::DisplayActualMonths(double estimated, std::map<int, double>& actual)
{
    mmHTMLBuilder hb;
    double est = estimated / 12;
    for (const auto &i : actual)
    {
        hb.startTableCell(wxString::Format(" style='text-align:right;%s' nowrap", (i.second - est < 0) ? "color:#FF0000;" : ""));
        const auto val = Model_Currency::toString(i.second, Model_Currency::GetBaseCurrency());
        hb.addText(val);
        hb.endTableCell();
    }
    return hb.getHTMLText();
}

wxString mmReportBudgetingPerformance::getHTMLText()
{
    int startDay = 1;
    int startMonth = wxDateTime::Jan;
    int endDay   = 31;
    int endMonth = wxDateTime::Dec;

    long startYear;
    wxString startYearStr = Model_Budgetyear::instance().Get(budgetYearID_);
    startYearStr.ToLong(&startYear);

    const wxString& headingStr = AdjustYearValues(startDay, startMonth, startYear, startYearStr);
    wxDateTime yearBegin(startDay, (wxDateTime::Month)startMonth, startYear);
    wxDateTime yearEnd(endDay, (wxDateTime::Month)endMonth, startYear);

    AdjustDateForEndFinancialYear(yearEnd);
    mmSpecifiedRange date_range(yearBegin, yearEnd);

    bool evaluateTransfer = false;
    if (Option::instance().BudgetIncludeTransfers())
    {
        evaluateTransfer = true;
    }
    //Get statistics
    std::map<int, std::map<int, Model_Budget::PERIOD_ENUM> > budgetPeriod;
    std::map<int, std::map<int, double> > budgetAmt;
    Model_Budget::instance().getBudgetEntry(budgetYearID_, budgetPeriod, budgetAmt);
    std::map<int, std::map<int, std::map<int, double> > > categoryStats;
    Model_Category::instance().getCategoryStats(categoryStats, &date_range, Option::instance().IgnoreFutureTransactions(),
        true, true, (evaluateTransfer ? &budgetAmt : nullptr));
    //Init totals
    const auto &allCategories = Model_Category::instance().all(Model_Category::COL_CATEGNAME);
    const auto &allSubcategories = Model_Subcategory::instance().all(Model_Subcategory::COL_SUBCATEGNAME);
    std::map<int, std::map<int, double> > totals;
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
                    totals[category.CATEGID][subcategory.SUBCATEGID] += categoryStats[category.CATEGID][subcategory.SUBCATEGID][i.first];
            }
        }
    }

    mmHTMLBuilder hb;
    hb.init();
    hb.addDivContainer();
    hb.addHeader(2, wxString::Format(_("Budget Performance for %s"), headingStr));
    hb.DisplayDateHeading(yearBegin, yearEnd);

    hb.startTable();
    hb.startThead();
    hb.startTableRow();
    hb.addTableHeaderCell(_("Category"));
    hb.addTableHeaderCell(_("Type"));

    for (int yidx = 0; yidx < 12; yidx++)
    {
        int m = startMonth + yidx;
        if (m >= 12) m -= 12;
        hb.addTableHeaderCell(wxGetTranslation(wxDateTime::GetEnglishMonthName(wxDateTime::Month(m), wxDateTime::Name_Abbr)), true);
    }
    hb.addTableHeaderCell(_("Overall"), true);
    hb.addTableHeaderCell(_("%"), true);
    hb.endTableRow();
    hb.endThead();

    hb.startTbody();
    for (const Model_Category::Data& category : allCategories)
    {
        // Set the estimated amount for the year
        double estimated = Model_Budget::getYearlyEstimate(budgetPeriod[category.CATEGID][-1], budgetAmt[category.CATEGID][-1]);

        // set the actual amount for the year
        double actual = totals[category.CATEGID][-1];

        // estimated stuff
        if ((estimated != 0.0) || (actual != 0.0))
        {
            hb.startTableRow();
            hb.addTableCell(category.CATEGNAME);
            hb.addTableCell(_("Estimated"));

            //hb.addText(DisplayEstimateMonths(estimated));
            hb.startTableCell(" style='text-align:right;' nowrap");
            const auto estVal = Model_Currency::toString(estimated, Model_Currency::GetBaseCurrency());
            hb.addText(estVal);
            hb.endTableCell();

            hb.addMoneyCell(estimated);
            hb.addTableCell("-");
            hb.endTableRow();

            // actual stuff
            hb.startTableRow();
            hb.addTableCell(category.CATEGNAME);
            hb.addTableCell(_("Actual"));

            hb.addText(DisplayActualMonths(estimated, categoryStats[category.CATEGID][-1]));

            // year end
            hb.startTableCell(wxString::Format(" style='text-align:right;%s' nowrap"
                , (actual - estimated < 0) ? "color:#FF0000;" : "color:#009900;"));
            const auto val = Model_Currency::toString(actual, Model_Currency::GetBaseCurrency());
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

        for (const Model_Subcategory::Data& subcategory : allSubcategories)
        {
            if (subcategory.CATEGID != category.CATEGID) continue;

            // Set the estimated amount for the year
            estimated = Model_Budget::getYearlyEstimate(budgetPeriod[category.CATEGID][subcategory.SUBCATEGID]
                , budgetAmt[category.CATEGID][subcategory.SUBCATEGID]);

            // set the actual abount for the year
            actual = totals[category.CATEGID][subcategory.SUBCATEGID];

            if ((estimated != 0.0) || (actual != 0.0))
            {
                hb.startTableRow();
                hb.addTableCell(category.CATEGNAME + ": " + subcategory.SUBCATEGNAME);
                hb.addTableCell(_("Estimated"));

                hb.addText(DisplayEstimateMonths(estimated));

                //hb.addMoneyCell(estimated);
                hb.startTableCell(" style='text-align:right;' nowrap");
                const auto estVal = Model_Currency::toString(estimated, Model_Currency::GetBaseCurrency());
                hb.addText(estVal);
                hb.endTableCell();

                hb.addTableCell("-");
                hb.endTableRow();

                hb.startTableRow();
                hb.addTableCell(category.CATEGNAME + ": " + subcategory.SUBCATEGNAME);
                hb.addTableCell(_("Actual"));

                hb.addText(DisplayActualMonths(estimated, categoryStats[category.CATEGID][subcategory.SUBCATEGID]));

                // year end
                hb.startTableCell(wxString::Format(" style='text-align:right;%s' nowrap"
                    , ((actual - estimated < 0)) ? "color:#FF0000;" : "color:#009900;"));
                const auto val = Model_Currency::toString(actual, Model_Currency::GetBaseCurrency());
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
    }
    hb.endTbody();

    hb.endTable();
    hb.endDiv();
    hb.end();

    Model_Report::outputReportFile(hb.getHTMLText());
    return "";
}
