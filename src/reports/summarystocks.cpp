/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#include "summarystocks.h"
#include "reports/htmlbuilder.h"

#include "constants.h"
#include "stockspanel.h"
#include "budget.h"
#include "util.h"
#include "reports/mmDateRange.h"
#include "Model_Account.h"
#include "Model_Currency.h"
#include "Model_CurrencyHistory.h"
#include "Model_StockHistory.h"

#include <algorithm>

#define STOCK_SORT_BY_NAME          1
#define STOCK_SORT_BY_SYMBOL        2
#define STOCK_SORT_BY_DATE          3
#define STOCK_SORT_BY_QTY           4
#define STOCK_SORT_BY_PUR_PRICE     5
#define STOCK_SORT_BY_CUR_PRICE     6
#define STOCK_SORT_BY_COMMISSION    7
#define STOCK_SORT_BY_GAIN_LOSS     8
#define STOCK_SORT_BY_VALUE         9

mmReportSummaryStocks::mmReportSummaryStocks()
    : mmPrintableBase(_("Summary of Stocks"))
    , m_gain_loss_sum_total(0.0)
    , m_stock_balance(0.0)
{
}

void  mmReportSummaryStocks::RefreshData()
{
    m_stocks.clear();
    m_gain_loss_sum_total = 0.0;
    m_stock_balance = 0.0;

    data_holder line;
    account_holder account;
    const wxDate today = wxDate::Today();

    for (const auto& a : Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
    {
        if (Model_Account::type(a) != Model_Account::INVESTMENT) continue;
        if (Model_Account::status(a) != Model_Account::OPEN) continue;

        account.id = a.id();
        account.name = a.ACCOUNTNAME;
        account.gainloss = 0.0;
        account.total = Model_Account::investment_balance(a).first;
        account.data.clear();

        for (const auto& stock : Model_Stock::instance().find(Model_Stock::HELDAT(a.ACCOUNTID)))
        {
            const Model_Currency::Data* currency = Model_Account::currency(a);
            const double today_rate = Model_CurrencyHistory::getDayRate(currency->CURRENCYID, today);
            m_stock_balance += today_rate * Model_Stock::CurrentValue(stock);
            account.gainloss += Model_Stock::CurrentValue(stock) - Model_Stock::InvestmentValue(stock);
            const double purchase_rate = Model_CurrencyHistory::getDayRate(currency->CURRENCYID, stock.PURCHASEDATE);
            m_gain_loss_sum_total += (Model_Stock::CurrentValue(stock) * today_rate - Model_Stock::InvestmentValue(stock) * purchase_rate);

            line.name = stock.STOCKNAME;
            line.symbol = stock.SYMBOL;
            line.date = stock.PURCHASEDATE;
            line.qty = stock.NUMSHARES;
            line.purchase = Model_Stock::InvestmentValue(stock);
            line.current = stock.CURRENTPRICE;
            line.commission = stock.COMMISSION;
            line.gainloss = Model_Stock::CurrentValue(stock) - Model_Stock::InvestmentValue(stock);
            line.value = Model_Stock::CurrentValue(stock);
            account.data.push_back(line);
        }
        m_stocks.push_back(account);
    }
}

wxString mmReportSummaryStocks::getHTMLText()
{
    RefreshData();
    mmHTMLBuilder hb;
    hb.init();
    hb.addDivContainer();
    hb.addHeader(2, m_title);
    hb.addDateNow();
    hb.addLineBreak();

    hb.addDivRow();
    hb.addDivCol17_67();

    for (const auto& acct : m_stocks)
    {
        const Model_Account::Data* account = Model_Account::instance().get(acct.id);
        const Model_Currency::Data* currency = Model_Account::currency(account);
        hb.addHeader(3, acct.name);

        hb.startTable();
        display_header(hb);

        hb.startTbody();
        for (const auto& entry : acct.data)
        {
            hb.startTableRow();
            hb.addTableCell(entry.name);
            hb.addTableCell(entry.symbol);
            hb.addTableCellDate(entry.date);
            hb.addTableCell(Model_Account::toString(entry.qty, account, floor(entry.qty) ? 0 : 4), true);
            hb.addCurrencyCell(entry.purchase, currency, 4);
            hb.addCurrencyCell(entry.current, currency, 4);
            hb.addCurrencyCell(entry.commission, currency, 4);
            hb.addCurrencyCell(entry.gainloss, currency);
            hb.addCurrencyCell(entry.value, currency);
            hb.endTableRow();
        }
        hb.endTbody();

        hb.startTfoot();
        hb.startTotalTableRow();
        hb.addTableCell(_("Total:"));
        hb.addEmptyTableCell(6);
        hb.addCurrencyCell(acct.gainloss, currency);
        hb.addCurrencyCell(acct.total, currency);
        hb.endTableRow();
        hb.endTfoot();
        hb.endTable();
    }

    hb.addDivCol17_67();
    hb.addHeader(3, _("Grand Total:"));
    hb.startTable();

    hb.startThead();
    hb.startTableRow();
    hb.addTableHeaderCell(_("Gain/Loss"), true);
    hb.addTableHeaderCell(_("Current Value"), true);
    hb.endTableRow();
    hb.endThead();

    hb.startTfoot();
    hb.startTotalTableRow();
    hb.addCurrencyCell(m_gain_loss_sum_total);
    hb.addCurrencyCell(m_stock_balance);
    hb.endTableRow();
    hb.endTfoot();
    hb.endTable();
    hb.endDiv();

    hb.endDiv();
    hb.endDiv();

    hb.endDiv();
    hb.end();

    return hb.getHTMLText();
}

void mmReportSummaryStocks::display_header(mmHTMLBuilder& hb) 
{
    hb.startThead();
    hb.startTableRow();
    hb.addTableHeaderCell(_("Name"));
    hb.addTableHeaderCell(_("Symbol"));
    hb.addTableHeaderCell(_("*Purchase Date"));
    hb.addTableHeaderCell(_("Quantity"), true);
    hb.addTableHeaderCell(_("Initial Value"), true);
    hb.addTableHeaderCell(_("Current Price"), true);
    hb.addTableHeaderCell(_("Commission"), true);
    hb.addTableHeaderCell(_("Gain/Loss"), true);
    hb.addTableHeaderCell(_("Current Value"), true);
    hb.endTableRow();
    hb.endThead();
}

mmReportChartStocks::mmReportChartStocks()
    : mmPrintableBase(_("Stocks Performance Charts"))
{
}

mmReportChartStocks::~mmReportChartStocks()
{
}

int mmReportChartStocks::report_parameters()
{
    return RepParams::DATE_RANGE;
}

wxString mmReportChartStocks::getHTMLText()
{
    mmHTMLBuilder hb;
    hb.init();
    hb.addDivContainer();
    hb.addHeader(2, title());
    hb.addDateNow();

    wxTimeSpan dtDiff = m_date_range->end_date() - m_date_range->start_date();
    if (m_date_range->is_with_date() && dtDiff.GetDays() <= 366)
        hb.DisplayDateHeading(m_date_range->start_date(), m_date_range->end_date(), true);
    hb.addHorizontalLine();

    int count = 0;
    bool pointDot = false, showGridLines = false;
    wxTimeSpan dist;
    wxDate precDateDt = wxInvalidDateTime;
    for (const auto& stock : Model_Stock::instance().all(Model_Stock::COL_HELDAT))
    {
        int dataCount = 0, freq = 1;
        Model_StockHistory::Data_Set histData = Model_StockHistory::instance().find(Model_StockHistory::SYMBOL(stock.SYMBOL),
            Model_StockHistory::DATE(m_date_range->start_date(), GREATER_OR_EQUAL),
            Model_StockHistory::DATE(m_date_range->end_date(), LESS_OR_EQUAL));
        std::stable_sort(histData.begin(), histData.end(), SorterByDATE());
        if (histData.size() <= 30)
            showGridLines = pointDot = true;
        else if (histData.size() <= 366)
            showGridLines = true;
        else
            freq = histData.size() / 366;
        std::vector<ValueTrio> aData;
        for (const auto& hist : histData)
        {
            if (dataCount % freq == 0)
            {
                ValueTrio val;
                const wxDate dateDt = Model_StockHistory::DATE(hist);
                if (histData.size() <= 30)
                    val.label = mmGetDateForDisplay(hist.DATE);
                else if (precDateDt.IsValid() && dateDt.GetMonth() != precDateDt.GetMonth())
                    val.label = dateDt.GetEnglishMonthName(dateDt.GetMonth());
                else
                    val.label = "";
                val.amount = hist.VALUE;
                aData.push_back(val);
                precDateDt = dateDt;
            }
            dataCount++;
        }
        if (!aData.empty())
        {
            hb.addDivRow();
            Model_Account::Data* account = Model_Account::instance().get(stock.HELDAT);
            hb.addHeader(1, wxString::Format("%s - (%s)", stock.STOCKNAME, account->ACCOUNTNAME));
            hb.addDivCol17_67();
            hb.addLineChart(aData, stock.STOCKNAME, count, 1000, 400, pointDot, showGridLines, true);
            hb.endDiv();
            hb.endDiv();
        }

        count++;
    }

    hb.endDiv();
    hb.end();

    return hb.getHTMLText();
}
