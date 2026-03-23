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

#include <algorithm>
#include "base/constants.h"
#include "htmlbuilder.h"

#include "model/_all.h"
#include "BalanceReport.h"

BalanceReport::BalanceReport(BalanceReport::PERIOD_ID period_id) :
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

std::map<mmDate, double> BalanceReport::loadAccountBalance_mDate(const AccountData& account_d)
{
    std::map<mmDate, double> date_balance_m;
    double balance = account_d.m_open_balance;

    for (const auto& trx_d : AccountModel::instance().find_id_trx_aBySN(account_d.m_id)) {
        mmDate date = trx_d.m_date();
        balance += trx_d.account_flow(account_d.m_id);
        date_balance_m[date] = balance;
    }
    return date_balance_m;
}

double BalanceReport::getCheckingBalance(const AccountData* account_n, const mmDate& date)
{
    std::map<mmDate, double> date_balance_m = m_account_balance_mDate_mId[account_n->m_id];
    auto const& it = std::upper_bound(date_balance_m.rbegin(), date_balance_m.rend(),
        std::pair<mmDate, double>(date, 0),
        [](const std::pair<mmDate, double> x, std::pair<mmDate, double> y) {
            return x.first >= y.first;
        }
    );
    if (it != date_balance_m.rend())
        return (*it).second;
    return account_n->m_open_balance;
}

std::pair<double, double> BalanceReport::getBalance(
    const AccountData* account_n,
    const mmDate& date
) {
    std::pair<double /*cash bal*/, double /*market bal*/> bal = { 0.0, 0.0 };
    if (date < account_n->m_open_date)
        return bal;
    bal.first = getCheckingBalance(account_n, date);
    if (AccountModel::type_id(*account_n) == NavigatorTypes::TYPE_ID_INVESTMENT) {
        bal.second = StockModel::instance().calculate_account_balance(*account_n, date);
    }
    return bal;
}

double BalanceReport::getCurrencyDateRate(int64 currency_id, const mmDate& date)
{
    wxString key = wxString::Format("%lld_%s", currency_id, date.isoDate());

    auto i = m_currencyDateRateCache.find(key);
    if (i != m_currencyDateRateCache.end())
        return (*i).second;

    double rate = CurrencyHistoryModel::instance().get_id_date_rate(
        currency_id, date
    );
    m_currencyDateRateCache[key] = rate;

    return rate;
}

