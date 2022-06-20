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
#include "model/Model_Subcategory.h"
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

    if (budget_month.ToLong(&tmp))
        startMonth = static_cast<wxDateTime::Month>(--tmp);

    wxDateTime yearBegin(startDay, startMonth, startYear);
    wxDateTime yearEnd = yearBegin;

    bool monthlyBudget = (!budget_month.empty());
    if (monthlyBudget) {
        yearEnd.Add(wxDateSpan::Month()).Subtract(wxDateSpan::Day());
        budget_year = wxString::Format("%i-%ld", startYear, tmp);
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
    std::map<int, std::map<int, Model_Budget::PERIOD_ENUM> > budgetPeriod;
    std::map<int, std::map<int, double> > budgetAmt;
    Model_Budget::instance().getBudgetEntry(m_date_selection, budgetPeriod, budgetAmt);
    std::map<int, std::map<int, std::map<int, double> > > categoryStats;
    Model_Category::instance().getCategoryStats(categoryStats
        , static_cast<wxSharedPtr<wxArrayString>>(nullptr)
        , &date_range, Option::instance().getIgnoreFutureTransactions()
        , false, (evaluateTransfer ? &budgetAmt : nullptr));

    auto categs = Model_Category::all_categories();
    categs[L"\uF8FF"] = std::make_pair(-1, -1); //last alphabetical character
    int categID = -1;

    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    wxString headingStr = AdjustYearValues(startDay, startMonth, startYear, budget_year);
    bool amply = Option::instance().BudgetReportWithSummaries();
    const wxString& headerStartupMsg = amply
            ? _("Budget Categories for %s") : _("Budget Category Summary for %s");

    headingStr = wxString::Format(headerStartupMsg
        , headingStr + "<br>" + _("( Estimated Vs Actual )"));
    hb.addReportHeader(headingStr, 1, Option::instance().getIgnoreFutureTransactions());
    hb.DisplayDateHeading(yearBegin, yearEnd);
    // Prime the filter
    m_filter.clear();
    m_filter.setDateRange(yearBegin, yearEnd);

    double estIncome = 0.0, estExpenses = 0.0, actIncome = 0.0, actExpenses = 0.0;
    // Chart
    if (getChartSelection() == 0)
    {
        GraphData gd;
        GraphSeries gsActual, gsEstimated;

        for (const auto& category : categs)
        {
            if (categID != category.second.first && categID != -1)
            {
                Model_Category::Data *c = Model_Category::instance().get(categID);
                wxString categName = "Categories";
                if (c) categName = c->CATEGNAME;
                gsEstimated.name = _("Estimated");
                gsActual.name = _("Actual");

                gd.title = categName;
                if (gd.labels.size() > 1) // Bar/Line are best with at least 2 items 
                {
                    gd.type = GraphData::BARLINE;
                    gsEstimated.type = "column";
                    gsActual.type = "line";
                } else
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
            if (category.second.first != -1)
            {
                double estimated = Model_Budget::getEstimate(monthlyBudget
                    , budgetPeriod[category.second.first][category.second.second]
                    , budgetAmt[category.second.first][category.second.second]);
                double actual = categoryStats[category.second.first][category.second.second][0];

                gd.labels.push_back(category.first);
                gsActual.values.push_back(actual);
                gsEstimated.values.push_back(estimated);
            }
            categID = category.second.first;
        }
    }
    hb.addDivContainer("shadow");
    {
        hb.startTable();
        {
            hb.startThead();
            {
                hb.startTableRow();
                {
                    hb.addTableHeaderCell(_("Category"));
                    hb.addTableHeaderCell(_("Amount"), "text-right");
                    hb.addTableHeaderCell(_("Frequency"));
                    hb.addTableHeaderCell(_("Estimated"), "text-right");
                    hb.addTableHeaderCell(_("Actual"), "text-right");
                }
                hb.endTableRow();
            }
            hb.endThead();
            hb.startTbody();
            {
                categID = -1;
                double catTotalsEstimated = 0.0, catTotalsActual = 0.0;

                for (const auto& category : categs)
                {
                    double estimated = Model_Budget::getEstimate(monthlyBudget
                        , budgetPeriod[category.second.first][category.second.second]
                        , budgetAmt[category.second.first][category.second.second]);

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
                        Model_Category::Data *c = Model_Category::instance().get(categID);
                        amply ? hb.startAltTableRow() : hb.startTableRow();
                        {
                            wxString categName = "";
                            if (c) categName = c->CATEGNAME;
                            hb.addTableCellLink(wxString::Format("viewtrans:%d:-2"
                                                            , c->CATEGID)
                                                            , categName);
                            hb.addTableCell(wxEmptyString);
                            hb.addTableCell(wxEmptyString);
                            hb.addMoneyCell(catTotalsEstimated);
                            hb.addMoneyCell(catTotalsActual);
                        }
                        hb.endTableRow();

                        catTotalsEstimated = catTotalsActual = 0.0;
                    }

                    catTotalsActual += actual;
                    catTotalsEstimated += estimated;

                    /***************************************************************************/
                    if (amply && category.second.first != -1)
                    {
                        double amt = budgetAmt[category.second.first][category.second.second];
                        hb.startTableRow();
                        {
                            hb.addTableCellLink(wxString::Format("viewtrans:%d:%d"
                                                                , category.second.first
                                                                , category.second.second)
                                                                , category.first);
                            hb.addMoneyCell(amt);
                            hb.addTableCell(Model_Budget::all_period()[budgetPeriod[category.second.first][category.second.second]]);
                            hb.addMoneyCell(estimated);
                            hb.addMoneyCell(actual);
                        }
                        hb.endTableRow();
                    }
                    categID = category.second.first;
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
