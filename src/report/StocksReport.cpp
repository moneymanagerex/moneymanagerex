/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2021,2024 Mark Whalley (mark@ipx.co.uk)

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

#include "base/defs.h"
#include <algorithm>
#include "base/constants.h"
#include "util/_util.h"
#include "util/mmDateRange.h"
#include "htmlbuilder.h"

#include "model/AccountModel.h"
#include "model/CurrencyModel.h"
#include "model/CurrencyHistoryModel.h"
#include "model/StockHistoryModel.h"

#include "panel/StockPanel.h"
#include "StocksReport.h"
#include "budget.h"
#include "uicontrols/navigatortypes.h"

StocksReport::StocksReport()
    : ReportBase(_n("Summary of Stocks"))
{
    setReportParameters(REPORT_ID::StocksReportSummary);
}

void  StocksReport::refreshData()
{
    m_stocks.clear();
    m_real_gain_loss_sum_total = 0.0;
    m_unreal_gain_loss_sum_total = 0.0;
    m_real_gain_loss_excl_forex = 0.0;
    m_unreal_gain_loss_excl_forex = 0.0;
    m_stock_balance = 0.0;

    DataHolder line;
    AccountHolder account_holder;

    for (const auto& account_d : AccountModel::instance().find_all(
        AccountCol::COL_ID_ACCOUNTNAME
    )) {
        if (AccountModel::type_id(account_d) != NavigatorTypes::TYPE_ID_INVESTMENT)
            continue;
        if (!account_d.is_open())
            continue;

        account_holder.id = account_d.m_id;
        account_holder.name = account_d.m_name;
        account_holder.realgainloss = 0.0;
        account_holder.unrealgainloss = 0.0;
        account_holder.total = AccountModel::instance().get_data_investment_balance(account_d).first;
        account_holder.data.clear();

        for (const auto& stock_d : StockModel::instance().find(
            StockCol::HELDAT(account_d.m_id)
        )) {
            const CurrencyData* currency_n = AccountModel::instance().get_data_currency_p(account_d);
            const double today_rate = CurrencyHistoryModel::instance().get_id_date_rate(
                currency_n->m_id
            );
            m_stock_balance += today_rate * stock_d.current_value();
            line.realgainloss = StockModel::instance().calculate_realized_gain(stock_d);
            account_holder.realgainloss += line.realgainloss;
            line.unrealgainloss = StockModel::instance().calculate_unrealiazed_gain(stock_d);
            account_holder.unrealgainloss += line.unrealgainloss;
            m_unreal_gain_loss_sum_total += StockModel::instance().calculate_unrealiazed_gain(stock_d, true);
            m_real_gain_loss_sum_total += StockModel::instance().calculate_realized_gain(stock_d, true);
            m_real_gain_loss_excl_forex += line.realgainloss * today_rate;
            m_unreal_gain_loss_excl_forex += line.unrealgainloss * today_rate;

            line.name       = stock_d.m_name;
            line.symbol     = stock_d.m_symbol;
            line.date       = stock_d.m_purchase_date.isoDate();
            line.qty        = stock_d.m_num_shares;
            line.purchase   = stock_d.m_purchase_value;
            line.current    = stock_d.m_current_price;
            line.commission = stock_d.m_commission;
            line.value      = stock_d.current_value();
            account_holder.data.push_back(line);
        }
        m_stocks.push_back(account_holder);
    }
}