wxString BalanceReport::getHTMLText()
{
    mmHTMLBuilder hb;
    struct BalanceEntry
    {
        mmDate date;
        std::vector<double> balance_a;

        BalanceEntry() : date(mmDate::today()) {}
    };
    std::vector<BalanceEntry> date_balanceA_a;

    GraphData gd;
    int acc_size = NavigatorTypes::instance().getNumberOfAccountTypes();
    // +1 as we add balance to the end;
    std::vector<GraphSeries> gs_data(acc_size + 1);

    std::vector<mmDate> end_date_a;

    hb.init();
    const auto name = wxString::Format(
        _t("Accounts Balance - %s"),
        m_period_id == PERIOD_ID::MONTH ? _t("Monthly Report") : _t("Yearly Report")
    );
    hb.addReportHeader(name);

    m_currencyDateRateCache.clear();
    m_stock_xa.clear();

    mmDate start_date = mmDate::today();
    // Calculate the report date
    for (const auto& account_d : AccountModel::instance().find_all()) {
        if (account_d.m_open_date < start_date)
            start_date = account_d.m_open_date;
        m_account_balance_mDate_mId[account_d.m_id] = loadAccountBalance_mDate(account_d);
        if (AccountModel::type_id(account_d) != NavigatorTypes::TYPE_ID_INVESTMENT)
            continue;
        for (const auto& stock_d : StockModel::instance().find(
            StockCol::HELDAT(account_d.m_id)
        )) {
            StockDataExt stock_dx = StockDataExt(stock_d);
            stock_dx.m_hist_data_a = StockHistoryModel::instance().find(
                StockHistoryCol::SYMBOL(stock_d.m_symbol)
            );
            std::stable_sort(stock_dx.m_hist_data_a.begin(), stock_dx.m_hist_data_a.end(),
                StockHistoryData::SorterByDATE()
            );
            std::reverse(stock_dx.m_hist_data_a.begin(), stock_dx.m_hist_data_a.end());
            m_stock_xa.push_back(stock_dx);
        }
    }

    wxDateTime end_datetime = mmDate::today().getDateTime();
    wxDateSpan span;
    if (m_period_id == PERIOD_ID::MONTH) {
        end_datetime.SetToLastMonthDay(end_datetime.GetMonth(), end_datetime.GetYear());
        span = wxDateSpan::Months(1);
    }
    else if (m_period_id == PERIOD_ID::YEAR) {
        end_datetime.Set(31, wxDateTime::Dec, end_datetime.GetYear());
        span = wxDateSpan::Years(1);
    }
    else {
        wxFAIL_MSG("unknown report mode");
    }

    // prepare the dates array
    while (start_date <= mmDate(end_datetime)) {
        end_date_a.push_back(mmDate(end_datetime));
        end_datetime -= span;
        if (m_period_id == PERIOD_ID::MONTH) {
            end_datetime.SetToLastMonthDay(end_datetime.GetMonth(), end_datetime.GetYear());
        }
    }
    std::reverse(end_date_a.begin(), end_date_a.end());

    for (const auto& end_date : end_date_a) {
        BalanceEntry date_balanceA;
        date_balanceA.date = end_date;
        double total = 0.0;

        std::vector<double> balance_a(acc_size +1);
        std::fill(balance_a.begin(), balance_a.end(), 0.0);
        int idx;
        for (const auto& account_d : AccountModel::instance().find_all()) {
            idx = NavigatorTypes::instance().getAccountTypeIdx(account_d.m_type_);
            if (idx == -1) {
                idx = NavigatorTypes::instance().getAccountTypeIdx(NavigatorTypes::TYPE_ID_CHECKING);
            }
            if (idx > -1) {
                double rate = getCurrencyDateRate(account_d.m_currency_id, end_date);
                std::pair<double, double> dailybal = getBalance(&account_d, end_date);
                balance_a[idx] += dailybal.first * rate;
                if (AccountModel::type_id(account_d) == NavigatorTypes::TYPE_ID_INVESTMENT) {
                    balance_a[idx] += dailybal.second * rate;
                }
            }
        }

        idx = NavigatorTypes::instance().getAccountTypeIdx(NavigatorTypes::TYPE_ID_ASSET);
        if (idx > -1) {
            for (const auto& asset_d : AssetModel::instance().find_all()) {
                double rate = getCurrencyDateRate(asset_d.m_currency_id_n, end_date);
                balance_a[idx] += AssetModel::instance().get_data_value_date(
                    asset_d, end_date
                ).second * rate;
            }
        }

        int k = -1;
        for (int i = 0; i < acc_size; ++i) {
            date_balanceA.balance_a.push_back(balance_a[i]);
            gs_data[++k].values.push_back(balance_a[i]);
            total += balance_a[i];
            gs_data[k].name = NavigatorTypes::instance().getAccountTypeName(i);
            gs_data[k].type = "column";
        }
        date_balanceA.balance_a.push_back(total);
        date_balanceA_a.push_back(date_balanceA);
        gs_data[++k].values.push_back(total);
        gs_data[k].name = _t("Balance");
        gs_data[k].type = "line";
    }

    std::vector<bool> is_visible_a;
    for (int i = 0; i <  acc_size; i++) {
        bool av;
        for (double value : gs_data[i].values) {
            av = false;
            if (value != 0) {
                av = true;
                break;
            }
        }
        is_visible_a.push_back(av);
    }
    // always show total Balance
    is_visible_a.push_back(true);

    // Chart
    if (getChartSelection() == 0) {
        for (int i = 0; i <  (acc_size + 1); ++i) {
            if (is_visible_a[i]) {
                gd.series.push_back(gs_data[i]);
            }
        }
        // create Labels:
        for (unsigned int i = 0; i < date_balanceA_a.size(); ++i) {
            gd.labels.push_back(m_period_id == PERIOD_ID::MONTH
                ? wxString::Format("%s %i",
                    wxGetTranslation(wxDateTime::GetEnglishMonthName(
                        date_balanceA_a[i].date.getDateTime().GetMonth())
                    ),
                    date_balanceA_a[i].date.getDateTime().GetYear()
                )
                : wxString::Format("%i", date_balanceA_a[i].date.getDateTime().GetYear())
            );
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
                    for (int i = 0; i < acc_size + 1; i++) {
                        if (is_visible_a[i])
                            hb.addTableHeaderCell(gs_data[i].name, "text-right");
                    }
                }
                hb.endTableRow();
            }
            hb.endThead();

            hb.startTbody();
            {
                for (const auto& date_balanceA : date_balanceA_a) {
                    wxDateTime dt = date_balanceA.date.getDateTime();
                    hb.startTableRow();
                    if (m_period_id == PERIOD_ID::MONTH)
                        hb.addTableCellMonth(dt.GetMonth(), dt.GetYear());
                    else
                        hb.addTableCell(wxString::Format("%d", dt.GetYear()));
                    for (int i = 0; i < acc_size + 1; i++) {
                        if (is_visible_a[i])
                            hb.addMoneyCell(date_balanceA.balance_a[i]);
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
