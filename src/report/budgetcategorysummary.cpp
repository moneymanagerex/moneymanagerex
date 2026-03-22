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

#include "base/defs.h"
#include "mmex.h"
#include "util/mmDateRange.h"
#include "htmlbuilder.h"

#include "model/BudgetPeriodModel.h"
#include "model/BudgetModel.h"
#include "model/CategoryModel.h"
#include "mmframe.h"
#include "budgetcategorysummary.h"

mmReportBudgetCategorySummary::mmReportBudgetCategorySummary()
{
    m_chart_selection = 1;
    setReportParameters(REPORT_ID::BudgetCategorySummary);
}

mmReportBudgetCategorySummary::~mmReportBudgetCategorySummary()
{}

wxString mmReportBudgetCategorySummary::getHTMLText()
{
    // Grab the data 
    int startDay;
    wxDateTime::Month startMonth;
    if (PrefModel::instance().getBudgetFinancialYears()) {
        GetFinancialYearValues(startDay, startMonth);
    }
    else {
        startDay = 1;
        startMonth = wxDateTime::Jan;
    }

    long tmp;
    int startYear = wxDateTime::Today().GetYear();

    wxString bp_name_n = BudgetPeriodModel::instance().get_id_name_n(m_date_selection);
    wxString budget_month;
    wxString budget_year = bp_name_n;

    wxRegEx pattern("^([0-9]{4})(-([0-9]{2}))?$");
    if (pattern.Matches(bp_name_n)) {
        budget_year = pattern.GetMatch(bp_name_n, 1);
        budget_month = pattern.GetMatch(bp_name_n, 3);
    }

    if (budget_year.ToLong(&tmp))
        startYear = static_cast<int>(tmp); // 0 <= tmp <= 9999

    long budgetMonth = 0;
    if (budget_month.ToLong(&budgetMonth)) {
        if (startMonth != wxDateTime::Jan)
            startMonth = wxDateTime(1, startMonth, startYear).Add(
                wxDateSpan::Months(--budgetMonth)
            ).GetMonth();
        else
            startMonth = static_cast<wxDateTime::Month>(--budgetMonth);
    }

    bool monthlyBudget = (!budget_month.empty());

    budget_year = monthlyBudget
        ? wxString::Format("%i-%ld", startYear, budgetMonth)
        : wxString::Format("%d", startYear);

    mmDate yearBegin = wxDateTime(startDay, startMonth, startYear);
    mmDate yearEnd = monthlyBudget
        ? yearBegin.plusDateSpan(wxDateSpan::Month()).minusDateSpan(wxDateSpan::Day())
        : yearBegin.plusDateSpan(wxDateSpan::Year()).minusDateSpan(wxDateSpan::Day());

    // Readjust dates by the Budget Offset Option
    PrefModel::instance().addBudgetDateOffset(yearBegin);
    PrefModel::instance().addBudgetDateOffset(yearEnd);
    mmSpecifiedRange date_range(yearBegin.getDateTime(), yearEnd.getDateTime());

    bool evaluateTransfer = false;
    if (PrefModel::instance().getBudgetIncludeTransfers()) {
        evaluateTransfer = true;
    }
    //Get statistics
    std::map<int64, BudgetFreq> budgetFreq;
    std::map<int64, double> budgetAmt;
    std::map<int64, wxString> budgetNotes;
    BudgetModel::instance().getBudgetEntry(m_date_selection, budgetFreq, budgetAmt, budgetNotes);

    std::map<int64, std::map<int, double> > categoryStats;
    CategoryModel::instance().getCategoryStats(
        categoryStats,
        static_cast<wxSharedPtr<wxArrayString>>(nullptr),
        &date_range,
        PrefModel::instance().getIgnoreFutureTransactions(),
        false,
        (evaluateTransfer ? &budgetAmt : nullptr)
    );

    std::map<int64, std::map<int, double> > budgetStats;
    BudgetModel::instance().getBudgetStats(budgetStats, &date_range, monthlyBudget);

    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    wxString headingStr = AdjustYearValues(startDay, startMonth, startYear, budget_year);
    bool amply = PrefModel::instance().getBudgetSummaryWithoutCategories();
    headingStr = wxString::Format(
        amply
            ? _t("Budget Categories for %s")
            : _t("Budget Category Summary for %s"),
        headingStr + "<br>" + _t("(Estimated vs. Actual)")
    );
    hb.addReportHeader(headingStr, 1, PrefModel::instance().getIgnoreFutureTransactions());
    hb.displayDateHeading(yearBegin.getDateTime(), yearEnd.getDateTime());
    // Prime the filter
    m_filter.clear();
    m_filter.setDateRange(yearBegin, yearEnd);

    CategoryModel::DataA cat_a = CategoryModel::instance().find(CategoryCol::PARENTID(-1));
    std::stable_sort(cat_a.begin(), cat_a.end(), CategoryData::SorterByCATEGNAME());

    // Chart
    if (getChartSelection() == 0) {
        GraphData gd;
        GraphSeries gsActual, gsEstimated;

        for (const auto& cat_d : cat_a) {
            gsEstimated.name = _t("Estimated");
            gsActual.name = _t("Actual");
            gd.title = cat_d.m_name;
            gd.labels.push_back(cat_d.m_name);
            gsActual.values.push_back(categoryStats[cat_d.m_id][0]);
            gsEstimated.values.push_back(budgetStats[cat_d.m_id][budgetMonth]);
            for (const auto& sub_d : CategoryModel::instance().find_data_subtree_a(cat_d)){
                gd.labels.push_back(CategoryModel::instance().get_id_fullname(sub_d.m_id));
                gsActual.values.push_back(categoryStats[sub_d.m_id][0]);
                gsEstimated.values.push_back(budgetStats[sub_d.m_id][budgetMonth]);
            }

            // Bar/Line are best with at least 2 items 
            if (gd.labels.size() > 1) {
                gd.type = GraphData::BARLINE;
                gsEstimated.type = "column";
                gsActual.type = "line";
            }
            else {
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
                    hb.addTableHeaderCell(_t("Category"));
                    hb.addTableHeaderCell(_t("Estimated"), "text-right");
                    hb.addTableHeaderCell(_t("Actual"), "text-right");
                }
                hb.endTableRow();
            }
            hb.endThead();
            hb.startTbody();
            {

    // -- reduce indentation by 3 tab stops to make more space
    std::map<int64, double> catTotalsEstimated, catTotalsActual;
    std::map<int64, std::pair<int, wxString>> categLevel;
    for (const auto& cat_d : cat_a) {
        categLevel[cat_d.m_id].first = 0;
        double estimated = budgetStats[cat_d.m_id][budgetMonth];

        if (estimated < 0)
            estExpenses += estimated;
        else
            estIncome += estimated;

        double actual = categoryStats[cat_d.m_id][0];
        if (actual < 0)
            actExpenses += actual;
        else
            actIncome += actual;

        catTotalsActual[cat_d.m_id] += actual;
        catTotalsEstimated[cat_d.m_id] += estimated;

        if (amply) {
            hb.startTableRow();
            {
                hb.addTableCellLink(
                    wxString::Format("viewtrans:%lld",
                        cat_d.m_id
                    ),
                    cat_d.m_name
                );
                hb.addMoneyCell(estimated);
                hb.addMoneyCell(actual);
            }
            hb.endTableRow();
        }
        
        std::vector<int> totals_stack;
        CategoryModel::DataA sub_a = CategoryModel::instance().find_data_subtree_a(cat_d);
        for (int i = 0; i < static_cast<int>(sub_a.size()); i++) {
            categLevel[sub_a[i].m_id].first = 1;
            estimated = budgetStats[sub_a[i].m_id][budgetMonth];

            if (estimated < 0)
                estExpenses += estimated;
            else
                estIncome += estimated;

            actual = categoryStats[sub_a[i].m_id][0];
            if (actual < 0)
                actExpenses += actual;
            else
                actIncome += actual;

            //save totals for this subcategory
            catTotalsEstimated[sub_a[i].m_id] = estimated;
            catTotalsActual[sub_a[i].m_id] = actual;

            //update totals of the category
            catTotalsEstimated[cat_d.m_id] += estimated;
            catTotalsActual[cat_d.m_id] += actual;

            //walk up the hierarchy and update all the parent totals as well
            int64 nextParent = sub_a[i].m_parent_id_n;
            for (int j = i; j > 0; j--) {
                if (sub_a[j - 1].m_id == nextParent) {
                    categLevel[sub_a[i].m_id].first++;
                    catTotalsEstimated[sub_a[j - 1].m_id] += estimated;
                    catTotalsActual[sub_a[j - 1].m_id] += actual;
                    nextParent = sub_a[j - 1].m_parent_id_n;
                    if (nextParent == cat_d.m_id)
                        break;
                }
            }
            categLevel[sub_a[i].m_id].second = "";
            for (int j = categLevel[sub_a[i].m_id].first; j > 0; j--) {
                categLevel[sub_a[i].m_id].second.Prepend("&nbsp;&nbsp;&nbsp;&nbsp;");
            }
            if (amply) {
                hb.startTableRow();
                {
                    hb.addTableCell(wxString::Format(
                        categLevel[sub_a[i].m_id].second + "<a href=\"viewtrans:%lld\" target=\"_blank\">%s</a>",
                        sub_a[i].m_id,
                        sub_a[i].m_name
                    ));
                    hb.addMoneyCell(estimated);
                    hb.addMoneyCell(actual);
                }
                hb.endTableRow();
                
                //not the last subcategory
                if (i < static_cast<int>(sub_a.size()) - 1) {
                    //if next subcategory is our child, queue the total for after the children
                    if (sub_a[i].m_id == sub_a[i + 1].m_parent_id_n) {
                        totals_stack.push_back(i);
                    }
                    // last sibling -- we've exhausted this branch, so display all the totals we held on to
                    else if (sub_a[i].m_parent_id_n != sub_a[i + 1].m_parent_id_n) {
                        while (!totals_stack.empty() &&
                            sub_a[totals_stack.back()].m_id != sub_a[i + 1].m_parent_id_n
                        ) {
                            hb.startAltTableRow();
                            {
                                int index = totals_stack.back();
                                hb.addTableCell(wxString::Format(
                                    categLevel[sub_a[index].m_id].second + "<a href=\"viewtrans:%lld:-2\" target=\"_blank\">%s</a>",
                                    sub_a[index].m_id,
                                    sub_a[index].m_name
                                ));
                                hb.addMoneyCell(catTotalsEstimated[sub_a[index].m_id]);
                                hb.addMoneyCell(catTotalsActual[sub_a[index].m_id]);
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
                            hb.addTableCell(wxString::Format(
                                categLevel[sub_a[index].m_id].second + "<a href=\"viewtrans:%lld:-2\" target=\"_blank\">%s</a>",
                                sub_a[index].m_id,
                                sub_a[index].m_name
                            ));
                            hb.addMoneyCell(catTotalsEstimated[sub_a[index].m_id]);
                            hb.addMoneyCell(catTotalsActual[sub_a[index].m_id]);
                        }
                        hb.endTableRow();
                        totals_stack.pop_back();
                    }
                }
            }
        }
        amply ? hb.startAltTableRow() : hb.startTableRow();
        {
            hb.addTableCellLink(
                wxString::Format("viewtrans:%lld:-2", cat_d.m_id),
                cat_d.m_name
            );
            hb.addMoneyCell(catTotalsEstimated[cat_d.m_id]);
            hb.addMoneyCell(catTotalsActual[cat_d.m_id]);
        }
        hb.endTableRow();
    }
    // -- restore indentation

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
                    hb.addTableCell(_t("Estimated Income:"));
                    hb.addMoneyCell(estIncome);
                    hb.addTableCell(_t("Actual Income:"));
                    hb.addMoneyCell(actIncome);
                    hb.addTableCell(_t("Difference Income:"));
                    hb.addMoneyCell(difIncome);
                }
                hb.endTableRow();

                hb.startTotalTableRow();
                {
                    hb.addTableCell(_t("Estimated Expenses:"));
                    hb.addMoneyCell(estExpenses);
                    hb.addTableCell(_t("Actual Expenses:"));
                    hb.addMoneyCell(actExpenses);
                    hb.addTableCell(_t("Difference Expenses:"));
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