wxString StocksReport::getHTMLText()
{
    // Grab the data
    refreshData();

    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    hb.addReportHeader(getTitle());

    hb.addDivContainer("shadow");
    {
        hb.startTable();
        {
            hb.startThead();
            {
                hb.startTableRow();
                {
                    hb.addTableHeaderCell(_t("Name"));
                    hb.addTableHeaderCell(_t("Symbol"));
                    hb.addTableHeaderCell(_t("Purchase Date"));
                    hb.addTableHeaderCell(_t("Quantity"), "text-right");
                    hb.addTableHeaderCell(_t("Total Cost"), "text-right");
                    hb.addTableHeaderCell(_t("Current Price"), "text-right");
                    hb.addTableHeaderCell(_t("Commission"), "text-right");
                    hb.addTableHeaderCell(_t("Realized Gain/Loss"), "text-right");
                    hb.addTableHeaderCell(_t("Unrealized Gain/Loss"), "text-right");
                    hb.addTableHeaderCell(_t("Current Value"), "text-right");
                }
                hb.endTableRow();
            }
            hb.endThead();

            for (const auto& acct : m_stocks) {
                const AccountData* account_n = AccountModel::instance().get_id_data_n(acct.id);
                const CurrencyData* currency_p = AccountModel::instance().get_data_currency_p(*account_n);

                hb.startThead();
                {
                    hb.startTableRow();
                    {
                        hb.addTableHeaderCell(acct.name, "text-left", 10);
                    }
                    hb.endTableRow();
                }
                hb.endThead();

                hb.startTbody();
                {
                    for (const auto& entry : acct.data)
                    {
                        hb.startTableRow();
                        {
                            hb.addTableCell(entry.name);
                            hb.addTableCell(entry.symbol);
                            hb.addTableCellDate(entry.date);
                            hb.addTableCell(AccountModel::instance().value_number(
                                *account_n, entry.qty,
                                trunc(entry.qty) == entry.qty ? 0 : 4
                            ), "text-right");
                            hb.addCurrencyCell(entry.purchase, currency_p, 4);
                            hb.addCurrencyCell(entry.current, currency_p, 4);
                            hb.addCurrencyCell(entry.commission, currency_p, 4);
                            hb.addCurrencyCell(entry.realgainloss, currency_p);
                            hb.addCurrencyCell(entry.unrealgainloss, currency_p);
                            hb.addCurrencyCell(entry.value, currency_p);
                        }
                        hb.endTableRow();
                    }
                    hb.startTotalTableRow();
                    {
                        hb.addTableCell(_t("Total:"));
                        hb.addEmptyTableCell(6);
                        hb.addCurrencyCell(acct.realgainloss, currency_p);
                        hb.addCurrencyCell(acct.unrealgainloss, currency_p);
                        hb.addCurrencyCell(acct.total, currency_p);
                    }
                    hb.endTableRow();
                    hb.addEmptyTableRow(9);
                }
                hb.endTbody();
            }

            hb.startTfoot();
            {
                // Round FX gain/loss to the scale of the base currency for display
                int scale = pow(10, log10(CurrencyModel::instance().instance().get_base_data_n()->m_scale.GetValue()));
                double forex_real_gain_loss = std::round((m_real_gain_loss_sum_total - m_real_gain_loss_excl_forex) * scale) / scale;
                double forex_unreal_gain_loss = std::round((m_unreal_gain_loss_sum_total - m_unreal_gain_loss_excl_forex) * scale) / scale;

                hb.startTotalTableRow();
                hb.addTableCell(_t("Grand Total:"));
                hb.addEmptyTableCell(6);

                hb.startTableCell(" style='text-align:right;' nowrap");
                if (forex_real_gain_loss != 0) {
                    hb.startSpan(CurrencyModel::instance().toCurrency(m_real_gain_loss_excl_forex), wxString::Format(" style='text-align:right;%s' nowrap"
                        , m_real_gain_loss_excl_forex < 0 ? "color:red;" : ""));
                    hb.endSpan();
                    hb.startSpan(" + ", "");
                    hb.endSpan();
                    hb.startSpan(CurrencyModel::instance().toCurrency(forex_real_gain_loss), wxString::Format(" style='text-align:right;%s' nowrap"
                        , forex_real_gain_loss < 0 ? "color:red;" : ""));
                    hb.endSpan();
                    hb.startSpan(" FX", "");
                    hb.endSpan();
                    hb.addLineBreak();
                }
                hb.startSpan(CurrencyModel::instance().toCurrency(m_real_gain_loss_sum_total), wxString::Format(" style='text-align:right;%s' nowrap"
                    , m_real_gain_loss_sum_total < 0 ? "color:red;" : ""));
                hb.endSpan();

                hb.endTableCell();

                hb.startTableCell(" style='text-align:right;' nowrap");
                if (forex_unreal_gain_loss != 0) {
                    hb.startSpan(CurrencyModel::instance().toCurrency(m_unreal_gain_loss_excl_forex), wxString::Format(" style='text-align:right;%s' nowrap"
                        , m_unreal_gain_loss_excl_forex < 0 ? "color:red;" : ""));
                    hb.endSpan();
                    hb.startSpan(" + ", "");
                    hb.endSpan();
                    hb.startSpan(CurrencyModel::instance().toCurrency(forex_unreal_gain_loss), wxString::Format(" style='text-align:right;%s' nowrap"
                        , forex_unreal_gain_loss < 0 ? "color:red;" : ""));
                    hb.endSpan();
                    hb.startSpan(" FX", "");
                    hb.endSpan();
                    hb.addLineBreak();
                }
                hb.startSpan(CurrencyModel::instance().toCurrency(m_unreal_gain_loss_sum_total), wxString::Format(" style='text-align:right;%s' nowrap"
                    , m_unreal_gain_loss_sum_total < 0 ? "color:red;" : ""));
                hb.endSpan();

                hb.endTableCell();

                hb.startTableCell(" style='text-align:right;' nowrap");
                hb.startSpan(CurrencyModel::instance().toCurrency(m_stock_balance), "");
                hb.endSpan();

                hb.endTableCell();
            }
            hb.endTfoot();
        }
        hb.endTable();
    }
    hb.endDiv();

    hb.end();

    return hb.getHTMLText();
}

