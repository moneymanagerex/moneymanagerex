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

#include "htmlbuilder.h"
#include "option.h"
#include "mmDateRange.h"
#include "model/Model_Currency.h"
#include "model/Model_CurrencyHistory.h"
#include "model/Model_Payee.h"
#include "model/Model_Account.h"

#include <algorithm>

ReportFlowByPayee::ReportFlowByPayee() :
    ReportBase(_n("Payee Report")),
    m_flow_pos(0.0),
    m_flow_neg(0.0)
{
    setReportParameters(REPORT_ID::Payees);
}

ReportFlowByPayee::~ReportFlowByPayee()
{
}

void  ReportFlowByPayee::refreshData()
{
    m_payee_data_a.clear();
    m_name_flow_a.clear();
    m_flow_pos = 0.0;
    m_flow_neg = 0.0;

    std::map<int64, std::pair<double, double>> payee_flow_a;
    loadPayeeFlow(
        payee_flow_a,
        const_cast<mmDateRange*>(m_date_range),
        Option::instance().getIgnoreFutureTransactions()
    );

    for (const auto& payee_flow : payee_flow_a) {
        Model_Payee::Data* payee = Model_Payee::instance().get(payee_flow.first);
        PayeeData payee_data;
        payee_data.name = payee ? payee->PAYEENAME : "";
        payee_data.id = payee ? payee->PAYEEID : -1;
        payee_data.incomes = payee_flow.second.first;
        payee_data.expenses = payee_flow.second.second;
        m_payee_data_a.push_back(payee_data);

        m_flow_pos += payee_flow.second.first;
        m_flow_neg += payee_flow.second.second;
    }

    // order by net flow (incomes + expenses)
    std::stable_sort(m_payee_data_a.begin(), m_payee_data_a.end(),
        [](const PayeeData& x, const PayeeData& y) {
            if (x.expenses + x.incomes != y.expenses + y.incomes)
                return x.expenses + x.incomes < y.expenses + y.incomes;
            else
                return x.name < y.name;
        }
    );

    for (const auto& payee_data : m_payee_data_a) {
        ValuePair name_flow;
        name_flow.label = payee_data.name;
        name_flow.amount = payee_data.incomes + payee_data.expenses;
        m_name_flow_a.push_back(name_flow);
    }
}

wxString ReportFlowByPayee::getHTMLText()
{
    // Grab the data
    refreshData();

    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    hb.addReportHeader(getTitle(), m_date_range->startDay(), m_date_range->isFutureIgnored());
    hb.DisplayDateHeading(m_date_range);
    // Prime the filter
    m_filter.clear();
    m_filter.setDateRange(m_date_range->start_date(), m_date_range->end_date());

    // Add the chart
    if (!m_name_flow_a.empty() && (getChartSelection() == 0)) {
        GraphData gd;
        GraphSeries data_usage;
        std::stable_sort(m_name_flow_a.begin(), m_name_flow_a.end(),
            [](const ValuePair& left, const ValuePair& right) {
                return abs(left.amount) > abs(right.amount);
            }
        );
        for (const auto &name_flow : m_name_flow_a) {
            data_usage.values.push_back(name_flow.amount);
            gd.labels.push_back(name_flow.label);
        }

        data_usage.name = _t("Payees");
        gd.series.push_back(data_usage);
        
        if (!gd.series.empty()) {
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
                for (const auto& payee_data : m_payee_data_a) {
                    hb.startTableRow();
                    {
                        hb.addTableCellLink(
                            wxString::Format("viewtrans:-1:-1:%lld", payee_data.id),
                            payee_data.name
                        );
                        hb.addMoneyCell(payee_data.incomes);
                        hb.addMoneyCell(payee_data.expenses);
                        hb.addMoneyCell(payee_data.incomes + payee_data.expenses);
                    }
                    hb.endTableRow();
                }
            }
            hb.endTbody();
        
            hb.startTfoot();
            {
                std::vector <double> totals;
                totals.push_back(m_flow_pos);
                totals.push_back(m_flow_neg);
                totals.push_back(m_flow_pos + m_flow_neg);
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

void ReportFlowByPayee::loadPayeeFlow(
    std::map<int64, std::pair<double, double>> &payee_flow_a,
    mmDateRange* date_range,
    bool WXUNUSED(ignoreFuture)
) const {
    // FIXME: do not ignore ignoreFuture param
    const auto all_splits = Model_Splittransaction::instance().get_all();
    const auto &transactions = Model_Checking::instance().find(
        Model_Checking::TRANSDATE(DateDay(date_range->start_date()), GREATER_OR_EQUAL),
        Model_Checking::TRANSDATE(DateDay(date_range->end_date()), LESS_OR_EQUAL),
        Model_Checking::DELETEDTIME(wxEmptyString, EQUAL),
        Model_Checking::STATUS(Model_Checking::STATUS_ID_VOID, NOT_EQUAL)
    );
    for (const auto& trx: transactions) {
        // Transfer transactions do not have a payee
        if (Model_Checking::type_id(trx) == Model_Checking::TYPE_ID_TRANSFER)
            continue;

        // Do not include asset or stock transfers
        if (Model_Checking::foreignTransactionAsTransfer(trx))
            continue;

        const double convRate = Model_CurrencyHistory::getDayRate(
            Model_Account::instance().get(trx.ACCOUNTID)->CURRENCYID,
            trx.TRANSDATE
        );

        Model_Splittransaction::Data_Set splits;
        if (all_splits.count(trx.id()))
            splits = all_splits.at(trx.id());
        if (splits.empty()) {
            if (Model_Checking::type_id(trx) == Model_Checking::TYPE_ID_DEPOSIT)
                payee_flow_a[trx.PAYEEID].first += trx.TRANSAMOUNT * convRate;
            else if (Model_Checking::type_id(trx) == Model_Checking::TYPE_ID_WITHDRAWAL)
                payee_flow_a[trx.PAYEEID].second -= trx.TRANSAMOUNT * convRate;
        }
        else {
            for (const auto& entry : splits) {
                if (Model_Checking::type_id(trx) == Model_Checking::TYPE_ID_DEPOSIT) {
                    if (entry.SPLITTRANSAMOUNT >= 0)
                        payee_flow_a[trx.PAYEEID].first += entry.SPLITTRANSAMOUNT * convRate;
                    else
                        payee_flow_a[trx.PAYEEID].second += entry.SPLITTRANSAMOUNT * convRate;
                }
                else if (Model_Checking::type_id(trx) == Model_Checking::TYPE_ID_WITHDRAWAL) {
                    if (entry.SPLITTRANSAMOUNT < 0)
                        payee_flow_a[trx.PAYEEID].first -= entry.SPLITTRANSAMOUNT * convRate;
                    else
                        payee_flow_a[trx.PAYEEID].second -= entry.SPLITTRANSAMOUNT * convRate;
                }
            }
        }
    }
}
