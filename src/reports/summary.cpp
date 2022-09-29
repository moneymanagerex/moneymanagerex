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

mmHistoryItem::mmHistoryItem()
{
    acctId = stockId = 0;
    purchasePrice = numShares = 0.0;
}

double mmHistoryData::getDailyBalanceAt(const Model_Account::Data *account, const wxDate& date)
{
    return Model_Stock::instance().getDailyBalanceAt(account, date);



}

mmReportSummaryByDate::mmReportSummaryByDate(int mode)
: mmPrintableBase(wxString::Format("Accounts Balance - %s", (mode == MONTHLY ? "Monthly" : "Yearly")))
, mode_(mode)
{

}

std::map<wxDate, double> mmReportSummaryByDate::createCheckingBalanceMap(const Model_Account::Data& account)
{
    std::map<wxDate, double> balanceMap;
    double balance = account.INITIALBAL;

    for (const auto& tran : Model_Account::transaction(account))
    {
        wxDate date = Model_Checking::TRANSDATE(tran);
        balance += Model_Checking::balance(tran, account.ACCOUNTID);
        balanceMap[date] = balance;
    }
    return balanceMap;
}

static bool sortFunction(const std::pair<wxDate, double> x, std::pair<wxDate, double> y)
{
    return x.first >= y.first;
}

double mmReportSummaryByDate::getCheckingDailyBalanceAt(const Model_Account::Data* account, const wxDate& date)
{
    std::map<wxDate, double> balanceMap = accountsBalanceMap[account->ACCOUNTID];

    auto const& i = std::upper_bound(balanceMap.rbegin(), balanceMap.rend(), std::pair<wxDate, double>(date, 0), sortFunction);
    if (i != balanceMap.rend())
    {
        return (*i).second;
    }
    return account->INITIALBAL;
}

double mmReportSummaryByDate::getInvestingDailyBalanceAt(const Model_Account::Data* account, const wxDate& date)
{
    return arHistory.getDailyBalanceAt(account, date);
}

double mmReportSummaryByDate::getDailyBalanceAt(const Model_Account::Data* account, const wxDate& date)
{
    if (date.FormatISODate() < account->INITIALDATE)
        return 0.0;

    if (Model_Account::type(account) == Model_Account::INVESTMENT)
    {
        return getInvestingDailyBalanceAt(account, date);
    }
    else
    {
        return getCheckingDailyBalanceAt(account, date);
    }
}

double mmReportSummaryByDate::getDayRate(int currencyid, const wxDate& date)
{
    wxString key = wxString::Format("%d_%s", currencyid, date.FormatDate());

    auto i = currencyDateRateCache.find(key);
    if (i != currencyDateRateCache.end())
    {
        return (*i).second;
    }

    double value = Model_CurrencyHistory::getDayRate(currencyid, date);
    currencyDateRateCache[key] = value;

    return value;
}

wxString mmReportSummaryByDate::getHTMLText()
{
    double balancePerDay[Model_Account::MAX];
    mmHTMLBuilder   hb;
    wxDate dateStart = wxDate::Today();
    wxDate dateEnd = wxDate::Today();
    wxDateSpan      span;
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

    currencyDateRateCache.clear();
    arHistory.clear();

    dateStart = wxDate::Today();
    // Calculate the report data
    for (const auto& account: Model_Account::instance().all())
    {
        const wxDate accountOpeningDate = Model_Account::get_date_by_string(account.INITIALDATE);
        if (accountOpeningDate.IsEarlierThan(dateStart))
            dateStart = accountOpeningDate;
        if (Model_Account::type(account) == Model_Account::INVESTMENT)
        {
            Model_Stock::Data_Set stocks = Model_Stock::instance().find(Model_Stock::HELDAT(account.id()));
            for (const auto& stock : stocks)
            {
                mmHistoryItem histItem;
                histItem.acctId = account.id();
                histItem.stockId = stock.STOCKID;
                histItem.purchasePrice = stock.PURCHASEPRICE;
                histItem.purchaseDate = Model_Stock::PURCHASEDATE(stock);
                histItem.purchaseDateStr = stock.PURCHASEDATE;
                histItem.numShares = stock.NUMSHARES;
                histItem.stockHist = Model_StockHistory::instance().find(Model_StockHistory::SYMBOL(stock.SYMBOL));
                std::stable_sort(histItem.stockHist.begin(), histItem.stockHist.end(), SorterByDATE());
                std::reverse(histItem.stockHist.begin(), histItem.stockHist.end());
                arHistory.push_back(histItem);
            }
        }
        else
        {
            accountsBalanceMap[account.ACCOUNTID] = createCheckingBalanceMap(account);
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
            balancePerDay[Model_Account::type(account)] += getDailyBalanceAt(&account, end_date) * getDayRate(account.CURRENCYID, end_date);
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
