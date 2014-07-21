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
#include "htmlbuilder.h"

#include "constants.h"
#include "stockspanel.h"
#include "util.h"
#include "model/Model_Account.h"
#include "model/Model_Currency.h"
#include "model/Model_StockHistory.h"
#include "budget.h"

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
: mmPrintableBase(STOCK_SORT_BY_NAME)
, gain_loss_sum_total_(0.0)
, stockBalance_(0.0)
{
}

void  mmReportSummaryStocks::RefreshData()
{
    stocks_.clear();
    gain_loss_sum_total_ = 0.0;
    stockBalance_ = 0.0;

    data_holder line;
    account_holder account;
    for (const auto& a : Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
    {
        if (Model_Account::type(a) != Model_Account::INVESTMENT) continue;
        if (Model_Account::status(a) != Model_Account::OPEN) continue;

        account.id = a.id();
        account.name = a.ACCOUNTNAME;
        account.gainloss = 0.0;
        account.total = Model_Account::investment_balance(a).second;
        account.data.clear();

        for (const auto& stock : Model_Stock::instance().find(Model_Stock::HELDAT(a.ACCOUNTID)))
        {
            const Model_Currency::Data* currency = Model_Account::currency(a);
            stockBalance_ += currency->BASECONVRATE * stock.VALUE;
            account.gainloss += stock.VALUE - Model_Stock::value(stock);
            gain_loss_sum_total_ += (stock.VALUE - Model_Stock::value(stock)) * currency->BASECONVRATE;

            line.name = stock.STOCKNAME;
            line.symbol = stock.SYMBOL;
            line.date = mmGetDateForDisplay(Model_Stock::PURCHASEDATE(stock));
            line.qty = stock.NUMSHARES;
            line.purchase = stock.PURCHASEPRICE;
            line.current = stock.CURRENTPRICE;
            line.commission = stock.COMMISSION;
            line.gainloss = stock.VALUE - Model_Stock::value(stock);
            line.value = stock.VALUE;
            account.data.push_back(line);
        }
        stocks_.push_back(account);
    }
}

wxString mmReportSummaryStocks::getHTMLText()
{
    for (auto& acct : stocks_)
    {
        switch (sortColumn_)
        {
        case STOCK_SORT_BY_SYMBOL:
            std::stable_sort(acct.data.begin(), acct.data.end()
                , [] (const data_holder& x, const data_holder& y)
                {
                    return x.symbol < y.symbol;
                }
            );
            break;
        case STOCK_SORT_BY_DATE:
            std::stable_sort(acct.data.begin(), acct.data.end()
                , [] (const data_holder& x, const data_holder& y)
                {
                    if (x.date != y.date) return x.date < y.date;
                    else return x.name < y.name;
                }
            );
            break;
        case STOCK_SORT_BY_QTY:
            std::stable_sort(acct.data.begin(), acct.data.end()
                , [] (const data_holder& x, const data_holder& y)
                {
                    if (x.qty != y.qty) return x.qty < y.qty;
                    else return x.name < y.name;
                }
            );
            break;
        case STOCK_SORT_BY_PUR_PRICE:
            std::stable_sort(acct.data.begin(), acct.data.end()
                , [] (const data_holder& x, const data_holder& y)
                {
                    if (x.purchase != y.purchase) return x.purchase < y.purchase;
                    else return x.name < y.name;
                }
            );
            break;
        case STOCK_SORT_BY_CUR_PRICE:
            std::stable_sort(acct.data.begin(), acct.data.end()
                , [] (const data_holder& x, const data_holder& y)
                {
                    if (x.current != y.current) return x.current < y.current;
                    else return x.name < y.name;
                }
            );
            break;
        case STOCK_SORT_BY_COMMISSION:
            std::stable_sort(acct.data.begin(), acct.data.end()
                , [] (const data_holder& x, const data_holder& y)
                {
                    if (x.commission != y.commission) return x.commission < y.commission;
                    else return x.name < y.name;
                }
            );
            break;
        case STOCK_SORT_BY_GAIN_LOSS:
            std::stable_sort(acct.data.begin(), acct.data.end()
                , [] (const data_holder& x, const data_holder& y)
                {
                    if (x.gainloss != y.gainloss) return x.gainloss < y.gainloss;
                    else return x.name < y.name;
                }
            );
            break;
        case STOCK_SORT_BY_VALUE:
            std::stable_sort(acct.data.begin(), acct.data.end()
                , [] (const data_holder& x, const data_holder& y)
                {
                    if (x.value != y.value) return x.value < y.value;
                    else return x.name < y.name;
                }
            );
            break;
        default:
            sortColumn_ = STOCK_SORT_BY_NAME;
            std::stable_sort(acct.data.begin(), acct.data.end()
                , [] (const data_holder& x, const data_holder& y)
                {
                    return x.name < y.name;
                }
            );
        }
    }

    mmHTMLBuilder hb;
    hb.init();

    hb.addHeader(2, _("Summary of Stocks"));
    hb.addDateNow();

    hb.startTable();
    for (const auto& acct : stocks_)
    {
        const Model_Account::Data* account = Model_Account::instance().get(acct.id);
        const Model_Currency::Data* currency = Model_Account::currency(account);
        hb.addTotalRow("", 9, "");
        hb.addTotalRow(acct.name, 9, "");

        display_header(hb);

        for (const auto& entry : acct.data)
        {
            hb.startTableRow();
            hb.addTableCell(entry.name);
            hb.addTableCell(entry.symbol);
            hb.addTableCell(entry.date);
            hb.addTableCell(Model_Account::toString(entry.qty, account, 4));
            hb.addCurrencyCell(entry.purchase, currency);
            hb.addCurrencyCell(entry.current, currency);
            hb.addCurrencyCell(entry.commission, currency);
            hb.addCurrencyCell(entry.gainloss, currency);
            hb.addCurrencyCell(entry.value, currency);
            hb.endTableRow();
        }

        //TODO: hb.addRowSeparator(9);
        hb.addTotalRow(_("Total:"), 8, acct.gainloss);
        hb.addTotalRow("", 9, acct.total);
    }

    //TODO: hb.addRowSeparator(9);
    hb.addTotalRow(_("Grand Total:"), 8, gain_loss_sum_total_);
    hb.addTotalRow("", 9, stockBalance_);
    hb.endTableRow();
    hb.endTable();

    hb.end();
    return hb.getHTMLText();
}

void mmReportSummaryStocks::display_header(mmHTMLBuilder& hb) 
{
    hb.startTableRow();
    hb.addTableHeaderCell(_("Name"));
    hb.addTableHeaderCell(_("Symbol"));
    hb.addTableHeaderCell(_("Purchase Date"));
    hb.addTableHeaderCell(_("Quantity"), true);
    hb.addTableHeaderCell(_("Purchase Price"), true);
    hb.addTableHeaderCell(_("Current Price"), true);
    hb.addTableHeaderCell(_("Commission"), true);
    hb.addTableHeaderCell(_("Gain/Loss"), true);
    hb.addTableHeaderCell(_("Value"), true);
    hb.endTableRow();
}

wxString mmReportChartStocks::getHTMLText()
{
    mmHTMLBuilder hb;
    hb.init();

    hb.addHeader(2, _("Stocks Performance Charts"));

    wxTimeSpan dtDiff = dtRange_->end_date() - dtRange_->start_date();
    if( dtRange_->is_with_date() && dtDiff.GetDays() <= 366 )
        hb.DisplayDateHeading(dtRange_->start_date(), dtRange_->end_date(), true);

    //hb.startTable("95%");
    int index = 0, heldAt = -1;
    wxString dateStr, strSql;
    wxTimeSpan dist;
    wxDateTime dateDt, prevDt;
    std::vector<ChartData> aData;
    for (const auto& stock: Model_Stock::instance().all(Model_Stock::COL_HELDAT))
    {
        if (heldAt != stock.HELDAT)
        {
            Model_Account::Data* account = Model_Account::instance().get(stock.HELDAT);
            if (account)
                hb.addHeader(1, account->ACCOUNTNAME);
        }

        prevDt = wxInvalidDateTime;
        Model_StockHistory::Data_Set histData = Model_StockHistory::instance().find(Model_StockHistory::STOCKID(stock.id()),
                                                    Model_StockHistory::DATE(dtRange_->start_date(), GREATER_OR_EQUAL),
                                                    Model_StockHistory::DATE(dtRange_->end_date(), LESS_OR_EQUAL));
        std::stable_sort(histData.begin(), histData.end(), SorterByDATE());
        for (const auto& hist : histData)
        {
            aData.push_back(ChartData(hist.DATE, hist.VALUE));
            prevDt = dateDt;
        }
        if (aData.size())
        {
            mmGraphHistoryStocks gg;
            gg.init(aData, index);
            gg.Generate("");
            hb.addImage(gg.getOutputFileName());
            hb.addHeader(1, stock.STOCKNAME);
        }

        aData.clear();
        index++;
        heldAt = stock.HELDAT;
    }

    hb.endTable();

    hb.end();

    return hb.getHTMLText();
}

mmGraphHistoryStocks::mmGraphHistoryStocks() : 
        mmGraphGenerator(),
        chart(new GraphChart(800, 400))
{
}

mmGraphHistoryStocks::~mmGraphHistoryStocks()
{
}

void mmGraphHistoryStocks::init(std::vector<ChartData> aData, int indColor)
{
    chart->SetData(aData);
    chart->Init(800, indColor);
}

bool mmGraphHistoryStocks::Generate(const wxString& chartTitle)
{
    chart->Render(chartTitle);
    return chart->Save(getOutputFileName());
}
