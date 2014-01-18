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

void mmReportBudgetingPerformance::DisplayEstimateMonths(mmHTMLBuilder& hb, double estimated)
{
    for (int yidx = 0; yidx < 12; yidx++)
    {
        // Set the estimate for each month
        hb.addMoneyCell(estimated / 12, false);
    }
}

void mmReportBudgetingPerformance::DisplayActualMonths(mmHTMLBuilder& hb, double estimated, std::map<int, double>& actual)
{
    double est = estimated / 12;
    for (const auto &i : actual)
    {
        double actualMonthVal = i.second;

        if (actualMonthVal < est)
        {
            hb.addMoneyCell(actualMonthVal, wxString("RED"));
        }
        else
        {
            hb.addMoneyCell(actualMonthVal, false);
        }
    }
}

wxString mmReportBudgetingPerformance::version()
{
    return "$Rev$";
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

    wxString headingStr = AdjustYearValues(startDay, startMonth, startYear, startYearStr);
    wxDateTime yearBegin(startDay, (wxDateTime::Month)startMonth, startYear);
    wxDateTime yearEnd(endDay, (wxDateTime::Month)endMonth, startYear);

    AdjustDateForEndFinancialYear(yearEnd);
    mmSpecifiedRange date_range(yearBegin, yearEnd);

    bool evaluateTransfer = false;
    if (wxGetApp().m_frame->budgetTransferTotal())
    {
        evaluateTransfer = true;
    }
    //Get statistics
    std::map<int, std::map<int, Model_Budget::PERIOD_ENUM> > budgetPeriod;
    std::map<int, std::map<int, double> > budgetAmt;
    Model_Budget::instance().getBudgetEntry(budgetYearID_, budgetPeriod, budgetAmt);
    std::map<int, std::map<int, std::map<int, double> > > categoryStats;
    Model_Category::instance().getCategoryStats(categoryStats, &date_range, mmIniOptions::instance().ignoreFutureTransactions_,
        true, true, (evaluateTransfer ? &budgetAmt : 0));
    //Init totals
    const Model_Category::Data_Set allCategories = Model_Category::instance().all(Model_Category::COL_CATEGNAME);
    const Model_Subcategory::Data_Set allSubcategories = Model_Subcategory::instance().all(Model_Subcategory::COL_SUBCATEGNAME);
    std::map<int, std::map<int, double> > totals;
    for (const auto& category : allCategories)
    {
        totals[category.CATEGID][-1] = 0;
        for (const Model_Subcategory::Data& subcategory : allSubcategories)
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
    hb.addHeader(2, _("Budget Performance for ") + headingStr );
    hb.DisplayDateHeading(yearBegin, yearEnd);

    hb.startCenter();

    hb.startTable();
    hb.startTableRow();
    hb.addTableHeaderCell(_("Category"));
    hb.addTableHeaderCell(_("Type"));

    for (int yidx = 0; yidx < 12; yidx++)
    {
        int m = startMonth + yidx;
        if (m >= 12) m -= 12;
        hb.addTableHeaderCell(wxGetTranslation(wxDateTime::GetEnglishMonthName(wxDateTime::Month(m), wxDateTime::Name_Abbr)));
    }
    hb.addTableHeaderCell(_("Overall"));
    hb.addTableHeaderCell(_("%"));
    hb.endTableRow();

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
            hb.addTableCell(category.CATEGNAME, false, true);
            hb.addTableCell(_("Estimated"));

            DisplayEstimateMonths(hb, estimated);

            hb.addMoneyCell(estimated, false);
            hb.addTableCell("-");
            hb.endTableRow();

            // actual stuff
            hb.startTableRow();
            hb.addTableCell(category.CATEGNAME, false, true);
            hb.addTableCell(_("Actual"));

            DisplayActualMonths(hb, estimated, categoryStats[category.CATEGID][-1]);

            // year end
            if(actual < estimated)
            {
                hb.addMoneyCell(actual, wxString("RED"));
            }
            else
            {
                hb.addMoneyCell(actual, false);
            }

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

            hb.addRowSeparator(16);
        }

        for (const Model_Subcategory::Data& subcategory : allSubcategories)
        {
            if (subcategory.CATEGID != category.CATEGID) continue;

            // Set the estimated amount for the year
            estimated = Model_Budget::getYearlyEstimate(budgetPeriod[category.CATEGID][subcategory.SUBCATEGID], budgetAmt[category.CATEGID][subcategory.SUBCATEGID]);

            // set the actual abount for the year
            actual = totals[category.CATEGID][subcategory.SUBCATEGID];

            if ((estimated != 0.0) || (actual != 0.0))
            {
                hb.startTableRow();
                hb.addTableCell(category.CATEGNAME + ": " + subcategory.SUBCATEGNAME, false, true);
                hb.addTableCell(_("Estimated"));

                DisplayEstimateMonths(hb, estimated);

                hb.addMoneyCell(estimated, false);
                hb.addTableCell("-");
                hb.endTableRow();

                hb.startTableRow();
                hb.addTableCell(category.CATEGNAME + ": " + subcategory.SUBCATEGNAME, false, true);
                hb.addTableCell(_("Actual"));

                DisplayActualMonths(hb, estimated, categoryStats[category.CATEGID][subcategory.SUBCATEGID]);

                // year end
                if(actual < estimated)
                {
                    hb.addMoneyCell(actual, wxString("RED"));
                }
                else
                {
                    hb.addMoneyCell(actual, false);
                }

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
                hb.addRowSeparator(16);
            }
        }
    }

    hb.endTable();
    hb.endCenter();

    hb.end();
    return hb.getHTMLText();
}
