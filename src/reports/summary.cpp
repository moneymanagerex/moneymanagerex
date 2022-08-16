/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2013 - 2022 Nikolay Akimov
 Copyright (C) 2017 James Higley
 Copyright (C) 2021 - 2022 Mark Whalley (mark@ipx.co.uk)

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
#include "htmlbuilder.h"
#include "model/allmodel.h"
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
    wxString strDate = date.FormatISODate();
    std::map<int, double> totBalance;

    for (const auto & stock : *this)
    {
        if (stock.acctId != account->id())
            continue;

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
            if (precValue != 0.0 && hist.DATE < strDate)
                break;
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
            if (precValue > 0.0 && nextValue > 0.0 && precValueDate >= stock.purchaseDateStr && nextValueDate >= stock.purchaseDateStr)
                valueAtDate = precValue;
        }

        totBalance[stock.stockId] += stock.numShares * valueAtDate;
    }

    double balance = 0.0;
    for (const auto& it : totBalance)
        balance += it.second;

    return balance;
}

mmReportSummaryByDate::mmReportSummaryByDate(int mode)
: mmPrintableBase(wxString::Format("Accounts Balance - %s", (mode == MONTHLY ? "Monthly" : "Yearly")))
, mode_(mode)
{
}

wxString mmReportSummaryByDate::getHTMLText()
{
    double          balancePerDay[Model_Account::MAX];
    mmHTMLBuilder   hb;
    wxDate          date = wxDate::Today();
    wxDate dateStart = date, dateEnd = date;
    wxDateSpan      span;
    mmHistoryItem   *pHistItem;
    mmHistoryData   arHistory;
    // Contains transactions totals day by day
    std::map<int, balanceMap> balanceMapVec;
    // Contains accounts initial balance
    std::map<int, double> arBalance;
    struct BalanceEntry
    {
        wxDate date;
        std::vector<double> values;
    };
    std::vector<BalanceEntry> totBalanceData;
    
    GraphData gd;
    GraphSeries gs_data[Model_Account::MAX + 1];    // +1 as we add balance to the end

    std::vector<wxDate> arDates;

    hb.init();
    const auto name = wxString::Format(_("Accounts Balance - %s"), mode_ == MONTHLY ? _("Monthly Report") : _("Yearly Report"));
    hb.addReportHeader(name);

    // Calculate the report data
    for (const auto& account: Model_Account::instance().all())
    {

        if (Model_Account::type(account) == Model_Account::INVESTMENT)
        {
            Model_Stock::Data_Set stocks = Model_Stock::instance().find(Model_Stock::HELDAT(account.id()));
            for (const auto& stock : stocks)
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
        else
        {
            for (const auto& tran : Model_Account::transaction(account))
            {
                balanceMapVec[account.ACCOUNTID][Model_Checking::TRANSDATE(tran)]
                    += Model_Checking::balance(tran, account.ACCOUNTID)
                    * Model_CurrencyHistory::getDayRate(account.CURRENCYID, tran.TRANSDATE);
            }

            if (Model_Account::type(account) != Model_Account::TERM && balanceMapVec[account.ACCOUNTID].size())
            {
                date = balanceMapVec[account.ACCOUNTID].begin()->first;
                if (date.IsEarlierThan(dateStart))
                    dateStart = date;
            }
            arBalance[account.ACCOUNTID] = account.INITIALBAL * Model_CurrencyHistory::getDayRate(account.CURRENCYID, dateStart);
        }
    }

    if (mode_ == MONTHLY)
    {
        dateEnd.SetToLastMonthDay(dateEnd.GetMonth(), dateEnd.GetYear());
        span = wxDateSpan::Months(1);
    }
    else if (mode_ == YEARLY)
    {
        dateEnd.Set(31, wxDateTime::Dec, dateEnd.GetYear());
        span = wxDateSpan::Years(1);
    }
    else
    {
        wxFAIL_MSG("unknown report mode");
    }

    //  prepare the dates array
    while (dateStart <= dateEnd) {
        if (mode_ == MONTHLY)
            dateEnd.SetToLastMonthDay(dateEnd.GetMonth(), dateEnd.GetYear());
        arDates.push_back(dateEnd);
        dateEnd -= span;
    };
    std::reverse(arDates.begin(), arDates.end());


    for (const auto & end_date : arDates)
    {
        double total = 0.0;
        // prepare columns for report: date, cash, checking, CC, loan, term, asset, shares, partial total, investment, grand total
        BalanceEntry totBalanceEntry;
        totBalanceEntry.date = end_date;
        wxDate begin_date = end_date;
        begin_date.SetDay(1);
        if (mode_ == YEARLY)
            begin_date.SetMonth(wxDateTime::Jan);

        for (int j = 0; j < sizeof(balancePerDay) / sizeof(*balancePerDay); j++)
            balancePerDay[j] = 0.0;

        for (const auto& account : Model_Account::instance().all())
        {
            if (Model_Account::type(account) != Model_Account::INVESTMENT)
            {
                for (const auto& ar : balanceMapVec[account.ACCOUNTID])
                {
                    if (ar.first.IsEarlierThan(begin_date))
                        continue;
                    if (ar.first.IsLaterThan(end_date))
                        break;
                    arBalance[account.ACCOUNTID] += ar.second;
                }
            }
            else
            {
                double convRate = Model_CurrencyHistory::getDayRate(account.CURRENCYID, end_date);
                arBalance[account.ACCOUNTID] = arHistory.getDailyBalanceAt(&account, end_date) * convRate;
            }
            balancePerDay[Model_Account::type(account)] += arBalance[account.ACCOUNTID];
        }

        totBalanceEntry.values.push_back(balancePerDay[Model_Account::CASH]);
        gs_data[0].values.push_back(balancePerDay[Model_Account::CASH]);
        totBalanceEntry.values.push_back(balancePerDay[Model_Account::CHECKING]);
        gs_data[1].values.push_back(balancePerDay[Model_Account::CHECKING]);
        totBalanceEntry.values.push_back(balancePerDay[Model_Account::CREDIT_CARD]);
        gs_data[2].values.push_back(balancePerDay[Model_Account::CREDIT_CARD]);
        totBalanceEntry.values.push_back(balancePerDay[Model_Account::LOAN]);
        gs_data[3].values.push_back(balancePerDay[Model_Account::LOAN]);
        totBalanceEntry.values.push_back(balancePerDay[Model_Account::TERM]);
        gs_data[4].values.push_back(balancePerDay[Model_Account::TERM]);
        totBalanceEntry.values.push_back(balancePerDay[Model_Account::ASSET]);
        gs_data[5].values.push_back(balancePerDay[Model_Account::ASSET]);
        totBalanceEntry.values.push_back(balancePerDay[Model_Account::SHARES]);
        gs_data[6].values.push_back(balancePerDay[Model_Account::SHARES]);

        for (int i = 0; i < Model_Account::MAX; i++) {
            if (i != Model_Account::INVESTMENT)
                total += balancePerDay[i];
        }

        totBalanceEntry.values.push_back(total);
        totBalanceEntry.values.push_back(balancePerDay[Model_Account::INVESTMENT]);
        gs_data[7].values.push_back(balancePerDay[Model_Account::INVESTMENT]);
        total += balancePerDay[Model_Account::INVESTMENT];
        totBalanceEntry.values.push_back(total);
        gs_data[8].values.push_back(total);
        totBalanceData.push_back(totBalanceEntry);
    }

    //Chart
    if (getChartSelection() == 0)
    {
        gs_data[0].name = _("Cash");
        gs_data[0].type = "column";
        gs_data[1].name = _("Bank Accounts");
        gs_data[1].type = "column";        
        gs_data[2].name = _("Credit Card Accounts");
        gs_data[2].type = "column";   
        gs_data[3].name = _("Loan Accounts");
        gs_data[3].type = "column";   
        gs_data[4].name = _("Term Accounts");
        gs_data[4].type = "column";   
        gs_data[5].name = _("Asset Accounts");
        gs_data[5].type = "column";   
        gs_data[6].name = _("Share Accounts");
        gs_data[6].type = "column";   
        gs_data[7].name = _("Stocks");
        gs_data[7].type = "column";   
        gs_data[8].name = _("Balance");
        gs_data[8].type = "line";  

        for (const auto& entry : totBalanceData)
        {
            const auto label = (mode_ == MONTHLY) ?
                wxString::Format("%s %i", wxGetTranslation(wxDateTime::GetEnglishMonthName(entry.date.GetMonth())), entry.date.GetYear()) :
                wxString::Format("%i", entry.date.GetYear());
            gd.labels.push_back(label);
        }
        for (const auto& gs : gs_data)
            gd.series.push_back(gs);

        gd.type = GraphData::STACKEDBARLINE; 
        hb.addChart(gd);
    }

    hb.addDivContainer("shadow");
    {
        hb.startSortTable();
        {
            hb.startThead();
            {
                hb.startTableRow();
                {
                    hb.addTableHeaderCell(_("Date"));
                    hb.addTableHeaderCell(_("Cash"), "text-right");
                    hb.addTableHeaderCell(_("Bank Accounts"), "text-right");
                    hb.addTableHeaderCell(_("Credit Card Accounts"), "text-right");
                    hb.addTableHeaderCell(_("Loan Accounts"), "text-right");
                    hb.addTableHeaderCell(_("Term Accounts"), "text-right");
                    hb.addTableHeaderCell(_("Asset Accounts"), "text-right");
                    hb.addTableHeaderCell(_("Share Accounts"), "text-right");
                    hb.addTableHeaderCell(_("Total"), "text-right");
                    hb.addTableHeaderCell(_("Stocks"), "text-right");
                    hb.addTableHeaderCell(_("Balance"), "text-right");
                }
                hb.endTableRow();
            }
            hb.endThead();

            hb.startTbody();
            {
                for (const auto& entry : totBalanceData)
                {
                    hb.startTableRow();
                        if (mode_ == MONTHLY)
                            hb.addTableCellMonth(entry.date.GetMonth(), entry.date.GetYear());
                        else
                            hb.addTableCell(wxString::Format("%d",entry.date.GetYear()));
                        for (const auto& value : entry.values)
                            hb.addMoneyCell(value);
                    hb.endTableRow();
                }
            }
            hb.endTbody();
        }
        hb.endTable();
    }
    hb.endDiv();
    
    hb.end();

    //wxLogDebug("======= mmReportSummaryByDateMontly::getHTMLText =======");
    //wxLogDebug("%s", hb.getHTMLText());

    return hb.getHTMLText();
}

mmReportSummaryByDateMontly::mmReportSummaryByDateMontly()
    : mmReportSummaryByDate(MONTHLY)
{
    setReportParameters(Reports::MonthlySummaryofAccounts);
}

mmReportSummaryByDateYearly::mmReportSummaryByDateYearly()
    : mmReportSummaryByDate(YEARLY)
{
    setReportParameters(Reports::YearlySummaryofAccounts);
}
