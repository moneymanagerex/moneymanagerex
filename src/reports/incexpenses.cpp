/*******************************************************
Copyright (C) 2006-2012 Madhan Kanagavel
Copyright (C) 2017 James Higley
Copyright (C) 2021 - 2022 Mark Whalley (mark@ipx.co.uk)

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

#include "images_list.h"
#include "incexpenses.h"

#include "reports/htmlbuilder.h"
#include "util.h"
#include "reports/mmDateRange.h"

#include "model/Model_Account.h"
#include "model/Model_Checking.h"
#include "model/Model_CurrencyHistory.h"
#include "model/Model_Category.h"


mmReportIncomeExpenses::mmReportIncomeExpenses()
    : mmPrintableBase(wxTRANSLATE("Income vs Expenses Summary"))
{
    setReportParameters(Reports::IncomevsExpensesSummary);
}

mmReportIncomeExpenses::~mmReportIncomeExpenses()
{
}

wxString mmReportIncomeExpenses::getHTMLText()
{
    // Grab the data
    std::pair<double, double> income_expenses_pair;
    for (const auto& transaction : Model_Checking::instance().find(
        Model_Checking::TRANSDATE(m_date_range->start_date(), GREATER_OR_EQUAL)
        , Model_Checking::TRANSDATE(m_date_range->end_date(), LESS_OR_EQUAL)
        , Model_Checking::STATUS(Model_Checking::VOID_, NOT_EQUAL)))
    {
        // Do not include asset or stock transfers in income expense calculations.
        if (Model_Checking::foreignTransactionAsTransfer(transaction))
            continue;

        Model_Account::Data *account = Model_Account::instance().get(transaction.ACCOUNTID);
        if (accountArray_)
        {
            if (!account || wxNOT_FOUND == accountArray_->Index(account->ACCOUNTNAME))
                continue;
        }
        double convRate = 1;
        // We got this far, get the currency conversion rate for this account
        if (account) convRate = Model_CurrencyHistory::getDayRate(Model_Account::currency(account)->CURRENCYID,transaction.TRANSDATE);

        if (Model_Checking::type(transaction) == Model_Checking::DEPOSIT)
            income_expenses_pair.first += transaction.TRANSAMOUNT * convRate;
        else if (Model_Checking::type(transaction) == Model_Checking::WITHDRAWAL)
            income_expenses_pair.second += transaction.TRANSAMOUNT * convRate;
    }

    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    hb.addReportHeader(getReportTitle(), m_date_range->startDay(), m_date_range->isFutureIgnored());
    hb.DisplayDateHeading(m_date_range->start_date(), m_date_range->end_date(), m_date_range->is_with_date());
    hb.DisplayFooter(getAccountNames());

    // Chart
    GraphData gd;
    GraphSeries gs;

    gs.values = { income_expenses_pair.first };
    gs.name = _("Income");
    gd.series.push_back(gs);
    gs.values = { income_expenses_pair.second };
    gs.name = _("Expenses");
    gd.series.push_back(gs);

    gd.labels.push_back(m_date_range->local_title());

    if (!gd.series.empty())
    {
        gd.type = GraphData::BAR;
        gd.colors = { mmThemeMetaColour(meta::COLOR_REPORT_CREDIT)
                        , mmThemeMetaColour(meta::COLOR_REPORT_DEBIT) };  
        hb.addChart(gd);
    }

    hb.addDivContainer("shadow");
    {
        hb.startTable();
        {
            hb.startThead();
            {
                hb.startTableRow();
                    hb.addTableHeaderCell(_("Type"));
                    hb.addTableHeaderCell(_("Amount"), "text-right");
                hb.endTableRow();
            }
            hb.endThead();
            hb.startTbody();
            {
                hb.addTableRow(_("Income:"), income_expenses_pair.first);
                hb.addTableRow(_("Expenses:"), income_expenses_pair.second);
                hb.addTotalRow(_("Difference:"), 2, income_expenses_pair.first - income_expenses_pair.second);
            }
            hb.endTbody();
        }
        hb.endTable();
    }
    hb.endDiv(); 
    hb.end();

    wxLogDebug("======= mmReportIncomeExpenses:getHTMLText =======");
    wxLogDebug("%s", hb.getHTMLText());

    return hb.getHTMLText();
}

mmReportIncomeExpensesMonthly::mmReportIncomeExpensesMonthly()
    : mmPrintableBase(wxTRANSLATE("Income vs Expenses Monthly"))
{
    setReportParameters(Reports::IncomevsExpensesMonthly);
}

mmReportIncomeExpensesMonthly::~mmReportIncomeExpensesMonthly()
{
}

wxString mmReportIncomeExpensesMonthly::getHTMLText()
{
    // Grab the data
    std::map<int, std::pair<double, double> > incomeExpensesStats;
    //TODO: init all the map values with 0.0
    for (const auto& transaction : Model_Checking::instance().find(
        Model_Checking::TRANSDATE(m_date_range->start_date(), GREATER_OR_EQUAL)
        , Model_Checking::TRANSDATE(m_date_range->end_date(), LESS_OR_EQUAL)
        , Model_Checking::STATUS(Model_Checking::VOID_, NOT_EQUAL)))
    {
        // Do not include asset or stock transfers in income expense calculations.
        if (Model_Checking::foreignTransactionAsTransfer(transaction))
            continue;

        Model_Account::Data *account = Model_Account::instance().get(transaction.ACCOUNTID);
        if (accountArray_)
        {
            if (!account || wxNOT_FOUND == accountArray_->Index(account->ACCOUNTNAME))
                continue;
        }
        double convRate = 1;
        // We got this far, get the currency conversion rate for this account
        if (account) convRate = Model_CurrencyHistory::getDayRate(Model_Account::currency(account)->CURRENCYID, transaction.TRANSDATE);
        int year = Model_Checking::TRANSDATE(transaction).GetYear();

        int idx = year * 100 + Model_Checking::TRANSDATE(transaction).GetMonth();

        if (Model_Checking::type(transaction) == Model_Checking::DEPOSIT) {
            incomeExpensesStats[idx].first += transaction.TRANSAMOUNT * convRate;
        }
        else if (Model_Checking::type(transaction) == Model_Checking::WITHDRAWAL) {
            incomeExpensesStats[idx].second += transaction.TRANSAMOUNT * convRate;
        }
    }

    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    hb.addReportHeader(getReportTitle(), m_date_range->startDay(), m_date_range->isFutureIgnored());
    hb.DisplayDateHeading(m_date_range->start_date(), m_date_range->end_date(), m_date_range->is_with_date());
    hb.DisplayFooter(getAccountNames());

    // Chart
    const wxDateTime start_date = m_date_range->start_date();
    wxDateSpan s = m_date_range->end_date().GetLastMonthDay().DiffAsDateSpan(start_date);
    int m = s.GetYears() * 12 + s.GetMonths() + 1;
    m = m > 60 ? 60 : m;

    wxLogDebug("%s %s %i", start_date.FormatISODate(), m_date_range->end_date().FormatISODate(), m);

    if (getChartSelection() == 0)
    {
        GraphData gd;
        GraphSeries data_negative, data_positive, data_difference, data_performance;

        double performance = 0;
        for (const auto &stats : incomeExpensesStats)
        {
            data_positive.values.push_back(stats.second.first);
            data_negative.values.push_back(stats.second.second);
            data_difference.values.push_back(stats.second.first - stats.second.second);
            performance = performance + stats.second.first - stats.second.second;
            data_performance.values.push_back(performance);

            const auto label = wxString::Format("%s %i"
                , wxGetTranslation(wxDateTime::GetEnglishMonthName(static_cast<wxDateTime::Month>(stats.first % 100))), stats.first / 100);
            gd.labels.push_back(label);
        }

        data_performance.name = _("Cumulative");
        data_difference.name = _("Difference");
        data_positive.name = _("Income");
        data_negative.name = _("Expenses");

        data_performance.type = "line";
        data_difference.type = "line";
        data_positive.type = "column";
        data_negative.type = "column";

        gd.series.push_back(data_performance);
        gd.series.push_back(data_difference);
        gd.series.push_back(data_positive);
        gd.series.push_back(data_negative);


        if (!gd.series.empty())
        {
            gd.type = GraphData::BARLINE; 
            gd.colors = { mmThemeMetaColour(meta::COLOR_REPORT_PERF)
                            , mmThemeMetaColour(meta::COLOR_REPORT_DELTA)
                            , mmThemeMetaColour(meta::COLOR_REPORT_CREDIT)
                            , mmThemeMetaColour(meta::COLOR_REPORT_DEBIT) }; 
            hb.addChart(gd);
        }
    }

    hb.addDivContainer("shadow"); // Table Container
    hb.startSortTable();
    {
        hb.startThead();
        {
            hb.startTableRow();
            hb.addTableHeaderCell(_("Date"));
            hb.addTableHeaderCell(_("Income"), "text-right");
            hb.addTableHeaderCell(_("Expenses"), "text-right");
            hb.addTableHeaderCell(_("Difference"), "text-right");
            hb.addTableHeaderCell(_("Cumulative"), "text-right");
            hb.endTableRow();
        }
        hb.endThead();
        wxLogDebug("from %s till %s", m_date_range->start_date().FormatISODate(), m_date_range->end_date().FormatISODate());

        double total_expenses = 0.0;
        double total_income = 0.0;
        hb.startTbody();
        for (const auto &stats : incomeExpensesStats)
        {
            total_expenses += stats.second.second;
            total_income += stats.second.first;

            hb.startTableRow();
            {
                hb.addTableCellMonth(stats.first % 100, stats.first / 100);
                hb.addMoneyCell(stats.second.first);
                hb.addMoneyCell(stats.second.second);
                hb.addMoneyCell(stats.second.first - stats.second.second);
                hb.addMoneyCell(total_income - total_expenses);
            }
            hb.endTableRow();
        }
        hb.endTbody();

        std::vector<double> totals;
        totals.push_back(total_income);
        totals.push_back(total_expenses);
        totals.push_back(total_income - total_expenses);
        totals.push_back(total_income - total_expenses);

        hb.addMoneyTotalRow(_("Total:"), 5, totals);
    }
    hb.endTable();

    hb.endDiv(); // Table container
    hb.endDiv(); // Report Container
    hb.endDiv(); // Main container
    hb.end();

    wxLogDebug("======= mmReportIncomeExpensesMonthly::getHTMLText =======");
    wxLogDebug("%s", hb.getHTMLText());

    return hb.getHTMLText();
}