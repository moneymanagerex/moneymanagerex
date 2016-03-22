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

#include "payee.h"

#include "htmlbuilder.h"
#include "model/Model_Currency.h"
#include "model/Model_Payee.h"
#include "model/Model_Account.h"

#include <algorithm>

mmReportPayeeExpenses::mmReportPayeeExpenses()
    : mmPrintableBase(_("Payee Report"))
    , positiveTotal_(0.0)
    , negativeTotal_(0.0)
{
}

mmReportPayeeExpenses::~mmReportPayeeExpenses()
{
}

bool mmReportPayeeExpenses::has_date_range()
{
    return true;
}

void  mmReportPayeeExpenses::RefreshData()
{
    data_.clear();
    valueList_.clear();
    positiveTotal_ = 0.0;
    negativeTotal_ = 0.0;

    std::map<int, std::pair<double, double> > payeeStats;
    getPayeeStats(payeeStats, const_cast<mmDateRange*>(m_date_range)
        , mmIniOptions::instance().ignoreFutureTransactions_);

    data_holder line;

    mmHTMLBuilder hb;
    for (const auto& entry : payeeStats)
    {
        positiveTotal_ += entry.second.first;
        negativeTotal_ += entry.second.second;

        Model_Payee::Data* payee = Model_Payee::instance().get(entry.first);

        line.name = payee ? payee->PAYEENAME : "";
        line.incomes = entry.second.first;
        line.expenses = entry.second.second;
        line.color = hb.getRandomColor((line.incomes + line.expenses) > 0);
        data_.push_back(line);
    }

    //Order by expenses + deposits diff
    std::stable_sort(data_.begin(), data_.end()
        , [](const data_holder& x, const data_holder& y)
        {
            if (x.expenses + x.incomes != y.expenses + y.incomes)
                return x.expenses + x.incomes < y.expenses + y.incomes;
            else
                return x.name < y.name;
        }
    );
    

    for (const auto& entry : data_) {
        ValueTrio vt;
        vt.label = entry.name;
        vt.amount = fabs(entry.incomes + entry.expenses);
        vt.color = entry.color;
        valueList_.push_back(vt);
    }

}

wxString mmReportPayeeExpenses::getHTMLText()
{
    RefreshData();
    mmHTMLBuilder hb;
    hb.init();
    hb.addDivContainer();
    hb.addHeader(2, title());
    hb.DisplayDateHeading(m_date_range->start_date(), m_date_range->end_date(), m_date_range->is_with_date());

    hb.addDivRow();
    hb.addDivCol17_67();
    // Add the graph
    hb.addDivCol25_50();
    if (!valueList_.empty())
        hb.addPieChart(valueList_, "Withdrawal");
    hb.endDiv();

    hb.startSortTable();
    hb.startThead();
    hb.startTableRow();
        hb.addTableHeaderCell(" ", false, false);
        hb.addTableHeaderCell(_("Payee"));
        hb.addTableHeaderCell(_("Incomes"), true);
        hb.addTableHeaderCell(_("Expenses"), true);
        hb.addTableHeaderCell(_("Difference"), true);
    hb.endTableRow();
    hb.endThead();

    hb.startTbody();
    for (const auto& entry : data_)
    {
        hb.startTableRow();
        hb.addColorMarker(entry.color);
        hb.addTableCell(entry.name);
        hb.addMoneyCell(entry.incomes);
        hb.addMoneyCell(entry.expenses);
        hb.addMoneyCell(entry.incomes + entry.expenses);
        hb.endTableRow();
    }
    hb.endTbody();

    hb.startTfoot();
    std::vector <double> totals;
    totals.push_back(positiveTotal_);
    totals.push_back(negativeTotal_);
    totals.push_back(positiveTotal_ + negativeTotal_);
    hb.addTotalRow(_("Total:"), 5, totals);
    hb.endTfoot();

    hb.endTable();
    hb.endDiv();
    hb.endDiv();
    hb.endDiv();
    hb.end();

    Model_Report::outputReportFile(hb.getHTMLText());
    return "";
}

void mmReportPayeeExpenses::getPayeeStats(std::map<int, std::pair<double, double> > &payeeStats
                                          , mmDateRange* date_range, bool ignoreFuture) const
{
    //Get base currency rates for all accounts
    std::map<int, double> acc_conv_rates;
    for (const auto& account: Model_Account::instance().all())
    {
        Model_Currency::Data* currency = Model_Account::currency(account);
        acc_conv_rates[account.ACCOUNTID] = currency->BASECONVRATE;
    }

    const auto &transactions = Model_Checking::instance().find(
        Model_Checking::STATUS(Model_Checking::VOID_, NOT_EQUAL)
        , Model_Checking::TRANSDATE(m_date_range->start_date(), GREATER_OR_EQUAL)
        , Model_Checking::TRANSDATE(m_date_range->end_date(), LESS_OR_EQUAL));
    const wxDateTime today = m_date_range->today();
    const auto all_splits = Model_Splittransaction::instance().get_all();
    for (const auto& trx: transactions)
    {
        if (Model_Checking::type(trx) == Model_Checking::TRANSFER) continue;

        double convRate = acc_conv_rates[trx.ACCOUNTID];

        Model_Splittransaction::Data_Set splits;
        if (all_splits.count(trx.id())) splits = all_splits.at(trx.id());
        if (splits.empty())
        {
            if (Model_Checking::type(trx) == Model_Checking::DEPOSIT)
                payeeStats[trx.PAYEEID].first += trx.TRANSAMOUNT * convRate;
            else
                payeeStats[trx.PAYEEID].second -= trx.TRANSAMOUNT * convRate;
        }
        else
        {
            for (const auto& entry : splits)
            {
                if (Model_Checking::type(trx) == Model_Checking::DEPOSIT)
                {
                    if (entry.SPLITTRANSAMOUNT >= 0)
                        payeeStats[trx.PAYEEID].first += entry.SPLITTRANSAMOUNT * convRate;
                    else
                        payeeStats[trx.PAYEEID].second += entry.SPLITTRANSAMOUNT * convRate;
                }
                else
                {
                    if (entry.SPLITTRANSAMOUNT < 0)
                        payeeStats[trx.PAYEEID].first -= entry.SPLITTRANSAMOUNT * convRate;
                    else
                        payeeStats[trx.PAYEEID].second -= entry.SPLITTRANSAMOUNT * convRate;
                }
            }
        }
    }
}
