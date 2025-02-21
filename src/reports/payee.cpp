/*******************************************************
Copyright (C) 2006-2012
Copyright (C) 2021          Mark Whalley (mark@ipx.co.uk)

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

#include "reports/htmlbuilder.h"
#include "option.h"
#include "reports/mmDateRange.h"
#include "model/Model_Currency.h"
#include "model/Model_CurrencyHistory.h"
#include "model/Model_Payee.h"
#include "model/Model_Account.h"

#include <algorithm>

mmReportPayeeExpenses::mmReportPayeeExpenses()
    : mmPrintableBase(_n("Payee Report"))
    , positiveTotal_(0.0)
    , negativeTotal_(0.0)
{
    setReportParameters(Reports::Payees);
}

mmReportPayeeExpenses::~mmReportPayeeExpenses()
{
}

void  mmReportPayeeExpenses::RefreshData()
{
    data_.clear();
    valueList_.clear();
    positiveTotal_ = 0.0;
    negativeTotal_ = 0.0;

    std::map<int64, std::pair<double, double> > payeeStats;
    getPayeeStats(payeeStats, const_cast<mmDateRange*>(m_date_range)
        , Option::instance().getIgnoreFutureTransactions());

    data_holder line;

    for (const auto& entry : payeeStats)
    {
        positiveTotal_ += entry.second.first;
        negativeTotal_ += entry.second.second;

        Model_Payee::Data* payee = Model_Payee::instance().get(entry.first);

        line.name = payee ? payee->PAYEENAME : "";
        line.payee = payee ? payee->PAYEEID : -1;
        line.incomes = entry.second.first;
        line.expenses = entry.second.second;
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
        ValuePair vt;
        vt.label = entry.name;
        vt.amount = entry.incomes + entry.expenses;
        valueList_.push_back(vt);
    }

}

wxString mmReportPayeeExpenses::getHTMLText()
{
    // Grab the data
    RefreshData();

    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    hb.addReportHeader(getReportTitle(), m_date_range->startDay(), m_date_range->isFutureIgnored());
    hb.DisplayDateHeading(m_date_range->start_date(), m_date_range->end_date(), m_date_range->is_with_date());
    // Prime the filter
    m_filter.clear();
    m_filter.setDateRange(m_date_range->start_date(), m_date_range->end_date());

    // Add the chart
    if (!valueList_.empty() && (getChartSelection() == 0))
    {
        GraphData gd;
        GraphSeries data_usage;
        std::stable_sort(valueList_.begin(), valueList_.end(), [](const ValuePair& left, const ValuePair& right) {
            return abs(left.amount) > abs(right.amount); });
        for (const auto &stats : valueList_)
        {
            data_usage.values.push_back(stats.amount);
            gd.labels.push_back(stats.label);
        }

        data_usage.name = _t("Payees");
        gd.series.push_back(data_usage);
        
        if (!gd.series.empty())
        {
            gd.type = GraphData::PIE;
            hb.addChart(gd);
        }
    }

    hb.addDivContainer("shadow"); 
    {    
        hb.startSortTable();
        {
            hb.startThead();
            {
                hb.startTableRow();
                hb.addTableHeaderCell(_t("Payee"));
                hb.addTableHeaderCell(_t("Incomes"), "text-right");
                hb.addTableHeaderCell(_t("Expenses"), "text-right");
                hb.addTableHeaderCell(_t("Difference"), "text-right");
                hb.endTableRow();
            }
            hb.endThead();

            hb.startTbody();
            {
                for (const auto& entry : data_)
                {
                    hb.startTableRow();
                    {
                        hb.addTableCellLink(wxString::Format("viewtrans:-1:-1:%lld", entry.payee)
                            , entry.name);
                        hb.addMoneyCell(entry.incomes);
                        hb.addMoneyCell(entry.expenses);
                        hb.addMoneyCell(entry.incomes + entry.expenses);
                    }
                    hb.endTableRow();
                }
            }
            hb.endTbody();
        
            hb.startTfoot();
            {
                std::vector <double> totals;
                totals.push_back(positiveTotal_);
                totals.push_back(negativeTotal_);
                totals.push_back(positiveTotal_ + negativeTotal_);
                hb.addMoneyTotalRow(_t("Total:"), 4, totals);
            }
            hb.endTfoot();
        }
        hb.endTable();
    }
    hb.endDiv();

    hb.end();

    wxLogDebug("======= mmReportPayess:getHTMLText =======");
    wxLogDebug("%s", hb.getHTMLText());

    return hb.getHTMLText();
}

void mmReportPayeeExpenses::getPayeeStats(std::map<int64, std::pair<double, double> > &payeeStats
                                          , mmDateRange* date_range, bool WXUNUSED(ignoreFuture)) const
{
// FIXME: do not ignore ignoreFuture param
    const auto &transactions = Model_Checking::instance().find(
        Model_Checking::STATUS(Model_Checking::STATUS_ID_VOID, NOT_EQUAL)
        , Model_Checking::TRANSDATE(date_range->start_date(), GREATER_OR_EQUAL)
        , Model_Checking::TRANSDATE(date_range->end_date().FormatISOCombined(), LESS_OR_EQUAL));
    const auto all_splits = Model_Splittransaction::instance().get_all();
    for (const auto& trx: transactions)
    {
        if (Model_Checking::type_id(trx) == Model_Checking::TYPE_ID_TRANSFER || !trx.DELETEDTIME.IsEmpty()) continue;

        // Do not include asset or stock transfers in income expense calculations.
        if (Model_Checking::foreignTransactionAsTransfer(trx))
            continue;

        const double convRate = Model_CurrencyHistory::getDayRate(Model_Account::instance().get(trx.ACCOUNTID)->CURRENCYID, trx.TRANSDATE);

        Model_Splittransaction::Data_Set splits;
        if (all_splits.count(trx.id())) splits = all_splits.at(trx.id());
        if (splits.empty())
        {
            if (Model_Checking::type_id(trx) == Model_Checking::TYPE_ID_DEPOSIT)
                payeeStats[trx.PAYEEID].first += trx.TRANSAMOUNT * convRate;
            else
                payeeStats[trx.PAYEEID].second -= trx.TRANSAMOUNT * convRate;
        }
        else
        {
            for (const auto& entry : splits)
            {
                if (Model_Checking::type_id(trx) == Model_Checking::TYPE_ID_DEPOSIT)
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
