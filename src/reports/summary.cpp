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
#include "model/_all.h"
#include <algorithm>

mmHistoryItem::mmHistoryItem()
{
    acctId = stockId = 0;
    purchasePrice = numShares = 0.0;
}

mmReportSummaryByDate::mmReportSummaryByDate(mmReportSummaryByDate::PERIOD_ID period_id) :
    ReportBase(wxString::Format(
        "Accounts Balance - %s", (period_id == PERIOD_ID::MONTH ? "Monthly" : "Yearly")
    )),
    m_period_id(period_id)
{
    setReportParameters((period_id == PERIOD_ID::MONTH)
        ? REPORT_ID::MonthlySummaryofAccounts
        : REPORT_ID::YearlySummaryofAccounts
    );
}

std::map<wxDate, double> mmReportSummaryByDate::loadCheckingDateBalance(const AccountModel::Data& account)
{
    std::map<wxDate, double> date_balance;
    double balance = account.INITIALBAL;

    for (const auto& tran : AccountModel::transactionsByDateTimeId(account)) {
        wxDate date = TransactionModel::getTransDateTime(tran);
        balance += TransactionModel::account_flow(tran, account.ACCOUNTID);
        date_balance[date] = balance;
    }
    return date_balance;
}

static bool sortFunction(const std::pair<wxDate, double> x, std::pair<wxDate, double> y)
{
    return x.first >= y.first;
}

double mmReportSummaryByDate::getCheckingBalance(const AccountModel::Data* account, const wxDate& date)
{
    std::map<wxDate, double> date_balance = m_account_date_balance[account->ACCOUNTID];

    auto const& i = std::upper_bound(date_balance.rbegin(), date_balance.rend(), std::pair<wxDate, double>(date, 0), sortFunction);
    if (i != date_balance.rend())
        return (*i).second;
    return account->INITIALBAL;
}

std::pair<double, double> mmReportSummaryByDate::getBalance(const AccountModel::Data* account, const wxDate& date)
{
    std::pair<double /*cash bal*/, double /*market bal*/> bal = { 0.0, 0.0 };
    if (date.FormatISODate() >= account->INITIALDATE) {
        bal.first = getCheckingBalance(account, date);
        if (AccountModel::type_id(account) == NavigatorTypes::TYPE_ID_INVESTMENT) {
            bal.second = StockModel::instance().getDailyBalanceAt(account, date);
        }
    }
    return bal;
}

