/*******************************************************
Copyright (C) 2006-2012 Madhan Kanagavel
Copyright (C) 2017 James Higley

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
    hb.addDivContainer(); // Main container
    hb.addHeader(2, this->getReportTitle());
    hb.DisplayDateHeading(m_date_range->start_date(), m_date_range->end_date(), m_date_range->is_with_date());
    hb.addHeader(3, getAccountNames());
    hb.addDateNow();
    hb.addLineBreak();
    hb.addDivRow(); // Report Container
    
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
        hb.addDivContainer();
        {
            gd.type = GraphData::BAR;
            gd.colors = { wxColour(0, 227, 150), wxColour(255, 69, 96) };  // Green, Red
            hb.addChart(gd);
        }
        hb.endDiv();
    }

    hb.addDivContainer(); // Table Container
    hb.startTable();
    {
        hb.startThead();
        {
            hb.startTableRow();
            hb.addTableHeaderCell(_("Type"));
            hb.addTableHeaderCell(_("Amount"), true);
            hb.endTableRow();
            hb.endThead();
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
    
    hb.endDiv();// Table container
    hb.endDiv(); // Report Container
    hb.endDiv(); // Main container
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

        int idx = (year * 100 + Model_Checking::TRANSDATE(transaction).GetMonth());

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
    hb.addDivContainer(); // Main container
    hb.addHeader(2, this->getReportTitle());
    hb.DisplayDateHeading(m_date_range->start_date(), m_date_range->end_date(), m_date_range->is_with_date());
    hb.addHeader(3, getAccountNames());
    hb.addDateNow();
    hb.addLineBreak();
    hb.addDivRow(); // Report Container

    // Chart
    const wxDateTime start_date = m_date_range->start_date();
    wxDateSpan s = m_date_range->end_date().GetLastMonthDay().DiffAsDateSpan(start_date);
    int m = s.GetYears() * 12 + s.GetMonths() + 1;
    m = m > 60 ? 60 : m;

    wxLogDebug("%s %s %i", start_date.FormatISODate(), m_date_range->end_date().FormatISODate(), m);

    if (getChartSelection() == 0)
    {
        GraphData gd;
        GraphSeries data_negative, data_positive;

        for (const auto &stats : incomeExpensesStats)
        {
            data_positive.values.push_back(stats.second.first);
            data_negative.values.push_back(stats.second.second);

            wxString label = wxString::Format("%02i/%i", (stats.first % 100)+1, stats.first / 100);
            gd.labels.push_back(label);
        }

        data_positive.name = _("Income");
        data_negative.name = _("Expenses");
        gd.series.push_back(data_positive);
        gd.series.push_back(data_negative);

        if (!gd.series.empty())
        {
            hb.addDivContainer();
            {
                gd.type = GraphData::BAR; 
                gd.colors = { wxColour(0, 227, 150), wxColour(255, 69, 96) };  // Green, Red
                hb.addChart(gd);
            }
            hb.endDiv();
        }
    }

    hb.addDivContainer(); // Table Container
    hb.startSortTable();
    {
        hb.startThead();
        {
            hb.startTableRow();
            hb.addTableHeaderCell(_("Year"));
            hb.addTableHeaderCell(_("Month"));
            hb.addTableHeaderCell(_("Income"), true);
            hb.addTableHeaderCell(_("Expenses"), true);
            hb.addTableHeaderCell(_("Difference"), true);
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
            hb.addTableCell(wxString() << stats.first / 100);
            hb.addTableCellMonth(static_cast<wxDateTime::Month>(stats.first % 100));
            hb.addMoneyCell(stats.second.first);
            hb.addMoneyCell(stats.second.second);
            hb.addMoneyCell(stats.second.first - stats.second.second);
            hb.endTableRow();
        }
        hb.endTbody();

        std::vector<double> totals;
        totals.push_back(total_income);
        totals.push_back(total_expenses);
        totals.push_back(total_income - total_expenses);

        hb.addTotalRow(_("Total:"), 5, totals);
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