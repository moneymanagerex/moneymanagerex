/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
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

#include "summary.h"
#include "constants.h"
#include "reports/htmlbuilder.h"
#include "Model_Account.h"
#include "Model_CurrencyHistory.h"
#include "Model_Stock.h"
#include "Model_StockHistory.h"
#include <algorithm>

class mmHistoryItem
{
public:
    mmHistoryItem();

    int         acctId;
    int         stockId;
    wxDate      purchaseDate;
    wxString    purchaseDateStr;
    double      purchasePrice;
    double      numShares;
    Model_StockHistory::Data_Set stockHist;
};

mmHistoryItem::mmHistoryItem()
{
    acctId = stockId = 0;
    purchasePrice = numShares = 0.0;
}

class mmHistoryData : public std::vector<mmHistoryItem>
{
public:
    double getDailyBalanceAt(const Model_Account::Data *account, const wxDate& date);
};

double mmHistoryData::getDailyBalanceAt(const Model_Account::Data *account, const wxDate& date)
{
    const wxString& strDate = date.FormatISODate();
    std::map<int, double> totBalance;

    for (const auto & stock : *this)
    {
        if (stock.acctId != account->id()) {
            continue;
        }

        wxString precValueDate, nextValueDate;

        double valueAtDate = 0.0, precValue = 0.0, nextValue = 0.0;

        for (const auto & hist : stock.stockHist)
        {
            // test for the date requested
            if (hist.DATE == strDate)
            {
                valueAtDate = hist.VALUE;
                break;
            }
            // if not found, search for previous and next date
            if (precValue == 0.0 && hist.DATE < strDate)
            {
                precValue = hist.VALUE;
                precValueDate = hist.DATE;
            }
            if (hist.DATE > strDate)
            {
                nextValue = hist.VALUE;
                nextValueDate = hist.DATE;
            }
            // end conditions: prec value assigned and price date < requested date
            if (precValue != 0.0 && hist.DATE < strDate) {
                break;
            }
        }
        if (valueAtDate == 0.0)
        {
            //  if previous not found but if the given date is after purchase date, takes purchase price
            if (precValue == 0.0 && date >= stock.purchaseDate)
            {
                precValue = stock.purchasePrice;
                precValueDate = stock.purchaseDateStr;
            }
            //  if next not found and the accoung is open, takes previous date
            if (nextValue == 0.0 && Model_Account::status(account) == Model_Account::OPEN)
            {
                nextValue = precValue;
                nextValueDate = precValueDate;
            }
            if (precValue > 0.0 && nextValue > 0.0 && precValueDate >= stock.purchaseDateStr && nextValueDate >= stock.purchaseDateStr) {
                valueAtDate = precValue;
            }
        }

        totBalance[stock.stockId] += stock.numShares * valueAtDate;
    }

    double balance = 0.0;
    for (const auto& it : totBalance)
        balance += it.second;

    return balance;
}

mmReportSummaryByDate::mmReportSummaryByDate(int mode)
: mmPrintableBase("mmReportSummaryByDate")
, mode_(mode)
{}

