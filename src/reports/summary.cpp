/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2013 - 2022 Nikolay Akimov
 Copyright (C) 2017 James Higley
 Copyright (C) 2021 - 2022 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025 Klaus Wich

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

mmReportSummaryByDate::mmReportSummaryByDate(int mode)
: mmPrintableBase(wxString::Format("Accounts Balance - %s", (mode == MONTHLY ? "Monthly" : "Yearly")))
, mode_(mode)
{

}

std::map<wxDate, double> mmReportSummaryByDate::createCheckingBalanceMap(const Model_Account::Data& account)
{
    std::map<wxDate, double> balanceMap;
    double balance = account.INITIALBAL;

    for (const auto& tran : Model_Account::transactionsByDateTimeId(account))
    {
        wxDate date = Model_Checking::TRANSDATE(tran);
        balance += Model_Checking::account_flow(tran, account.ACCOUNTID);
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

std::pair<double, double> mmReportSummaryByDate::getDailyBalanceAt(const Model_Account::Data* account, const wxDate& date)
{
    std::pair<double /*cash bal*/, double /*market bal*/> bal = { 0.0, 0.0 };
    if (date.FormatISODate() >= account->INITIALDATE) {
        bal.first = getCheckingDailyBalanceAt(account, date);
        if (Model_Account::type_id(account) == NavigatorTypes::TYPE_ID_INVESTMENT) {
            bal.second = Model_Stock::instance().getDailyBalanceAt(account, date);
        }
    }
    return bal;
}

double mmReportSummaryByDate::getDayRate(int64 currencyid, const wxDate& date)
{
    wxString key = wxString::Format("%lld_%s", currencyid, date.FormatDate());

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
    mmHTMLBuilder hb;
    wxDate dateStart = wxDate::Today();
    wxDate dateEnd = wxDate::Today();
    wxDateSpan span;
    struct BalanceEntry
    {
        wxDate date;
        std::vector<double> values;
    };
    std::vector<BalanceEntry> totBalanceData;

    GraphData gd;
    int acc_size = NavigatorTypes::instance().getNumberOfAccountTypes();
    std::vector<GraphSeries> gs_data(acc_size + 1);    // +1 as we add balance to the end);

    std::vector<wxDate> arDates;

    hb.init();
    const auto name = wxString::Format(_t("Accounts Balance - %s"), mode_ == MONTHLY ? _t("Monthly Report") : _t("Yearly Report"));
    hb.addReportHeader(name);

    currencyDateRateCache.clear();
    arHistory.clear();

    dateStart = wxDate::Today();
    // Calculate the report date
    for (const auto& account: Model_Account::instance().all())
    {
        const wxDate accountOpeningDate = Model_Account::get_date_by_string(account.INITIALDATE);
        if (accountOpeningDate.IsEarlierThan(dateStart))
            dateStart = accountOpeningDate;
        accountsBalanceMap[account.ACCOUNTID] = createCheckingBalanceMap(account);
        if (Model_Account::type_id(account) == NavigatorTypes::TYPE_ID_INVESTMENT)
        {
            Model_Stock::Data_Set stocks = Model_Stock::instance().find(Model_Stock::HELDAT(account.id()));
            for (const auto& stock : stocks) {
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
    }

    if (mode_ == MONTHLY) {
        dateEnd.SetToLastMonthDay(dateEnd.GetMonth(), dateEnd.GetYear());
        span = wxDateSpan::Months(1);
    }
    else if (mode_ == YEARLY) {
        dateEnd.Set(31, wxDateTime::Dec, dateEnd.GetYear());
        span = wxDateSpan::Years(1);
    }
     else {
        wxFAIL_MSG("unknown report mode");
    }

    //  prepare the dates array
    while (dateStart <= dateEnd) {
        if (mode_ == MONTHLY) {
            dateEnd.SetToLastMonthDay(dateEnd.GetMonth(), dateEnd.GetYear());
        }
        arDates.push_back(dateEnd);
        dateEnd -= span;
    }
    std::reverse(arDates.begin(), arDates.end());

    wxString m_temp_view = Model_Setting::instance().getViewAccounts();

    for (const auto & end_date : arDates) {
        double total = 0.0;
        BalanceEntry totBalanceEntry;
        totBalanceEntry.date = end_date;
        wxDate begin_date = end_date;
        begin_date.SetDay(1);

        if (mode_ == YEARLY)
            begin_date.SetMonth(wxDateTime::Jan);

        int idx;
        std::vector<double> balancePerDay(acc_size +1);
        std::fill(balancePerDay.begin(), balancePerDay.end(), 0.0);
        for (const auto& account : Model_Account::instance().all()) {
            if ((m_temp_view == VIEW_ACCOUNTS_OPEN_STR && Model_Account::status_id(account) != Model_Account::STATUS_ID_OPEN) ||
                (m_temp_view == VIEW_ACCOUNTS_CLOSED_STR && Model_Account::status_id(account) == Model_Account::STATUS_ID_OPEN) ||
                (m_temp_view == VIEW_ACCOUNTS_FAVORITES_STR && !Model_Account::FAVORITEACCT(account))) {
                continue;
            }

            idx = NavigatorTypes::instance().getAccountTypeIdx(account.ACCOUNTTYPE);
            if (idx == -1) {
                idx = NavigatorTypes::instance().getAccountTypeIdx(NavigatorTypes::TYPE_ID_CHECKING);
            }
            if (idx > -1) {
                std::pair<double, double> dailybal = getDailyBalanceAt(&account, end_date);
                balancePerDay[idx] += dailybal.first * getDayRate(account.CURRENCYID, end_date);
                if (Model_Account::type_id(account) == NavigatorTypes::TYPE_ID_INVESTMENT) {
                    balancePerDay[idx] += dailybal.second * getDayRate(account.CURRENCYID, end_date);
                }
            }
        }

        idx = NavigatorTypes::instance().getAccountTypeIdx(NavigatorTypes::TYPE_ID_ASSET);
        if (idx > -1) {
            for (const auto& asset : Model_Asset::instance().all()) {
                balancePerDay[idx] += Model_Asset::instance().valueAtDate(&asset, end_date).second * getDayRate(asset.CURRENCYID, end_date);
            }
        }

        int k = -1;
        for (int i = 0; i < acc_size; ++i) {
            totBalanceEntry.values.push_back(balancePerDay[i]);
            gs_data[++k].values.push_back(balancePerDay[i]);
            total += balancePerDay[i];
            gs_data[k].name = NavigatorTypes::instance().getAccountTypeName(i);
            gs_data[k].type = "column";
        }
        totBalanceEntry.values.push_back(total);
        totBalanceData.push_back(totBalanceEntry);
        gs_data[++k].values.push_back(total);
        gs_data[k].name = _t("Balance");
        gs_data[k].type = "line";
    }

    std::vector<bool> hasAccounts;
    for (int i = 0; i <  acc_size; i++) {
        bool av;
        for (double value :  gs_data[i].values) {
            av = false;
            if (value != 0) {
                av = true;
                break;
            }
        }
        hasAccounts.push_back(av);
    }
    hasAccounts.push_back(true);  // always include Balance

    //Chart
    if (getChartSelection() == 0) {
        for (int i = 0; i <  (acc_size + 1); i++) {
            if (hasAccounts[i]) {
                gd.series.push_back(gs_data[i]);
            }
        }
        // create Labels:
        for (unsigned int i = 0; i < totBalanceData.size(); i++) {
            gd.labels.push_back(mode_ == MONTHLY ?
                                wxString::Format("%s %i", wxGetTranslation(wxDateTime::GetEnglishMonthName(totBalanceData[i].date.GetMonth())), totBalanceData[i].date.GetYear()) :
                                wxString::Format("%i", totBalanceData[i].date.GetYear()));
        }

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
                    hb.addTableHeaderCell(_t("Date"));
                    for (int i = 0; i <  acc_size + 1; i++) {
                        if (!hasAccounts[i]) {
                            continue;
                        }
                        hb.addTableHeaderCell(gs_data[i].name, "text-right");
                    }
                }
                hb.endTableRow();
            }
            hb.endThead();

            hb.startTbody();
            {
                for (const auto& entry : totBalanceData) {
                    hb.startTableRow();
                    if (mode_ == MONTHLY)
                        hb.addTableCellMonth(entry.date.GetMonth(), entry.date.GetYear());
                    else
                        hb.addTableCell(wxString::Format("%d",entry.date.GetYear()));
                    for (int i = 0; i <  acc_size + 1; i++) {
                        if (hasAccounts[i]) {
                            hb.addMoneyCell(entry.values[i]);
                        }
                    }
                    hb.endTableRow();
                }
            }
            hb.endTbody();
        }
        hb.endTable();
    }
    hb.endDiv();

    hb.end();

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
