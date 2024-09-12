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

#include "budgetingperf.h"
#include "reports/htmlbuilder.h"
#include "mmex.h"
#include "mmframe.h"
#include "model/Model_Budgetyear.h"
#include "model/Model_Budget.h"
#include "model/Model_Category.h"
#include "reports/mmDateRange.h"

mmReportBudgetingPerformance::mmReportBudgetingPerformance()
{
    setReportParameters(Reports::BudgetPerformance);
}

mmReportBudgetingPerformance::~mmReportBudgetingPerformance()
{}


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

    wxString value = Model_Budgetyear::instance().Get(m_date_selection);
    wxString budget_year;
    wxString budget_month;

    wxRegEx pattern("^([0-9]{4})(-([0-9]{2}))?$");
    if (pattern.Matches(value))
    {
        budget_year = pattern.GetMatch(value, 1);
        budget_month = pattern.GetMatch(value, 3);
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
    std::map<int, Model_Budget::PERIOD_ID> budgetPeriod;
    std::map<int, double> budgetAmt;
    std::map<int, wxString> budgetNotes;
    Model_Budget::instance().getBudgetEntry(m_date_selection, budgetPeriod, budgetAmt, budgetNotes);

    std::map<int, std::map<int, double> > categoryStats;
    Model_Category::instance().getCategoryStats(categoryStats
        , accountArray_
        , &date_range
        , Option::instance().getIgnoreFutureTransactions()
        , true
        , (evaluateTransfer ? &budgetAmt : nullptr)
        , Option::instance().BudgetFinancialYears());

    std::map<int, std::map<int, double> > budgetStats;
    Model_Budget::instance().getBudgetStats(budgetStats, &date_range, true);

    //Totals
    std::map<int, double> actualTotal;
    std::map<int, double> estimateTotal;

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
                        actualTotal[yidx] = 0;
                        estimateTotal[yidx] = 0;
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
                        hb.addTableCell(_("Est.") + "<BR>" + _("Act."), false, false);
                    }
                    hb.addTableCell(_("Est.") + "<BR>" + _("Act."), false, false);
                    hb.addTableCell(_("%"), false, false);
                }
                hb.endTableRow();
            }
            hb.endThead();
            hb.startTbody();
            {
                std::map<int, std::map<int, double>> catTotalsEstimated, catTotalsActual;
                std::map<int, wxString> formattedNames;
                std::map<int, std::vector<Model_Category::Data>> categ_children;

                bool budgetDeductMonthly = Option::instance().BudgetDeductMonthly();
                // pull categories from DB and store
                for (Model_Category::Data category : Model_Category::instance().all(Model_Category::COL_CATEGNAME, false)) {
                    categ_children[category.PARENTID].push_back(category);
                }

                std::vector<Model_Category::Data> totals_stack;
                std::vector<Model_Category::Data> categ_stack = categ_children[-1];
                while (!categ_stack.empty())
                {
                    Model_Category::Data category = categ_stack.back();
                    categ_stack.pop_back();
                    int catID = category.CATEGID;
                    double estimate = 0;
                    double actual = 0;
                    for (auto child : categ_children[catID]) {
                        formattedNames[child.CATEGID] = wxString("&nbsp;&nbsp;&nbsp;&nbsp;").Append(formattedNames[catID]);
                        categ_stack.push_back(child);
                    }
                    int month = -1;
                    hb.startTableRow();
                    hb.startTableCell(" style='vertical-align:middle;'");
                    hb.addText(wxString::Format(formattedNames[catID] + "<a href=\"viewtrans:%d\" target=\"_blank\">%s</a>"
                        , catID
                        , category.CATEGNAME));
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

                        wxString estimateVal = Model_Currency::toString(estimate, Model_Currency::GetBaseCurrency());
                        // If monthly budget is deducted and the monthly budgets have exceeded the yearly budget, show estimate in red color
                        hb.startSpan(estimateVal, wxString::Format(" style='text-align:right;%s' nowrap"
                            , (budgetDeductMonthly && estimate != 0 && round(estimateTotal[12]/budgetStats[catID][12] * 100)/100 > 1) ? "color:red;" : ""));
                        hb.endSpan();
                        hb.addLineBreak();

                        hb.startSpan(Model_Currency::toString(actual, Model_Currency::GetBaseCurrency()), wxString::Format(" style='text-align:right;%s' nowrap"
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
                            catTotalsEstimated[i->CATEGID][month] += estimate;
                            catTotalsActual[i->CATEGID][month] += actual;
                            catTotalsEstimated[i->CATEGID][12] += estimate;
                            catTotalsActual[i->CATEGID][12] += actual;
                        }
                    }

                    // year end
                    hb.startTableCell(" style='text-align:right;' nowrap");
                    hb.addText(Model_Currency::toString(catTotalsEstimated[catID][12], Model_Currency::GetBaseCurrency()));
                    hb.addLineBreak();

                    hb.addText(Model_Currency::toString(catTotalsActual[catID][12], Model_Currency::GetBaseCurrency()));
                    hb.endTableCell();

                    if (catTotalsEstimated[catID][12] != 0)
                    {
                        double percent = (catTotalsActual[catID][12] / catTotalsEstimated[catID][12]) * 100.0;
                        hb.addTableCell(wxString::Format("%.1f", percent), true);
                    }
                    else
                        hb.addTableCell("-");
                    hb.endTableRow();

                    if (!categ_stack.empty() && categ_stack.back().PARENTID == catID)
                        totals_stack.push_back(category); //if next subcategory is our child, store the parent to display after the children
                    else
                        while (!totals_stack.empty() && !categ_stack.empty() && totals_stack.back().CATEGID != categ_stack.back().PARENTID) {
                            hb.startAltTableRow();
                            {
                                int id = totals_stack.back().CATEGID;
                                hb.startTableCell(" style='vertical-align:middle;'");
                                hb.addText(wxString::Format(formattedNames[id] + "<a href=\"viewtrans:%d:-2\" target=\"_blank\">%s</a>"
                                    , id
                                    , totals_stack.back().CATEGNAME));
                                hb.endTableCell();
                                // monthly totals
                                for (int m = 0; m < 12; m++)
                                {
                                    hb.startTableCell(" style='text-align:right;' nowrap");
                                    hb.addText(Model_Currency::toString(catTotalsEstimated[id][m], Model_Currency::GetBaseCurrency()));
                                    hb.addLineBreak();
                                    hb.startSpan(Model_Currency::toString(catTotalsActual[id][m], Model_Currency::GetBaseCurrency()), wxString::Format(" style='text-align:right;%s' nowrap"
                                        , (catTotalsActual[id][m] - catTotalsEstimated[id][m] < 0) ? "color:red;" : ""));
                                    hb.endSpan();

                                    hb.endTableCell();
                                }
                                // year total
                                hb.startTableCell(" style='text-align:right;' nowrap");
                                hb.addText(Model_Currency::toString(catTotalsEstimated[id][12], Model_Currency::GetBaseCurrency()));
                                hb.addLineBreak();

                                hb.addText(Model_Currency::toString(catTotalsActual[id][12], Model_Currency::GetBaseCurrency()));
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
                }

                // the very last subcategory, so show the rest of the pending totals
                while (!totals_stack.empty())
                {
                    hb.startAltTableRow();
                    {
                        int id = totals_stack.back().CATEGID;
                        hb.startTableCell(" style='vertical-align:middle;'");
                        hb.addText(wxString::Format(formattedNames[id] + "<a href=\"viewtrans:%d:-2\" target=\"_blank\">%s</a>"
                            , id
                            , totals_stack.back().CATEGNAME));
                        hb.endTableCell();
                        // monthly totals
                        for (int m = 0; m < 12; m++)
                        {
                            hb.startTableCell(" style='text-align:right;' nowrap");
                            hb.addText(Model_Currency::toString(catTotalsEstimated[id][m], Model_Currency::GetBaseCurrency()));
                            hb.addLineBreak();
                            hb.startSpan(Model_Currency::toString(catTotalsActual[id][m], Model_Currency::GetBaseCurrency()), wxString::Format(" style='text-align:right;%s' nowrap"
                                , (catTotalsActual[id][m] - catTotalsEstimated[id][m] < 0) ? "color:red;" : ""));
                            hb.endSpan();

                            hb.endTableCell();
                        }
                        // year total
                        hb.startTableCell(" style='text-align:right;' nowrap");
                        hb.addText(Model_Currency::toString(catTotalsEstimated[id][12], Model_Currency::GetBaseCurrency()));
                        hb.addLineBreak();

                        hb.addText(Model_Currency::toString(catTotalsActual[id][12], Model_Currency::GetBaseCurrency()));
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
                        ,_("Estimated:")
                        ,_("Actual:")
                        ,_("Difference: ")));

                    double estimateGrandTotal = 0;
                    double actualGrandTotal = 0;
                    for (int m = 0; m < 12; m++)
                    {
                        hb.startTableCell(" style='text-align:right;' nowrap");

                        const double estimate = estimateTotal[m];
                        wxString estimateVal = Model_Currency::toString(estimate, Model_Currency::GetBaseCurrency());
                        hb.addText(estimateVal);
                        hb.addLineBreak();

                        const double actual = actualTotal[m];
                        const auto actualVal = Model_Currency::toString(actual, Model_Currency::GetBaseCurrency());
                        hb.startSpan(actualVal, wxString::Format(" style='text-align:right;%s' nowrap"
                            , (actual - estimate < 0) ? "color:red;" : ""));
                        hb.endSpan();
                        hb.addLineBreak();

                        const double difference = actual - estimate;
                        const auto differenceVal = Model_Currency::toString(difference, Model_Currency::GetBaseCurrency());
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
                    const auto estimateVal = Model_Currency::toString(estimateGrandTotal, Model_Currency::GetBaseCurrency());
                    hb.startTableCell(" style='text-align:right;' nowrap");
                    hb.addText(estimateVal);
                    hb.addLineBreak();

                    const auto actualVal = Model_Currency::toString(actualGrandTotal, Model_Currency::GetBaseCurrency());
                    hb.addText(actualVal);
                    hb.addLineBreak();

                    const double differenceGrandTotal = actualGrandTotal - estimateGrandTotal;
                    const auto differenceVal = Model_Currency::toString(differenceGrandTotal, Model_Currency::GetBaseCurrency());
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