wxString mmReportSummaryByDate::getHTMLText()
{
    size_t account_types_num = Model_Account::INVESTMENT + 1;
    double          total = 0.0;
    std::map<size_t, double> balancePerDay;
    mmHTMLBuilder   hb;
    wxString        datePrec;
    wxDate          date, dateStart = wxDate::Today(), dateEnd = wxDate::Today();
    wxDateSpan      span;
    mmHistoryItem   *pHistItem;
    mmHistoryData   arHistory;
    std::vector<balanceMap> balanceMapVec(Model_Account::instance().all().size());
    std::vector<std::map<wxDate, double>::const_iterator> arIt(balanceMapVec.size());
    std::vector<double> arBalance(balanceMapVec.size());
    std::vector<wxString> totBalanceData;
    std::vector<wxDate> arDates;

    hb.init();
    hb.addDivContainer();
    hb.addHeader(2, wxString::Format(_("Accounts Balance - %s")
        , mode_ == 0 ? _("Monthly Report") : _("Yearly Report")));
    hb.addDateNow();
    hb.addLineBreak();

    hb.startTable();
    hb.startThead();
    hb.startTableRow();
    hb.addTableHeaderCell(_("Date"));
    hb.addTableHeaderCell(_("Cash"), true);
    hb.addTableHeaderCell(_("Bank Accounts"), true);
    hb.addTableHeaderCell(_("Credit Card Accounts"), true);
    hb.addTableHeaderCell(_("Loan Accounts"), true);
    hb.addTableHeaderCell(_("Term Accounts"), true);
    hb.addTableHeaderCell(_("Crypto Wallets"), true);
    hb.addTableHeaderCell(_("Total"), true);
    hb.addTableHeaderCell(_("Stocks"), true);
    hb.addTableHeaderCell(_("Balance"), true);
    hb.endTableRow();
    hb.endDiv();
    hb.endThead();

    int i = 0;
    for (const auto& account: Model_Account::instance().all())
    {
        if (Model_Account::type(account) != Model_Account::INVESTMENT)
        {
            //  in balanceMapVec ci sono i totali dei movimenti giorno per giorno
            const Model_Currency::Data* currency = Model_Account::currency(account);
            for (const auto& tran : Model_Account::transaction(account))
            {
                balanceMapVec[i][Model_Checking::TRANSDATE(tran)]
                    += Model_Checking::balance(tran, account.ACCOUNTID)
                    * Model_CurrencyHistory::getDayRate(currency->id(), tran.TRANSDATE);
            }
            if (Model_Account::type(account) != Model_Account::TERM && balanceMapVec[i].size())
            {
                date = balanceMapVec[i].begin()->first;
                if (date.IsEarlierThan(dateStart)) {
                    dateStart = date;
                }
            }
            arBalance[i] = account.INITIALBAL * Model_CurrencyHistory::getDayRate(currency->id(), dateStart);
        }
        else
        {
            Model_Stock::Data_Set stocks = Model_Stock::instance().find(Model_Stock::HELDAT(account.id()));
            for (const auto & stock : stocks)
            {
                arHistory.resize(arHistory.size() + 1);
                pHistItem = arHistory.data() + arHistory.size() - 1;
                pHistItem->acctId = account.id();
                pHistItem->stockId = stock.STOCKID;
                pHistItem->purchasePrice = stock.PURCHASEPRICE;
                pHistItem->purchaseDate = Model_Stock::PURCHASEDATE(stock);
                pHistItem->purchaseDateStr = stock.PURCHASEDATE;
                pHistItem->numShares = stock.NUMSHARES;
                pHistItem->stockHist = Model_StockHistory::instance().find(Model_StockHistory::SYMBOL(stock.SYMBOL));
                std::stable_sort(pHistItem->stockHist.begin(), pHistItem->stockHist.end(), SorterByDATE());
                std::reverse(pHistItem->stockHist.begin(), pHistItem->stockHist.end());
            }
        }
        i++;
    }

    if (mode_ == 0)
    {
        dateEnd -= wxDateSpan::Months(1);
        dateEnd.SetToLastMonthDay(dateEnd.GetMonth(), dateEnd.GetYear());
        span = wxDateSpan::Months(1);
    }
    else if (mode_ == 1)
    {
        dateEnd.Set(31, wxDateTime::Dec, wxDateTime::Now().GetYear());
        span = wxDateSpan::Years(1);
    }
    else
        wxASSERT(0);

    date = dateEnd;
    while (date.IsLaterThan(dateStart)) {
        date -= span;
    }
    dateStart = date;

    int c = 0;
    for (const auto& acctMap : balanceMapVec) {
        arIt[c++] = acctMap.begin();
    }

    //  prepare the dates array
    while (dateStart <= dateEnd)
    {
        if (mode_ == 0) {
            dateStart.SetToLastMonthDay(dateStart.GetMonth(), dateStart.GetYear());
        }
        arDates.push_back(dateStart);
        dateStart += span;
    }
    date = wxDate::Today().SetToLastMonthDay();
    if (date.GetDay() != dateEnd.GetDay()
        || date.GetMonth() != dateEnd.GetMonth()
        || date.GetYear() != dateEnd.GetYear()) {
        arDates.push_back(date);
    }


    for (const auto & dd : arDates)
    {
        int k = 0;
        for (auto& account: Model_Account::instance().all())
        {
            if (Model_Account::type(account) != Model_Account::INVESTMENT)
            {
                for (; arIt[k] != balanceMapVec[k].end(); ++arIt[k])
                {
                    if (arIt[k]->first.IsLaterThan(dd)) {
                        break;
                    }
                    arBalance[k] += arIt[k]->second;
                }
            }
            else
            {
                double convRate = 1.0;
                Model_Currency::Data* currency = Model_Account::currency(account);
                if (currency) {
                    convRate = Model_CurrencyHistory::getDayRate(currency->id(), dd);
                }
                arBalance[k] = arHistory.getDailyBalanceAt(&account, dd) * convRate;
            }
            k++;
        }

        // prepare columns for report: date, cash, checking, credit card, term, partial total, investment, grand total
        totBalanceData.push_back(dd.FormatISODate());
        for (size_t j = 0; j < account_types_num; j++) {
            balancePerDay[j] = 0.0;
        }
        int a = 0;
        for (const auto& account: Model_Account::instance().all())
        {
            double t = arBalance[a++];
            balancePerDay[Model_Account::type(account)] += t;
        }

        totBalanceData.push_back(Model_Currency::toCurrency(balancePerDay[Model_Account::CASH]));
        totBalanceData.push_back(Model_Currency::toCurrency(balancePerDay[Model_Account::CHECKING]));
        totBalanceData.push_back(Model_Currency::toCurrency(balancePerDay[Model_Account::CREDIT_CARD]));
        totBalanceData.push_back(Model_Currency::toCurrency(balancePerDay[Model_Account::LOAN]));
        totBalanceData.push_back(Model_Currency::toCurrency(balancePerDay[Model_Account::TERM]));
        totBalanceData.push_back(Model_Currency::toCurrency(balancePerDay[Model_Account::CRYPTO]));

        total = balancePerDay[Model_Account::CASH] + balancePerDay[Model_Account::CHECKING] 
            + balancePerDay[Model_Account::CREDIT_CARD] + balancePerDay[Model_Account::LOAN] 
            + balancePerDay[Model_Account::TERM] + balancePerDay[Model_Account::CRYPTO];

        totBalanceData.push_back(Model_Currency::toCurrency(total));
        totBalanceData.push_back(Model_Currency::toCurrency(balancePerDay[Model_Account::INVESTMENT]));
        total += balancePerDay[Model_Account::INVESTMENT];
        totBalanceData.push_back(Model_Currency::toCurrency(total));
    }

    hb.startTbody();
    int x = 1 + (account_types_num) + 2;
    for (int k = totBalanceData.size() - x; k >= 0; k -= x)
    {
        if (datePrec.Left(4) != totBalanceData[k].Left(4))
        {
            hb.startTotalTableRow();
            hb.addTableCell(totBalanceData[k].Left(4));
            for (int j = 0; j < x - 1; j++) {
                hb.addTableCell("");
            }
            hb.endTableRow();
        }
        hb.startTableRow();
        hb.addTableCellDate(totBalanceData[k]);
        for (int j = 0; j < x - 1; j++) {
            hb.addTableCell(totBalanceData[k + j + 1], true);
        }
        hb.endTableRow();
        datePrec = totBalanceData[k];
    }
    hb.endTbody();

    hb.endTable();
    hb.end();

    return hb.getHTMLText();
}