mmReportChartStocks::mmReportChartStocks()
    : ReportBase(_n("Stocks Performance Charts"))
{
    setReportParameters(REPORT_ID::StocksReportPerformance);
}

mmReportChartStocks::~mmReportChartStocks()
{
}

wxString mmReportChartStocks::getHTMLText()
{
    // Build the report
    mmHTMLBuilder hb;
    hb.init();
    hb.addReportHeader(getTitle(), m_date_range->startDay(), m_date_range->isFutureIgnored());
    wxTimeSpan dtDiff = m_date_range->end_date() - m_date_range->start_date();
    if (m_date_range->is_with_date() && dtDiff.GetDays() <= 366)
        hb.displayDateHeading(m_date_range->start_date(), m_date_range->end_date(), true);

    wxTimeSpan dist;
    wxArrayString symbols;
    /*for (const auto& stock_d : StockModel::instance().find_all(
        StockCol::COL_ID_SYMBOL
    )) {
        const AccountData* account = AccountModel::instance().get_id_data_n(
            stock_d.m_account_id_n
        );
        if (!account->is_open())
            continue;
        if (symbols.Index(stock_d.m_symbol) != wxNOT_FOUND)
            continue;

        symbols.Add(stock_d.m_symbol);
    */
    StockModel::Data stock;
    bool found = false;
    for (StockModel::Data stock_d : StockModel::instance().find_all()) {
        if (stock_d.m_name == m_stock_name) {
            stock = stock_d;
            found = true;
            break;
        }
    }
    /*for (const auto& stock : StockModel::instance().all(StockModel::COL_SYMBOL))
    {
        AccountModel::Data* account = AccountModel::instance().get_id(stock.HELDAT);
        if (AccountModel::status_id(account) != AccountModel::STATUS_ID_OPEN) continue;
        if (symbols.Index(stock.SYMBOL) != wxNOT_FOUND) continue;
    */
    if (found) {
        const AccountModel::Data* account = AccountModel::instance().get_id_data_n(stock.m_account_id_n);
        symbols.Add(stock.m_symbol);
        int dataCount = 0, freq = 1;
        auto sh_a = StockHistoryModel::instance().find(
            StockHistoryCol::SYMBOL(stock.m_symbol),
            StockHistoryModel::DATE(OP_GE, m_date_range->start_date()),
            StockHistoryModel::DATE(OP_LE, m_date_range->end_date())
        );
        std::stable_sort(sh_a.begin(), sh_a.end(), StockHistoryData::SorterByDATE());

        //bool showGridLines = (sh_a.size() <= 366);
        //bool pointDot = (sh_a.size() <= 30);
        if (sh_a.size() > 366) {
            freq = sh_a.size() / 366;
        }

        GraphData gd;
        GraphSeries data;

        for (const auto& sh_d : sh_a) {
            if (dataCount % freq == 0) {
                gd.labels.push_back(sh_d.m_date.isoDate());
                data.values.push_back(sh_d.m_price);
            }
            dataCount++;
        }
        gd.series.push_back(data);

        if (!gd.series.empty()) {
            hb.addHeader(1, wxString::Format("%s / %s - (%s)",
                stock.m_symbol, stock.m_name, account->m_name
            ));
            gd.type = GraphData::LINE_DATETIME;
            hb.addChart(gd);
        }
    }
    hb.endDiv();

    hb.end();

    wxLogDebug("======= mmReportChartStocks:getHTMLText =======");
    wxLogDebug("%s", hb.getHTMLText());

    return hb.getHTMLText();
}
