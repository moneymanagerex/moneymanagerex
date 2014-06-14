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

    hb.startTable("95%");
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
            hb.addTableCell(entry.name, false, true);
            hb.addTableCell(entry.symbol);
            hb.addTableCell(entry.date);
            hb.addTableCell(Model_Account::toString(entry.qty, account, 4), true);
            hb.addCurrencyCell(entry.purchase, currency, -1, 4);
            hb.addCurrencyCell(entry.current, currency, -1, 4);
            hb.addCurrencyCell(entry.commission, currency, -1, 4);
            hb.addCurrencyCell(entry.gainloss, currency);
            hb.addCurrencyCell(entry.value, currency);
            hb.endTableRow();
        }

        hb.addRowSeparator(9);
        hb.addTotalRow(_("Total:"), 8, acct.gainloss);
        hb.addTotalRow("", 9, acct.total);
    }

    hb.addRowSeparator(9);
    hb.addTotalRow(_("Grand Total:"), 8, gain_loss_sum_total_);
    hb.addTotalRow("", 9, stockBalance_);
    hb.endTableRow();
    hb.endTable();

    hb.endCenter();
    hb.end();
    return hb.getHTMLText();
}

void mmReportSummaryStocks::display_header(mmHTMLBuilder& hb) 
{
    hb.startTableRow();
    if(STOCK_SORT_BY_NAME == sortColumn_)
        hb.addTableHeaderCell(_("Name"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", STOCK_SORT_BY_NAME), _("Name"));
    if(STOCK_SORT_BY_SYMBOL == sortColumn_)
        hb.addTableHeaderCell(_("Symbol"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", STOCK_SORT_BY_SYMBOL), _("Symbol"));
    if(STOCK_SORT_BY_DATE == sortColumn_)
        hb.addTableHeaderCell(_("Purchase Date"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", STOCK_SORT_BY_DATE), _("Purchase Date"));
    if(STOCK_SORT_BY_QTY == sortColumn_)
        hb.addTableHeaderCell(_("Quantity"), true);
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", STOCK_SORT_BY_QTY), _("Quantity"), true);
    if(STOCK_SORT_BY_PUR_PRICE == sortColumn_)
        hb.addTableHeaderCell(_("Purchase Price"), true);
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", STOCK_SORT_BY_PUR_PRICE), _("Purchase Price"), true);
    if(STOCK_SORT_BY_CUR_PRICE == sortColumn_)
        hb.addTableHeaderCell(_("Current Price"), true);
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", STOCK_SORT_BY_CUR_PRICE), _("Current Price"), true);
    if(STOCK_SORT_BY_COMMISSION == sortColumn_)
        hb.addTableHeaderCell(_("Commission"), true);
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", STOCK_SORT_BY_COMMISSION), _("Commission"), true);
    if(STOCK_SORT_BY_GAIN_LOSS == sortColumn_)
        hb.addTableHeaderCell(_("Gain/Loss"), true);
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", STOCK_SORT_BY_GAIN_LOSS), _("Gain/Loss"), true);
    if(STOCK_SORT_BY_VALUE == sortColumn_)
        hb.addTableHeaderCell(_("Value"), true);
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", STOCK_SORT_BY_VALUE), _("Value"), true);
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
    wxDateTime dateDt, prevDt, lastInsDt;
    std::vector<ChartData> aData;
    for (const auto& stock: Model_Stock::instance().all(Model_Stock::COL_HELDAT))
    {
        if (heldAt != stock.HELDAT)
        {
            Model_Account::Data* account = Model_Account::instance().get(stock.HELDAT);
            if (account)
                hb.addHeaderItalic(4, account->ACCOUNTNAME);
        }

        prevDt = lastInsDt = wxInvalidDateTime;
        Model_StockHistory::Data_Set histData;
        histData = Model_StockHistory::instance().search(stock.id(), true, 0, dtRange_->start_date(), dtRange_->end_date());
        for (const auto& hist : histData)
        {
#if 0
            dateDt = mmGetStorageStringAsDate(dateStr);
            dateStr.Empty();
            if (prevDt.IsValid())
            {
                if (dtDiff.GetDays() <= 30)         // very low: every 5 days
                {
                    dist = dateDt - lastInsDt;
                    if (dist.GetDays() >= 5)
                        dateStr = dateDt.Format(wxS("%d/%m"));
                }
                else if (dtDiff.GetDays() <= 60)    // low: every 10 days
                {
                    dist = dateDt - lastInsDt;
                    if (dist.GetDays() >= 10)
                        dateStr = dateDt.Format(wxS("%d/%m"));
                }
                else if (dtDiff.GetDays() > 366)    // very high: every year
                {
                    if (prevDt.GetYear() != dateDt.GetYear())
                        dateStr = dateDt.Format(wxS("%Y"));
                }
                else                                // high: every month
                {
                    if (prevDt.GetMonth() != dateDt.GetMonth())
                        dateStr = dateDt.Format(wxS("%m/%Y"));
                }
                if (!dateStr.IsEmpty())
                    lastInsDt = dateDt;
            }
            else
            {
                dtDiff = dtEnd_ - dateDt;
                lastInsDt = dateDt;
            }
#endif
            aData.push_back(ChartData(hist.DATE, hist.VALUE));
            prevDt = dateDt;
        }
        if (aData.size())
        {
            mmGraphHistoryStocks gg;
            gg.init(aData, index);
            gg.Generate("");
            hb.startCenter();
            hb.addImage(gg.getOutputFileName());
            hb.endCenter();

            hb.startCenter();
            hb.addHeader(1, stock.STOCKNAME);
            hb.endCenter();
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