double mmReportSummaryByDate::getCurrencyDateRate(int64 currencyid, const wxDate& date)
{
    wxString key = wxString::Format("%lld_%s", currencyid, date.FormatDate());

    auto i = m_currencyDateRateCache.find(key);
    if (i != m_currencyDateRateCache.end())
        return (*i).second;

    double value = CurrencyHistoryModel::getDayRate(currencyid, date);
    m_currencyDateRateCache[key] = value;

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
    const auto name = wxString::Format(
        _t("Accounts Balance - %s"),
        m_period_id == PERIOD_ID::MONTH ? _t("Monthly Report") : _t("Yearly Report")
    );
    hb.addReportHeader(name);

    m_currencyDateRateCache.clear();
    m_stock_a.clear();

    dateStart = wxDate::Today();
    // Calculate the report date
    for (const auto& account: AccountModel::instance().all()) {
        const wxDate accountOpeningDate = parseDateTime(account.INITIALDATE);
        if (accountOpeningDate.IsEarlierThan(dateStart))
            dateStart = accountOpeningDate;
        m_account_date_balance[account.ACCOUNTID] = loadCheckingDateBalance(account);
        if (AccountModel::type_id(account) != NavigatorTypes::TYPE_ID_INVESTMENT)
            continue;
        StockModel::Data_Set stocks = StockModel::instance().find(
            StockModel::HELDAT(account.id())
        );
        for (const auto& stock : stocks) {
            mmHistoryItem histItem;
            histItem.acctId = account.id();
            histItem.stockId = stock.STOCKID;
            histItem.purchasePrice = stock.PURCHASEPRICE;
            histItem.purchaseDate = StockModel::PURCHASEDATE(stock);
            histItem.purchaseDateStr = stock.PURCHASEDATE;
            histItem.numShares = stock.NUMSHARES;
            histItem.stockHist = StockHistoryModel::instance().find(
                StockHistoryModel::SYMBOL(stock.SYMBOL)
            );
            std::stable_sort(histItem.stockHist.begin(), histItem.stockHist.end(), SorterByDATE());
            std::reverse(histItem.stockHist.begin(), histItem.stockHist.end());
            m_stock_a.push_back(histItem);
        }
    }

    if (m_period_id == PERIOD_ID::MONTH) {
        dateEnd.SetToLastMonthDay(dateEnd.GetMonth(), dateEnd.GetYear());
        span = wxDateSpan::Months(1);
    }
    else if (m_period_id == PERIOD_ID::YEAR) {
        dateEnd.Set(31, wxDateTime::Dec, dateEnd.GetYear());
        span = wxDateSpan::Years(1);
    }
     else {
        wxFAIL_MSG("unknown report mode");
    }

    //  prepare the dates array
    while (dateStart <= dateEnd) {
        if (m_period_id == PERIOD_ID::MONTH) {
            dateEnd.SetToLastMonthDay(dateEnd.GetMonth(), dateEnd.GetYear());
        }
        arDates.push_back(dateEnd);
        dateEnd -= span;
    }
    std::reverse(arDates.begin(), arDates.end());

    wxString m_temp_view = SettingModel::instance().getViewAccounts();

    for (const auto & end_date : arDates) {
        double total = 0.0;
        BalanceEntry totBalanceEntry;
        totBalanceEntry.date = end_date;
        wxDate begin_date = end_date;
        begin_date.SetDay(1);

        if (m_period_id == PERIOD_ID::YEAR)
            begin_date.SetMonth(wxDateTime::Jan);

        int idx;
        std::vector<double> balancePerDay(acc_size +1);
        std::fill(balancePerDay.begin(), balancePerDay.end(), 0.0);
        for (const auto& account : AccountModel::instance().all()) {
            if ((m_temp_view == VIEW_ACCOUNTS_OPEN_STR && AccountModel::status_id(account) != AccountModel::STATUS_ID_OPEN) ||
                (m_temp_view == VIEW_ACCOUNTS_CLOSED_STR && AccountModel::status_id(account) == AccountModel::STATUS_ID_OPEN) ||
                (m_temp_view == VIEW_ACCOUNTS_FAVORITES_STR && !AccountModel::FAVORITEACCT(account))) {
                continue;
            }

            idx = NavigatorTypes::instance().getAccountTypeIdx(account.ACCOUNTTYPE);
            if (idx == -1) {
                idx = NavigatorTypes::instance().getAccountTypeIdx(NavigatorTypes::TYPE_ID_CHECKING);
            }
            if (idx > -1) {
                std::pair<double, double> dailybal = getBalance(&account, end_date);
                balancePerDay[idx] += dailybal.first * getCurrencyDateRate(account.CURRENCYID, end_date);
                if (AccountModel::type_id(account) == NavigatorTypes::TYPE_ID_INVESTMENT) {
                    balancePerDay[idx] += dailybal.second * getCurrencyDateRate(account.CURRENCYID, end_date);
                }
            }
        }

        idx = NavigatorTypes::instance().getAccountTypeIdx(NavigatorTypes::TYPE_ID_ASSET);
        if (idx > -1) {
            for (const auto& asset : AssetModel::instance().all()) {
                balancePerDay[idx] += AssetModel::instance().valueAtDate(&asset, end_date).second * getCurrencyDateRate(asset.CURRENCYID, end_date);
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
            gd.labels.push_back(m_period_id == PERIOD_ID::MONTH ?
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
                    if (m_period_id == PERIOD_ID::MONTH)
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
