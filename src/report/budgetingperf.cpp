/*******************************************************
Copyright (C) 2006-2012 Nikolay Akimov
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
********************************************************/

#include <string>

#include "base/defs.h"
#include "mmex.h"
#include "util/mmDateRange.h"
#include "htmlbuilder.h"

#include "model/BudgetPeriodModel.h"
#include "model/BudgetModel.h"
#include "model/CategoryModel.h"
#include "mmframe.h"
#include "budgetingperf.h"

mmReportBudgetingPerformance::mmReportBudgetingPerformance()
{
    setReportParameters(REPORT_ID::BudgetPerformance);
}

mmReportBudgetingPerformance::~mmReportBudgetingPerformance()
{}


wxString mmReportBudgetingPerformance::getHTMLText()
{

    int startDay;
    wxDateTime::Month startMonth;
    if (PrefModel::instance().getBudgetFinancialYears()) {
        GetFinancialYearValues(startDay, startMonth);
    } else {
        startDay = 1;
        startMonth = wxDateTime::Jan;
    }

    long startYear;

    wxString bp_name_n = BudgetPeriodModel::instance().get_id_name_n(m_date_selection);
    wxString budget_year;
    wxString budget_month;

    wxRegEx pattern("^([0-9]{4})(-([0-9]{2}))?$");
    if (pattern.Matches(bp_name_n)) {
        budget_year = pattern.GetMatch(bp_name_n, 1);
        budget_month = pattern.GetMatch(bp_name_n, 3);
    }

    if (!budget_year.ToLong(&startYear)) {
        startYear = static_cast<long>(wxDateTime::Today().GetYear());
        budget_year = wxString::Format("%ld", startYear);
    }

    mmDate yearBegin = wxDateTime(startDay, startMonth, static_cast<int>(startYear));
    mmDate yearEnd = yearBegin.plusDateSpan(wxDateSpan::Year()).minusDateSpan(wxDateSpan::Day());

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
        m_account_a,
        &date_range,
        PrefModel::instance().getIgnoreFutureTransactions(),
        true,
        (evaluateTransfer ? &budgetAmt : nullptr),
        PrefModel::instance().getBudgetFinancialYears()
    );

    std::map<int64, std::map<int, double> > budgetStats;
    BudgetModel::instance().getBudgetStats(budgetStats, &date_range, true);

    //Totals
    std::map<int64, double> actualTotal;
    std::map<int64, double> estimateTotal;

    const wxString& headingStr = wxString::Format(_t("Budget Performance for %s"),
        AdjustYearValues(startDay, startMonth, startYear, budget_year)
    );
    mmHTMLBuilder hb;
    hb.init();

    hb.addReportHeader(headingStr, 1, PrefModel::instance().getIgnoreFutureTransactions());
    hb.displayDateHeading(yearBegin.getDateTime(), yearEnd.getDateTime());
    // Prime the filter
    m_filter.clear();
    m_filter.setDateRange(yearBegin, yearEnd);
    m_filter.setAccountList(m_account_a);

    hb.addDivContainer("shadow");
    {
        hb.startTable();
        {
            hb.startThead();
            {
                hb.startTableRow();
                {
                    hb.addTableHeaderCell(_t("Category"));
                    for (int yidx = 0; yidx < 12; yidx++)
                    {
                        actualTotal[yidx] = 0;
                        estimateTotal[yidx] = 0;
                        int m = startMonth + yidx;
                        if (m >= 12) m -= 12;
                        hb.addTableHeaderCell(wxGetTranslation(
                            wxDateTime::GetEnglishMonthName(wxDateTime::Month(m)
                                , wxDateTime::Name_Abbr)), "text-center", 1);
                    }
                    hb.addTableHeaderCell(_t("Total"), "text-center", 2);
                }
                hb.endTableRow();
                hb.startTableRow();
                {
                    hb.addEmptyTableCell();
                    for (int yidx = 0; yidx < 12; yidx++)
                    {
                        hb.addTableCell(_t("Est.") + "<BR>" + _t("Act."), false, false);
                    }
                    hb.addTableCell(_t("Est.") + "<BR>" + _t("Act."), false, false);
                    hb.addTableCell("%", false, false);
                }
                hb.endTableRow();
            }
            hb.endThead();
            hb.startTbody();
            {
                std::map<int64, std::map<int, double>> catTotalsEstimated, catTotalsActual;
                std::map<int64, wxString> formattedNames;
                std::map<int64, std::vector<CategoryData>> categ_children;

                bool budgetDeductMonthly = PrefModel::instance().getBudgetDeductMonthly();
                // pull categories from DB and store
                for (CategoryData category : CategoryModel::instance().find_all(CategoryCol::COL_ID_CATEGNAME, false)) {
                    categ_children[category.m_parent_id_n].push_back(category);
                }

                std::vector<CategoryData> totals_stack;
                std::vector<CategoryData> categ_stack = categ_children[-1];
                while (!categ_stack.empty())
                {
                    CategoryData category = categ_stack.back();
                    categ_stack.pop_back();
                    int64 catID = category.m_id;
                    double estimate = 0;
                    double actual = 0;
                    for (auto child : categ_children[catID]) {
                        formattedNames[child.m_id] = wxString("&nbsp;&nbsp;&nbsp;&nbsp;").Append(formattedNames[catID]);
                        categ_stack.push_back(child);
                    }
                    int month = -1;
                    hb.startTableRow();
                    hb.startTableCell(" style='vertical-align:middle;'");
                    hb.addText(wxString::Format(formattedNames[catID] + "<a href=\"viewtrans:%lld\" target=\"_blank\">%s</a>"
                        , catID
                        , category.m_name));
                    hb.endTableCell();
                    for (const auto& stat : categoryStats[catID])
                    {
                        month++;

                        hb.startTableCell(" style='text-align:right;' nowrap");

                        estimate = budgetStats[catID][stat.first];
                        actual = stat.second;

                        estimateTotal[month] += estimate;
                        actualTotal[month] += actual;

                        actualTotal[12] += actual;
                        estimateTotal[12] += estimate;;

                        wxString estimateVal = CurrencyModel::instance().toString(estimate, CurrencyModel::instance().get_base_data_n());
                        //make the href string parameters
                        std::stringstream ss;
                        ss << "budget:" << estimateVal << "|" << CurrencyModel::instance().toString(actual, CurrencyModel::instance().get_base_data_n()) << "|" << catID << "|"
                           << budget_year << "|" << month + 1;
                        std::string editBudgetEntry = ss.str();

                        // If monthly budget is deducted and the monthly budgets have exceeded the yearly budget, show estimate in red color
                        //+ add link to budget dlg
                        hb.startSpan((budgetDeductMonthly && estimate != 0 && round(estimateTotal[12] / budgetStats[catID][12] * 100) / 100 > 1)
                                         ? hb.getFormattedLink("red",editBudgetEntry,estimateVal)
                                         : hb.getFormattedLink("", editBudgetEntry, estimateVal),
                                     wxString::Format(" style='text-align:right;%s' nowrap", ""));
                        hb.endSpan();
                        hb.addLineBreak();

                        hb.startSpan(CurrencyModel::instance().toString(actual, CurrencyModel::instance().get_base_data_n()), wxString::Format(" style='text-align:right;%s' nowrap"
                            , (actual - estimate < 0) ? "color:red;" : ""));
                        hb.endSpan();

                        hb.endTableCell();

                        //save totals for this category
                        catTotalsEstimated[catID][month] = estimate;
                        catTotalsActual[catID][month] = actual;
                        catTotalsEstimated[catID][12] += estimate;
                        catTotalsActual[catID][12] += actual;

                        //update all the ancestor totals
                        for (auto i = totals_stack.rbegin(); i < totals_stack.rend(); i++)
                        {
                            catTotalsEstimated[i->m_id][month] += estimate;
                            catTotalsActual[i->m_id][month] += actual;
                            catTotalsEstimated[i->m_id][12] += estimate;
                            catTotalsActual[i->m_id][12] += actual;
                        }
                    }

                    // year end
                    hb.startTableCell(" style='text-align:right;' nowrap");
                    hb.addText(CurrencyModel::instance().toString(catTotalsEstimated[catID][12], CurrencyModel::instance().get_base_data_n()));
                    hb.addLineBreak();

                    hb.addText(CurrencyModel::instance().toString(catTotalsActual[catID][12], CurrencyModel::instance().get_base_data_n()));
                    hb.endTableCell();

                    if (catTotalsEstimated[catID][12] != 0)
                    {
                        double percent = (catTotalsActual[catID][12] / catTotalsEstimated[catID][12]) * 100.0;
                        hb.addTableCell(wxString::Format("%.1f", percent), true);
                    }
                    else
                        hb.addTableCell("-");
                    hb.endTableRow();

                    if (!categ_stack.empty() && categ_stack.back().m_parent_id_n == catID)
                        totals_stack.push_back(category); //if next subcategory is our child, store the parent to display after the children
                    else
                        while (!totals_stack.empty() && !categ_stack.empty() && totals_stack.back().m_id != categ_stack.back().m_parent_id_n) {
                            hb.startAltTableRow();
                            {
                                int64 id = totals_stack.back().m_id;
                                hb.startTableCell(" style='vertical-align:middle;'");
                                hb.addText(wxString::Format(formattedNames[id] + "<a href=\"viewtrans:%lld:-2\" target=\"_blank\">%s</a>"
                                    , id
                                    , totals_stack.back().m_name));
                                hb.endTableCell();
                                // monthly totals
                                for (int m = 0; m < 12; m++)
                                {
                                    hb.startTableCell(" style='text-align:right;' nowrap");
                                    hb.addText(CurrencyModel::instance().toString(catTotalsEstimated[id][m], CurrencyModel::instance().get_base_data_n()));
                                    hb.addLineBreak();
                                    hb.startSpan(CurrencyModel::instance().toString(catTotalsActual[id][m], CurrencyModel::instance().get_base_data_n()), wxString::Format(" style='text-align:right;%s' nowrap"
                                        , (catTotalsActual[id][m] - catTotalsEstimated[id][m] < 0) ? "color:red;" : ""));
                                    hb.endSpan();

                                    hb.endTableCell();
                                }
                                // year total
                                hb.startTableCell(" style='text-align:right;' nowrap");
                                hb.addText(CurrencyModel::instance().toString(catTotalsEstimated[id][12], CurrencyModel::instance().get_base_data_n()));
                                hb.addLineBreak();

                                hb.addText(CurrencyModel::instance().toString(catTotalsActual[id][12], CurrencyModel::instance().get_base_data_n()));
                                hb.endTableCell();

                                if (catTotalsEstimated[id][12] != 0)
                                {
                                    double percent = (catTotalsActual[id][12] / catTotalsEstimated[id][12]) * 100.0;
                                    hb.addTableCell(wxString::Format("%.1f", percent), true);
                                }
                                else
                                    hb.addTableCell("-");
                            }
                            hb.endTableRow();
                            totals_stack.pop_back();
                        }
                    estimateTotal[12] = 0;  // reset estimateTotal for new category
                }

                // the very last subcategory, so show the rest of the pending totals
                while (!totals_stack.empty())
                {
                    hb.startAltTableRow();
                    {
                        int64 id = totals_stack.back().m_id;
                        hb.startTableCell(" style='vertical-align:middle;'");
                        hb.addText(wxString::Format(formattedNames[id] + "<a href=\"viewtrans:%lld:-2\" target=\"_blank\">%s</a>"
                            , id
                            , totals_stack.back().m_name));
                        hb.endTableCell();
                        // monthly totals
                        for (int m = 0; m < 12; m++)
                        {
                            hb.startTableCell(" style='text-align:right;' nowrap");
                            hb.addText(CurrencyModel::instance().toString(catTotalsEstimated[id][m], CurrencyModel::instance().get_base_data_n()));
                            hb.addLineBreak();
                            hb.startSpan(CurrencyModel::instance().toString(catTotalsActual[id][m], CurrencyModel::instance().get_base_data_n()), wxString::Format(" style='text-align:right;%s' nowrap"
                                , (catTotalsActual[id][m] - catTotalsEstimated[id][m] < 0) ? "color:red;" : ""));
                            hb.endSpan();

                            hb.endTableCell();
                        }
                        // year total
                        hb.startTableCell(" style='text-align:right;' nowrap");
                        hb.addText(CurrencyModel::instance().toString(catTotalsEstimated[id][12], CurrencyModel::instance().get_base_data_n()));
                        hb.addLineBreak();

                        hb.addText(CurrencyModel::instance().toString(catTotalsActual[id][12], CurrencyModel::instance().get_base_data_n()));
                        hb.endTableCell();
                        if (catTotalsEstimated[id][12] != 0)
                        {
                            double percent = (catTotalsActual[id][12] / catTotalsEstimated[id][12]) * 100.0;
                            hb.addTableCell(wxString::Format("%.1f", percent), true);
                        }
                        else
                            hb.addTableCell("-");
                    }
                    hb.endTableRow();
                    totals_stack.pop_back();
                }
                hb.endTbody();
                hb.startTfoot();
                {
                    hb.startTotalTableRow();

                    hb.addTableCell(wxString::Format("%s<br>%s<br>%s"
                        ,_t("Estimated:")
                        ,_t("Actual:")
                        ,_t("Difference: ")));

                    double estimateGrandTotal = 0;
                    double actualGrandTotal = 0;
                    for (int m = 0; m < 12; m++)
                    {
                        hb.startTableCell(" style='text-align:right;' nowrap");

                        const double estimate = estimateTotal[m];
                        wxString estimateVal = CurrencyModel::instance().toString(estimate, CurrencyModel::instance().get_base_data_n());
                        hb.addText(estimateVal);
                        hb.addLineBreak();

                        const double actual = actualTotal[m];
                        const auto actualVal = CurrencyModel::instance().toString(actual, CurrencyModel::instance().get_base_data_n());
                        hb.startSpan(actualVal, wxString::Format(" style='text-align:right;%s' nowrap"
                            , (actual - estimate < 0) ? "color:red;" : ""));
                        hb.endSpan();
                        hb.addLineBreak();

                        const double difference = actual - estimate;
                        const auto differenceVal = CurrencyModel::instance().toString(difference, CurrencyModel::instance().get_base_data_n());
                        hb.startSpan(differenceVal, wxString::Format(" style='text-align:right;%s' nowrap"
                            , (difference < 0) ? "color:red;" : ""));
                        hb.endSpan();
                        hb.addLineBreak();

                        if (estimate != 0)
                        {
                            double percent = (actual / estimate) * 100.0;
                            const auto percentVal = wxString::Format("%.1f%%", percent);
                            hb.startSpan(percentVal, wxString::Format(" style='text-align:right;%s' nowrap"
                                , (difference < 0) ? "color:red;" : ""));
                            hb.endSpan();
                        }

                        hb.endTableCell();
                        estimateGrandTotal += estimate;
                        actualGrandTotal += actual;
                    }
                    // Grand total end
                    const auto estimateVal = CurrencyModel::instance().toString(estimateGrandTotal, CurrencyModel::instance().get_base_data_n());
                    hb.startTableCell(" style='text-align:right;' nowrap");
                    hb.addText(estimateVal);
                    hb.addLineBreak();

                    const auto actualVal = CurrencyModel::instance().toString(actualGrandTotal, CurrencyModel::instance().get_base_data_n());
                    hb.addText(actualVal);
                    hb.addLineBreak();

                    const double differenceGrandTotal = actualGrandTotal - estimateGrandTotal;
                    const auto differenceVal = CurrencyModel::instance().toString(differenceGrandTotal, CurrencyModel::instance().get_base_data_n());
                    hb.startSpan(differenceVal, wxString::Format(" style='text-align:right;%s' nowrap"
                        , (differenceGrandTotal < 0) ? "color:red;" : ""));
                    hb.endSpan();
                    hb.addLineBreak();

                    if (estimateGrandTotal != 0)
                    {
                        double percentGrandTotal = (actualGrandTotal / estimateGrandTotal) * 100.0;
                        const auto percentVal = wxString::Format("%.1f%%", percentGrandTotal);
                        hb.startSpan(percentVal, wxString::Format(" style='text-align:right;%s' nowrap"
                            , (differenceGrandTotal < 0) ? "color:red;" : ""));
                        hb.endSpan();
                    }
                    hb.endTableCell();
                    hb.addEmptyTableCell();
                    hb.endTableRow();
                }
                hb.endTfoot();
            }
        }
        hb.endTable();
    }
    hb.endDiv();
    hb.end();

    return hb.getHTMLText();
}
