/*******************************************************
Copyright (C) 2006-2012
Copyright (C) 2021          Mark Whalley (mark@ipx.co.uk)
Copyright (C) 2026          George Ef (george.a.ef@gmail.com)

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

ReportFlowByPayee::Data::Data() :
    payee_name(""),
    flow_pos(0.0),
    flow_neg(0.0),
    flow(0.0)
{
}

ReportFlowByPayee::ReportFlowByPayee() :
    ReportBase(_n("Payee Report"))
{
    setReportParameters(REPORT_ID::Payees);
}

ReportFlowByPayee::~ReportFlowByPayee()
{
}

void ReportFlowByPayee::updateData(Data& data, Model_Checking::TYPE_ID type_id, double amount)
{
    double flow = (type_id == Model_Checking::TYPE_ID_DEPOSIT) ? amount : -amount;
    if (flow > 0.0)
        data.flow_pos += flow;
    else if (flow < 0.0)
        data.flow_neg += flow;
    data.flow += flow;
}

void ReportFlowByPayee::loadData(mmDateRange* date_range, bool WXUNUSED(ignoreFuture))
{
    // FIXME: do not ignore ignoreFuture param

    m_id_data.clear();

    const auto all_splits = Model_Splittransaction::instance().get_all();
    const auto &trx_a = Model_Checking::instance().find(
        Model_Checking::TRANSDATE(DateDay(date_range->start_date()), GREATER_OR_EQUAL),
        Model_Checking::TRANSDATE(DateDay(date_range->end_date()), LESS_OR_EQUAL),
        Model_Checking::DELETEDTIME(wxEmptyString, EQUAL),
        Model_Checking::STATUS(Model_Checking::STATUS_ID_VOID, NOT_EQUAL)
    );
    for (const auto& trx: trx_a) {
        // Do not include asset or stock transfers
        if (Model_Checking::foreignTransactionAsTransfer(trx))
            continue;

        Model_Checking::TYPE_ID type_id = Model_Checking::type_id(trx);
        // Transfer transactions do not have a payee
        if (type_id == Model_Checking::TYPE_ID_TRANSFER)
            continue;

        int64 payee_id = trx.PAYEEID;
        if (payee_id < 0)
            continue;
        auto [it, new_payee] = m_id_data.try_emplace(trx.PAYEEID, Data{});
        Data& data = it->second;
        if (new_payee) {
            Model_Payee::Data* payee = Model_Payee::instance().get(payee_id);
            data.payee_name = payee ? payee->PAYEENAME : "";
            data.flow_pos = 0.0;
            data.flow_neg = 0.0;
            data.flow = 0.0;
        }

        // NOTE: call to getDayRate() in every transaction is slow
        // if "Use historical currency" is enabled in settings
        const double convRate = Model_CurrencyHistory::getDayRate(
            Model_Account::instance().get(trx.ACCOUNTID)->CURRENCYID,
            trx.TRANSDATE
        );

        Model_Splittransaction::Data_Set splits;
        if (all_splits.count(trx.id()))
            splits = all_splits.at(trx.id());
        if (splits.empty()) {
            updateData(data, type_id, trx.TRANSAMOUNT * convRate);
        }
        else {
            for (const auto& split : splits) {
                updateData(data, type_id, split.SPLITTRANSAMOUNT * convRate);
            }
        }
    }
}

void  ReportFlowByPayee::refreshData()
{
    loadData(
        const_cast<mmDateRange*>(m_date_range),
        Option::instance().getIgnoreFutureTransactions()
    );

    m_order_net_flow.clear();
    m_total = Data();
    for (const auto& it : m_id_data) {
        m_order_net_flow.push_back(it.first);
        m_order_abs_flow.push_back(it.first);
        m_total.flow_pos += it.second.flow_pos;
        m_total.flow_neg += it.second.flow_neg;
        m_total.flow     += it.second.flow;
    }

    // order by net flow
    std::stable_sort(m_order_net_flow.begin(), m_order_net_flow.end(),
        [this](int64 x_id, int64 y_id) {
            double x_flow = m_id_data[x_id].flow;
            double y_flow = m_id_data[y_id].flow;
            return (x_flow < y_flow) ? true
                : (x_flow > y_flow) ? false
                : m_id_data[x_id].payee_name < m_id_data[y_id].payee_name;
        }
    );

    // order by abs flow
    m_order_abs_flow.clear();
    for (int64 payee_id : m_order_net_flow) {
        m_order_abs_flow.push_back(payee_id);
    }
    std::stable_sort(m_order_abs_flow.begin(), m_order_abs_flow.end(),
        [this](int64 x_id, int64 y_id) {
            double x_flow = m_id_data[x_id].flow;
            double y_flow = m_id_data[y_id].flow;
            return abs(x_flow) > abs(y_flow);
        }
    );
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
    if (!m_order_abs_flow.empty() && (getChartSelection() == 0)) {
        GraphData gd;
        GraphSeries data_usage;

        data_usage.name = _t("Payees");
        for (int64 payee_id : m_order_abs_flow) {
            gd.labels.push_back(m_id_data[payee_id].payee_name);
            data_usage.values.push_back(m_id_data[payee_id].flow);
        }
        gd.series.push_back(data_usage);
        gd.type = GraphData::PIE;
        hb.addChart(gd);
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
                hb.addTableHeaderCell(_t("Net flow"), "text-right");
                hb.endTableRow();
            }
            hb.endThead();

            hb.startTbody();
            {
                for (int64 payee_id : m_order_net_flow) {
                    const Data& data = m_id_data[payee_id];
                    hb.startTableRow();
                    {
                        hb.addTableCellLink(
                            wxString::Format("viewtrans:-1:-1:%lld", payee_id),
                            data.payee_name
                        );
                        hb.addMoneyCell(data.flow_pos);
                        hb.addMoneyCell(data.flow_neg);
                        hb.addMoneyCell(data.flow);
                    }
                    hb.endTableRow();
                }
            }
            hb.endTbody();
        
            hb.startTfoot();
            {
                std::vector<double> total;
                total.push_back(m_total.flow_pos);
                total.push_back(m_total.flow_neg);
                total.push_back(m_total.flow);
                hb.addMoneyTotalRow(_t("Total:"), 4, total);
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

