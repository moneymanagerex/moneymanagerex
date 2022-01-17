/*******************************************************
Copyright (C) 2006-2012 Nikolay Akimov
Copyright (C) 2017 James Higley
Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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
#include "model/Model_Budgetyear.h"
#include "model/Model_Budget.h"
#include "model/Model_Category.h"
#include "model/Model_Subcategory.h"
#include "reports/mmDateRange.h"

mmReportBudgetingPerformance::mmReportBudgetingPerformance()
{
    setReportParameters(Reports::BudgetPerformance);
}

mmReportBudgetingPerformance::~mmReportBudgetingPerformance()
{}

void mmReportBudgetingPerformance::DisplayRow(const wxString& catName
    , int catID
    , int subCatID
    , double estimated
    , double actual
    , int month
    , mmHTMLBuilder &hb
    , const std::map<int, double>& stats
    , bool bTotalRow)
{
    if ((estimated != 0.0) || (actual != 0.0))
    {
        if (bTotalRow)
            hb.startTotalTableRow();
        else
            hb.startTableRow();
        if (bTotalRow)
            hb.addTableCell(catName);
        else
            hb.addTableCellLink(wxString::Format("viewtrans:%d:%d", catID, subCatID), catName);

        const double est = estimated / 12.0;
        for (const auto &i : stats)
        {
            hb.startTableCell(" style='text-align:right;' nowrap");

            wxString estVal;
            if (month < 0 || (i.first % 100 == month))
            {
                estVal = Model_Currency::toString(est, Model_Currency::GetBaseCurrency());
            }
            hb.addText(estVal);
            hb.addLineBreak();

            const auto val = Model_Currency::toString(i.second, Model_Currency::GetBaseCurrency());
            hb.startSpan(val, wxString::Format(" style='text-align:right;%s' nowrap"
                , (i.second - est < 0) ? "color:red;" : ""));
            hb.endSpan();

            hb.endTableCell();
        }

        // year end
        const auto estVal = Model_Currency::toString(estimated, Model_Currency::GetBaseCurrency());
        hb.startTableCell(" style='text-align:right;' nowrap");
        hb.addText(estVal);
        hb.addLineBreak();

        const auto val = Model_Currency::toString(actual, Model_Currency::GetBaseCurrency());
        //hb.startTableCell(wxString::Format(" style='text-align:right;%s' nowrap"
        //    , (actual - estimated < 0) ? "color:red;" : "color:#009900;"));
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

wxString mmReportBudgetingPerformance::getHTMLText()
{

    int startDay;
    wxDate::Month startMonth;
    if (Option::instance().BudgetFinancialYears())
    {
        GetFinancialYearValues(startDay, startMonth);
    } else
    {
        startDay = 1;
        startMonth = wxDateTime::Jan;    
    }

    long startYear;
    int month = -1;

    wxString value = Model_Budgetyear::instance().Get(m_date_selection);
    wxString budget_year;
    wxString budget_month;

    wxRegEx pattern("^([0-9]{4})(-([0-9]{2}))?$");
    if (pattern.Matches(value))
    {
        budget_year = pattern.GetMatch(value, 1);
        budget_month = pattern.GetMatch(value, 3);
        month = wxAtoi(budget_month) - 1;
    }

    if (!budget_year.ToLong(&startYear)) {
        startYear = static_cast<long>(wxDateTime::Today().GetYear());
        budget_year = wxString::Format("%ld", startYear);
    }

    wxDateTime yearBegin(startDay, startMonth
        , static_cast<int>(startYear));
    wxDateTime yearEnd = yearBegin;
    yearEnd.Add(wxDateSpan::Year()).Subtract(wxDateSpan::Day());

    // Readjust dates by the Budget Offset Option
    Option::instance().setBudgetDateOffset(yearBegin);
    Option::instance().setBudgetDateOffset(yearEnd);
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
        , accountArray_
        , &date_range
        , Option::instance().getIgnoreFutureTransactions()
        , true
        , (evaluateTransfer ? &budgetAmt : nullptr)
        , Option::instance().BudgetFinancialYears());
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

    const wxString& headingStr = wxString::Format(_("Budget Performance for %s"),
        AdjustYearValues(startDay
            , startMonth, startYear, budget_year)
    );
    mmHTMLBuilder hb;
    hb.init();

    hb.addReportHeader(headingStr, 1, Option::instance().getIgnoreFutureTransactions());
    hb.DisplayDateHeading(yearBegin, yearEnd);
    // Prime the filter
    m_filter.clear();
    m_filter.setDateRange(yearBegin, yearEnd);
    m_filter.setAccountList(accountArray_);

    hb.addDivContainer("shadow");
    {
        hb.startTable();
        {
            hb.startThead();
            {
                hb.startTableRow();
                {
                    hb.addTableHeaderCell(_("Category"));
                    for (int yidx = 0; yidx < 12; yidx++)
                    {
                        int m = startMonth + yidx;
                        if (m >= 12) m -= 12;
                        hb.addTableHeaderCell(wxGetTranslation(
                            wxDateTime::GetEnglishMonthName(wxDateTime::Month(m)
                            , wxDateTime::Name_Abbr)), "text-center", 1);
                    }
                    hb.addTableHeaderCell(_("Total"), "text-center", 2);
                }
                hb.endTableRow();
                hb.startTableRow();
                {
                    hb.addEmptyTableCell();
                    for (int yidx = 0; yidx < 12; yidx++)
                    {
                        hb.addTableCell(_("Est.") + "<BR>" + _("Act."), false, true);
                    }
                    hb.addTableCell(_("Est.") + "<BR>" + _("Act."), false, true);
                    hb.addTableCell(_("%"), false, true);
                }
                hb.endTableRow();
            }
            hb.endThead();
            hb.startTbody();
            {
                wxString delimiter = Model_Infotable::instance().GetStringInfo("CATEG_DELIMITER", ":");
                for (const Model_Category::Data& category : allCategories)
                {
                    // Set the estimated amount for the year
                    double estimated = Model_Budget::getEstimate(false
                        , budgetPeriod[category.CATEGID][-1]
                        , budgetAmt[category.CATEGID][-1]);

                    // set the actual amount for the year
                    double actual = totals[category.CATEGID][-1];

                    monthlyEst += estimated;
                    monthlyAct += actual;
                    for (const auto &i : categoryStats[category.CATEGID][-1])
                        monthlyActual[i.first] += i.second;

                    DisplayRow(category.CATEGNAME, category.CATEGID, -1, estimated, actual
                        , month, hb, categoryStats[category.CATEGID][-1]);

                    for (const Model_Subcategory::Data& subcategory : allSubcategories)
                    {
                        if (subcategory.CATEGID != category.CATEGID) continue;

                        // Set the estimated amount for the year
                        estimated = Model_Budget::getEstimate(false
                            , budgetPeriod[category.CATEGID][subcategory.SUBCATEGID]
                            , budgetAmt[category.CATEGID][subcategory.SUBCATEGID]);

                        // set the actual abount for the year
                        actual = totals[category.CATEGID][subcategory.SUBCATEGID];

                        monthlyEst += estimated;
                        monthlyAct += actual;
                        for (const auto &i : categoryStats[category.CATEGID][subcategory.SUBCATEGID])
                            monthlyActual[i.first] += i.second;

                        DisplayRow(category.CATEGNAME + delimiter + subcategory.SUBCATEGNAME
                            , category.CATEGID
                            , subcategory.SUBCATEGID
                            , estimated, actual
                            , month, hb
                            , categoryStats[category.CATEGID][subcategory.SUBCATEGID]);
                    }
                }
            }
            hb.endTbody();
            hb.startTfoot();
            {
                DisplayRow(_("Monthly Total"), -1, -1, monthlyEst, monthlyAct, month, hb, monthlyActual, true);
            }
            hb.endTfoot();
        }
        hb.endTable();
    }
    hb.endDiv();
    hb.end();

    return hb.getHTMLText();
}
