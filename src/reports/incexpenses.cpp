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

#include "Model_Account.h"
#include "Model_Checking.h"
#include "Model_CurrencyHistory.h"


mmReportIncomeExpenses::mmReportIncomeExpenses()
    : mmPrintableBase(_("Income vs Expenses"))
{
}

mmReportIncomeExpenses::~mmReportIncomeExpenses()
{
}

int mmReportIncomeExpenses::report_parameters()
{
    return RepParams::DATE_RANGE | RepParams::ACCOUNTS_LIST;
}

wxString mmReportIncomeExpenses::getHTMLText()
{
    wxString headerMsg = _("Accounts: ");
    if (accountArray_ == nullptr)
    {
        headerMsg << _("All Accounts");
    }
    else
    {
        int arrIdx = 0;
        if ((int)accountArray_->size() == 0)
            headerMsg << "?";

        if (!accountArray_->empty())
        {
            headerMsg << accountArray_->Item(arrIdx);
            arrIdx++;
        }
        while (arrIdx < (int)accountArray_->size())
        {
            headerMsg << ", " << accountArray_->Item(arrIdx);
            arrIdx++;
        }
    }
    mmHTMLBuilder hb;
    hb.init();
    hb.addDivContainer();
    hb.addHeader(2, this->title());
    hb.addDateNow();
    hb.DisplayDateHeading(m_date_range->start_date(), m_date_range->end_date(), m_date_range->is_with_date());
    hb.addHeader(3, headerMsg);

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

    ValueTrio vt;
    std::vector<ValueTrio> valueList;
    vt.amount = income_expenses_pair.first;
    vt.color = "rgba(151,187,205,0.5)";
    vt.label = _("Income");
    valueList.push_back(vt);
    vt.amount = income_expenses_pair.second;
    vt.color = "rgba(220,66,66,0.5)";
    vt.label = _("Expenses");
    valueList.push_back(vt);

    hb.addDivRow();
    hb.addDivCol17_67();
    hb.startTable();
    {
        hb.startTableRow();
        {
            hb.startTableCell(" style='vertical-align:middle' width='70%'");
            hb.addDivCol17_67();
            if (!valueList.empty())
                hb.addBarChart("''", valueList, "BarChart");
            hb.endDiv();
            hb.endTableCell();

            hb.startTableCell(" style='vertical-align:middle' width='30%'");
            hb.addDivCol17_67();
            hb.startTable();
            {
                hb.startThead();
                hb.startTableRow();
                hb.addTableHeaderCell(_("Type"));
                hb.addTableHeaderCell(_("Amount"), true);
                hb.endTableRow();
                hb.endThead();

                hb.addTableRow(_("Income:"), income_expenses_pair.first);
                hb.addTableRow(_("Expenses:"), income_expenses_pair.second);

                hb.addTotalRow(_("Difference:"), 2, income_expenses_pair.first - income_expenses_pair.second);
            }
            hb.endTable();
            hb.addDivCol6();
            hb.endDiv();
            hb.endTableCell();
        }
        hb.endTableRow();
    }
    hb.endTable();

    hb.endDiv();
    hb.endDiv();
    hb.endDiv();
    hb.end();

    return hb.getHTMLText();
}

mmReportIncomeExpensesMonthly::mmReportIncomeExpensesMonthly()
    : mmPrintableBase(_("Income vs Expenses"))
{
}

mmReportIncomeExpensesMonthly::~mmReportIncomeExpensesMonthly()
{
}

int mmReportIncomeExpensesMonthly::report_parameters()
{
    return RepParams::DATE_RANGE | RepParams::ACCOUNTS_LIST;
}

wxString mmReportIncomeExpensesMonthly::getHTMLText()
{
    wxString headerMsg = _("Accounts: ");
    if (accountArray_ == nullptr)
    {
        headerMsg << _("All Accounts");
    }
    else
    {
        if ((int)accountArray_->size() == 0)
            headerMsg << "?";

        for (const auto& entry : *accountArray_)
        { 
            headerMsg << entry << ", ";
        }
        if (headerMsg.Mid(headerMsg.size() - 2, 2) == ", ") headerMsg.RemoveLast(2);
    }

    std::map<int, std::pair<double, double> > incomeExpensesStats;
    //TODO: init all the map values with 0.0
    for (const auto& transaction : Model_Checking::instance().find(
        Model_Checking::TRANSDATE(m_date_range->start_date(), GREATER_OR_EQUAL)
        , Model_Checking::TRANSDATE(m_date_range->end_date(), LESS_OR_EQUAL)
        , Model_Checking::STATUS(Model_Checking::VOID_, NOT_EQUAL)))
    {
        Model_Account::Data *account = Model_Account::instance().get(transaction.ACCOUNTID);
        if (accountArray_)
        {
            if (!account || wxNOT_FOUND == accountArray_->Index(account->ACCOUNTNAME))
                continue;
        }
        double convRate = 1;
        // We got this far, get the currency conversion rate for this account
        if (account) convRate = Model_CurrencyHistory::getDayRate(Model_Account::currency(account)->CURRENCYID, transaction.TRANSDATE);

        int idx = (Model_Checking::TRANSDATE(transaction).GetYear() * 100
            + (int) Model_Checking::TRANSDATE(transaction).GetMonth());

        if (Model_Checking::type(transaction) == Model_Checking::DEPOSIT)
            incomeExpensesStats[idx].first += transaction.TRANSAMOUNT * convRate;
        else if (Model_Checking::type(transaction) == Model_Checking::WITHDRAWAL)
            incomeExpensesStats[idx].second += transaction.TRANSAMOUNT * convRate;
    }

    mmHTMLBuilder hb;
    hb.init();
    hb.addDivContainer();
    hb.addHeader(2, this->title());
    hb.addDateNow();
    hb.DisplayDateHeading(m_date_range->start_date(), m_date_range->end_date(), m_date_range->is_with_date());
    hb.addHeader(3, headerMsg);
    hb.addLineBreak();
    hb.addDivRow();
    hb.addDivCol17_67();

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
            hb.addTableCell(wxString() << (int) (stats.first / 100));
            hb.addTableCellMonth(stats.first % 100);
            hb.addMoneyCell(stats.second.first);
            hb.addMoneyCell(stats.second.second);
            hb.addMoneyCell(stats.second.first - stats.second.second);
            hb.endTableRow();
        }
        hb.endTbody();

        std::vector<double> data;
        data.push_back(total_income);
        data.push_back(total_expenses);
        data.push_back(total_income - total_expenses);

        hb.addTotalRow(_("Total:"), 5, data);

    }
    hb.endTable();
    hb.endDiv();
    hb.endDiv();
    hb.endDiv(); 
    hb.end();

    return hb.getHTMLText();
}
