/*************************************************************************
 Copyright (C) 2012 Stefano Giorgio
 Copyright (C) 2017 James Higley
 Copyright (C) 2021-2022 Mark Whalley (mark@ipx.co.uk)

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
#include "model/Model_Budgetyear.h"
#include "model/Model_Budget.h"
#include "model/Model_Category.h"
#include "reports/mmDateRange.h"

mmReportBudgetCategorySummary::mmReportBudgetCategorySummary()
{
    m_chart_selection = 1;
    setReportParameters(Reports::BudgetCategorySummary);
}

mmReportBudgetCategorySummary::~mmReportBudgetCategorySummary()
{}

wxString mmReportBudgetCategorySummary::getHTMLText()
{
    // Grab the data 
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

    long tmp;
    int startYear = wxDateTime::Today().GetYear();

    wxString value = Model_Budgetyear::instance().Get(m_date_selection);
    wxString budget_month, budget_year = value;

    wxRegEx pattern("^([0-9]{4})(-([0-9]{2}))?$");
    if (pattern.Matches(value))
    {
        budget_year = pattern.GetMatch(value, 1);
        budget_month = pattern.GetMatch(value, 3);
    }

    if (budget_year.ToLong(&tmp))
        startYear = static_cast<int>(tmp); // 0 <= tmp <= 9999

    budget_year = wxString::Format("%d", startYear);

    long budgetMonth = 0;
    if (budget_month.ToLong(&budgetMonth))
    {
        if (startMonth != wxDateTime::Jan)
            startMonth = wxDateTime(1, startMonth, startYear).Add(wxDateSpan::Months(--budgetMonth)).GetMonth();
        else
            startMonth = static_cast<wxDateTime::Month>(--budgetMonth);
    }

    wxDateTime yearBegin(startDay, startMonth, startYear);
    wxDateTime yearEnd = yearBegin;

    bool monthlyBudget = (!budget_month.empty());
    if (monthlyBudget) {
        yearEnd.Add(wxDateSpan::Month()).Subtract(wxDateSpan::Day());
        budget_year = wxString::Format("%i-%ld", startYear, budgetMonth);
    }
    else
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
    std::map<int64, Model_Budget::PERIOD_ID> budgetPeriod;
    std::map<int64, double> budgetAmt;
    std::map<int64, wxString> budgetNotes;
    Model_Budget::instance().getBudgetEntry(m_date_selection, budgetPeriod, budgetAmt, budgetNotes);

    std::map<int64, std::map<int, double> > categoryStats;
    Model_Category::instance().getCategoryStats(categoryStats
        , static_cast<wxSharedPtr<wxArrayString>>(nullptr)
        , &date_range, Option::instance().getIgnoreFutureTransactions()
        , false, (evaluateTransfer ? &budgetAmt : nullptr));

    std::map<int64, std::map<int, double> > budgetStats;
    Model_Budget::instance().getBudgetStats(budgetStats, &date_range, monthlyBudget);


    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    wxString headingStr = AdjustYearValues(startDay, startMonth, startYear, budget_year);
    bool amply = Option::instance().BudgetReportWithSummaries();
    const wxString headerStartupMsg = amply
        ? _("Budget Categories for %s") : _("Budget Category Summary for %s");

    headingStr = wxString::Format(headerStartupMsg
        , headingStr + "<br>" + _("(Estimated vs. Actual)"));
    hb.addReportHeader(headingStr, 1, Option::instance().getIgnoreFutureTransactions());
    hb.DisplayDateHeading(yearBegin, yearEnd);
    // Prime the filter
    m_filter.clear();
    m_filter.setDateRange(yearBegin, yearEnd);

    Model_Category::Data_Set categs = Model_Category::instance().find(Model_Category::PARENTID(-1));
    std::stable_sort(categs.begin(), categs.end(), SorterByCATEGNAME());

    // Chart
    if (getChartSelection() == 0)
    {
        GraphData gd;
        GraphSeries gsActual, gsEstimated;

        for (const auto& category : categs)
        {
            wxString categName = category.CATEGNAME;
            gsEstimated.name = _("Estimated");
            gsActual.name = _("Actual");

            gd.title = categName;
            gd.labels.push_back(category.CATEGNAME);
            gsActual.values.push_back(categoryStats[category.CATEGID][0]);
            gsEstimated.values.push_back(budgetStats[category.CATEGID][budgetMonth]);
            for(const auto& subcat : Model_Category::sub_tree(category)){
                gd.labels.push_back(Model_Category::full_name(subcat.CATEGID));
                gsActual.values.push_back(categoryStats[subcat.CATEGID][0]);
                gsEstimated.values.push_back(budgetStats[subcat.CATEGID][budgetMonth]);
            }

            if (gd.labels.size() > 1) // Bar/Line are best with at least 2 items 
            {
                gd.type = GraphData::BARLINE;
                gsEstimated.type = "column";
                gsActual.type = "line";
            }
            else
            {
                gd.type = GraphData::BAR;
            }
            gd.series.push_back(gsActual);
            gd.series.push_back(gsEstimated);
            hb.addChart(gd);

            // Now clear for next chart
            gsActual.values.clear();
            gsEstimated.values.clear();
            gd.labels.clear();
            gd.series.clear();
        }
    }
    hb.addDivContainer("shadow");
    {
        double estIncome = 0.0, estExpenses = 0.0, actIncome = 0.0, actExpenses = 0.0;
        hb.startTable();
        {
            hb.startThead();
            {
                hb.startTableRow();
                {
                    hb.addTableHeaderCell(_("Category"));
                    hb.addTableHeaderCell(_("Estimated"), "text-right");
                    hb.addTableHeaderCell(_("Actual"), "text-right");
                }
                hb.endTableRow();
            }
            hb.endThead();
            hb.startTbody();
            {
                std::map<int64, double> catTotalsEstimated, catTotalsActual;
                std::map<int64, std::pair<int, wxString>> categLevel;
                for (const auto& category : categs)
                {
                    categLevel[category.CATEGID].first = 0;
                    double estimated = budgetStats[category.CATEGID][budgetMonth];

                    if (estimated < 0)
                        estExpenses += estimated;
                    else
                        estIncome += estimated;

                    double actual = categoryStats[category.CATEGID][0];
                    if (actual < 0)
                        actExpenses += actual;
                    else
                        actIncome += actual;

                    catTotalsActual[category.CATEGID] += actual;
                    catTotalsEstimated[category.CATEGID] += estimated;

                    if (amply)
                    {
                        hb.startTableRow();
                        {
                            hb.addTableCellLink(wxString::Format("viewtrans:%d"
                                , category.CATEGID)
                                , category.CATEGNAME);
                            hb.addMoneyCell(estimated);
                            hb.addMoneyCell(actual);
                        }
                        hb.endTableRow();
                    }
                    
                    std::vector<int> totals_stack;
                    Model_Category::Data_Set subcats = Model_Category::sub_tree(category);
                    for (int i = 0; i < subcats.size(); i++) {
                        categLevel[subcats[i].CATEGID].first = 1;
                        estimated = budgetStats[subcats[i].CATEGID][budgetMonth];

                        if (estimated < 0)
                            estExpenses += estimated;
                        else
                            estIncome += estimated;

                        actual = categoryStats[subcats[i].CATEGID][0];
                        if (actual < 0)
                            actExpenses += actual;
                        else
                            actIncome += actual;

                        //save totals for this subcategory
                        catTotalsEstimated[subcats[i].CATEGID] = estimated;
                        catTotalsActual[subcats[i].CATEGID] = actual;

                        //update totals of the category
                        catTotalsEstimated[category.CATEGID] += estimated;
                        catTotalsActual[category.CATEGID] += actual;

                        //walk up the hierarchy and update all the parent totals as well
                        int64 nextParent = subcats[i].PARENTID;
                        for (int j = i; j > 0; j--) {
                            if (subcats[j - 1].CATEGID == nextParent) {
                                categLevel[subcats[i].CATEGID].first++;
                                catTotalsEstimated[subcats[j - 1].CATEGID] += estimated;
                                catTotalsActual[subcats[j - 1].CATEGID] += actual;
                                nextParent = subcats[j - 1].PARENTID;
                                if (nextParent == category.CATEGID)
                                    break;
                            }
                        }
                        categLevel[subcats[i].CATEGID].second = "";
                        for (int j = categLevel[subcats[i].CATEGID].first; j > 0; j--) {
                            categLevel[subcats[i].CATEGID].second.Prepend("&nbsp;&nbsp;&nbsp;&nbsp;");
                        }
                        if (amply) {
                            hb.startTableRow();
                            {
                                hb.addTableCell(wxString::Format(categLevel[subcats[i].CATEGID].second + "<a href=\"viewtrans:%d\" target=\"_blank\">%s</a>"
                                    , subcats[i].CATEGID
                                    , subcats[i].CATEGNAME));
                                hb.addMoneyCell(estimated);
                                hb.addMoneyCell(actual);
                            }
                            hb.endTableRow();
                            
                            if (i < subcats.size() - 1) { //not the last subcategory
                                if (subcats[i].CATEGID == subcats[i + 1].PARENTID) totals_stack.push_back(i); //if next subcategory is our child, queue the total for after the children
                                else if (subcats[i].PARENTID != subcats[i + 1].PARENTID) { // last sibling -- we've exhausted this branch, so display all the totals we held on to
                                    while (!totals_stack.empty() && subcats[totals_stack.back()].CATEGID != subcats[i + 1].PARENTID) {
                                        hb.startAltTableRow();
                                        {
                                            int index = totals_stack.back();
                                            hb.addTableCell(wxString::Format(categLevel[subcats[index].CATEGID].second + "<a href=\"viewtrans:%d:-2\" target=\"_blank\">%s</a>"
                                                , subcats[index].CATEGID
                                                , subcats[index].CATEGNAME));
                                            hb.addMoneyCell(catTotalsEstimated[subcats[index].CATEGID]);
                                            hb.addMoneyCell(catTotalsActual[subcats[index].CATEGID]);
                                        }
                                        hb.endTableRow();
                                        totals_stack.pop_back();
                                    }
                                }
                            }
                            // the very last subcategory, so show the rest of the queued totals
                            else {
                                while (!totals_stack.empty()) {
                                    hb.startAltTableRow();
                                    {
                                        int index = totals_stack.back();
                                        hb.addTableCell(wxString::Format(categLevel[subcats[index].CATEGID].second + "<a href=\"viewtrans:%d:-2\" target=\"_blank\">%s</a>"
                                            , subcats[index].CATEGID
                                            , subcats[index].CATEGNAME));
                                        hb.addMoneyCell(catTotalsEstimated[subcats[index].CATEGID]);
                                        hb.addMoneyCell(catTotalsActual[subcats[index].CATEGID]);
                                    }
                                    hb.endTableRow();
                                    totals_stack.pop_back();
                                }
                            }
                        }
                    }
                    amply ? hb.startAltTableRow() : hb.startTableRow();
                    {
                        hb.addTableCellLink(wxString::Format("viewtrans:%d:-2"
                            , category.CATEGID)
                            , category.CATEGNAME);
                        hb.addMoneyCell(catTotalsEstimated[category.CATEGID]);
                        hb.addMoneyCell(catTotalsActual[category.CATEGID]);
                    }
                    hb.endTableRow();
                }
            }
            hb.endTbody();
        }
        hb.endTable();
        hb.startTable();
        {
            double difIncome = actIncome - estIncome;
            double difExpense = actExpenses - estExpenses;

            //Summary of Estimated Vs Actual totals
            hb.startTbody();
            {
                hb.startTotalTableRow();
                {
                    hb.addTableCell(_("Estimated Income:"));
                    hb.addMoneyCell(estIncome);
                    hb.addTableCell(_("Actual Income:"));
                    hb.addMoneyCell(actIncome);
                    hb.addTableCell(_("Difference Income:"));
                    hb.addMoneyCell(difIncome);
                }
                hb.endTableRow();

                hb.startTotalTableRow();
                {
                    hb.addTableCell(_("Estimated Expenses:"));
                    hb.addMoneyCell(estExpenses);
                    hb.addTableCell(_("Actual Expenses:"));
                    hb.addMoneyCell(actExpenses);
                    hb.addTableCell(_("Difference Expenses:"));
                    hb.addMoneyCell(difExpense);
                }
                hb.endTableRow();
            }
            hb.endTfoot();
        }
        hb.endTable();
    }
    hb.endDiv();

    hb.end();

    wxLogDebug("======= mmReportBudgetCategorySummary:getHTMLText =======");
    wxLogDebug("%s", hb.getHTMLText());

    return hb.getHTMLText();
}
