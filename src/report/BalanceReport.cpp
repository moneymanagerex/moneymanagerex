/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2013 - 2022 Nikolay Akimov
 Copyright (C) 2017 James Higley
 Copyright (C) 2021 - 2022 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025, 2026 Klaus Wich

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
#include "base/_constants.h"
#include "htmlbuilder.h"

#include "model/_all.h"
#include "BalanceReport.h"
#include "util/mmNavigatorList.h"

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
    m_selection_map["name"] = _t("View");
    m_selection_map["values"] = _t("Account Types") + "," + _t("Accounts");
    m_selection_map["default"] = _t("Account Types");
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
    const auto account_it = m_account_balance_mDate_mId.find(account_n->m_id);
    if (account_it == m_account_balance_mDate_mId.end())
        return account_n->m_open_balance;

    const auto& date_balance_m = account_it->second;
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
    if (AccountModel::type_id(*account_n) == mmNavigatorItem::TYPE_ID_INVESTMENT) {
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
        currency_id,
        date
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
    const bool view_accounts = (getGenericSelection() == 1);
    int acc_size = mmNavigatorList::instance().getNumberOfAccountTypes();
    const AccountModel::DataA account_a = AccountModel::instance().find_data_a(
        TableClause::ORDERBY(AccountCol::s_primary_name)
    );
    const AssetModel::DataA asset_a = AssetModel::instance().find_data_a(
        TableClause::ORDERBY(AssetCol::s_primary_name)
    );

    std::vector<mmDate> end_date_a;

    hb.init();
    const auto name = wxString::Format(
        _t("Accounts Balance - %s"),
        m_period_id == PERIOD_ID::MONTH ? _t("Monthly Report") : _t("Yearly Report")
    );
    hb.addReportHeader(name,
        m_date_range ? m_date_range->startDay() : 0,
        m_date_range ? m_date_range->isFutureIgnored() : false
    );
    if (m_date_range)
        hb.displayDateHeading(m_date_range);

    m_currencyDateRateCache.clear();
    m_stock_xa.clear();

    mmDate selected_start_date = m_date_range
        ? mmDate(m_date_range->start_date())
        : mmDate::today();
    mmDate selected_end_date = m_date_range
        ? mmDate(m_date_range->end_date())
        : mmDate::today();

    if (selected_end_date > mmDate::today())
        selected_end_date = mmDate::today();

    mmDate start_date = selected_start_date;
    bool has_included_accounts = false;
    mmDate earliest_open_date = selected_end_date;
    std::vector<wxString> series_name_a;

    // Calculate the report date
    for (const auto& account_d : account_a) {
        if (m_account_a && wxNOT_FOUND == m_account_a->Index(account_d.m_name))
            continue;

        if (view_accounts)
            series_name_a.push_back(account_d.m_name);

        if (!has_included_accounts || account_d.m_open_date < earliest_open_date) {
            earliest_open_date = account_d.m_open_date;
            has_included_accounts = true;
        }

        m_account_balance_mDate_mId[account_d.m_id] = loadAccountBalance_mDate(account_d);
        if (AccountModel::type_id(account_d) != mmNavigatorItem::TYPE_ID_INVESTMENT)
            continue;
        for (const auto& stock_d : StockModel::instance().find_data_a(
            StockCol::WHERE_HELDAT(OP_EQ, account_d.m_id)
        )) {
            StockDataExt stock_dx = StockDataExt(stock_d);
            stock_dx.m_hist_data_a = StockHistoryModel::instance().find_data_a(
                StockHistoryCol::WHERE_SYMBOL(OP_EQ, stock_d.m_symbol),
                TableClause::ORDERBY(StockHistoryCol::NAME_DATE, true)
            );
            m_stock_xa.push_back(stock_dx);
        }
    }

    const bool include_assets = mmNavigatorList::instance().isAssetAccountActive();
    const bool include_asset_series = view_accounts && !asset_a.empty() && include_assets;
    if (include_asset_series)
        series_name_a.push_back(_t("Assets"));

    if (!view_accounts) {
        for (int i = 0; i < acc_size; ++i)
            series_name_a.push_back(mmNavigatorList::instance().getAccountTypeName(i));
    }

    const int series_count = static_cast<int>(series_name_a.size());
    // +1 as we add total balance series to the end.
    std::vector<GraphSeries> gs_data(series_count + 1);
    for (int i = 0; i < series_count; ++i) {
        gs_data[i].name = series_name_a[i];
        gs_data[i].type = "column";
    }
    gs_data[series_count].name = _t("Balance");
    gs_data[series_count].type = "line";

    // Skip leading periods where selected accounts cannot have any balance yet.
    if (has_included_accounts && start_date < earliest_open_date)
        start_date = earliest_open_date;

    wxDateTime end_datetime = selected_end_date.dateTime();
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

        std::vector<double> balance_a(series_count);
        std::fill(balance_a.begin(), balance_a.end(), 0.0);
        int idx = 0;
        int type_idx;
        for (const auto& account_d : account_a) {
            if (m_account_a && wxNOT_FOUND == m_account_a->Index(account_d.m_name))
                continue;

            if (view_accounts) {
                double rate = getCurrencyDateRate(account_d.m_currency_id, end_date);
                std::pair<double, double> dailybal = getBalance(&account_d, end_date);
                balance_a[idx] = dailybal.first * rate;
                if (AccountModel::type_id(account_d) == mmNavigatorItem::TYPE_ID_INVESTMENT) {
                    balance_a[idx] += dailybal.second * rate;
                }
                idx++;
            }
            else {
                type_idx = mmNavigatorList::instance().getAccountTypeIdx(account_d.m_type_);
                if (type_idx == -1) {
                    type_idx = mmNavigatorList::instance().getAccountTypeIdx(mmNavigatorItem::TYPE_ID_CHECKING);
                }
                if (type_idx > -1) {
                    double rate = getCurrencyDateRate(account_d.m_currency_id, end_date);
                    std::pair<double, double> dailybal = getBalance(&account_d, end_date);
                    balance_a[type_idx] += dailybal.first * rate;
                    if (AccountModel::type_id(account_d) == mmNavigatorItem::TYPE_ID_INVESTMENT) {
                        balance_a[type_idx] += dailybal.second * rate;
                    }
                }
            }
        }

        if (view_accounts) {
            if (include_asset_series) {
                double asset_balance = 0.0;
                for (const auto& asset_d : asset_a) {
                    double rate = getCurrencyDateRate(asset_d.m_currency_id_n, end_date);
                    asset_balance += AssetModel::instance().get_data_value_date(
                        asset_d, end_date
                    ).second * rate;
                }
                balance_a[idx] = asset_balance;
            }
        }
        else {
            if (include_assets) {
                type_idx = mmNavigatorList::instance().getAccountTypeIdx(mmNavigatorItem::TYPE_ID_ASSET);
                if (type_idx > -1) {
                    for (const auto& asset_d : asset_a) {
                        double rate = getCurrencyDateRate(asset_d.m_currency_id_n, end_date);
                        balance_a[type_idx] += AssetModel::instance().get_data_value_date(
                            asset_d, end_date
                        ).second * rate;
                    }
                }
            }
        }

        int k = -1;
        for (int i = 0; i < series_count; ++i) {
            date_balanceA.balance_a.push_back(balance_a[i]);
            gs_data[++k].values.push_back(balance_a[i]);
            total += balance_a[i];
        }
        date_balanceA.balance_a.push_back(total);
        date_balanceA_a.push_back(date_balanceA);
        gs_data[++k].values.push_back(total);
    }

    std::vector<bool> is_visible_a;
    for (int i = 0; i < series_count; i++) {
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
        for (int i = 0; i <  (series_count + 1); ++i) {
            if (is_visible_a[i]) {
                gd.series.push_back(gs_data[i]);
            }
        }
        // create Labels:
        for (unsigned int i = 0; i < date_balanceA_a.size(); ++i) {
            wxDateTime dateTime = date_balanceA_a[i].date.dateTime();
            gd.labels.push_back(m_period_id == PERIOD_ID::MONTH
                ? wxString::Format("%s %i",
                    wxGetTranslation(wxDateTime::GetEnglishMonthName(
                        dateTime.GetMonth()
                    )),
                    dateTime.GetYear()
                )
                : wxString::Format("%i", dateTime.GetYear())
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
                    for (int i = 0; i < series_count + 1; i++) {
                        if (is_visible_a[i])
                            hb.addTableHeaderCell(gs_data[i].name, "text-right");
                    }
                }
                hb.endTableRow();
            }
            hb.endThead();

            hb.startTbody();
            {
                for (auto& date_balanceA : date_balanceA_a) {
                    wxDateTime dateTime = date_balanceA.date.dateTime();
                    hb.startTableRow();
                    if (m_period_id == PERIOD_ID::MONTH)
                        hb.addTableCellMonth(dateTime.GetMonth(), dateTime.GetYear());
                    else
                        hb.addTableCell(wxString::Format("%d", dateTime.GetYear()));
                    for (int i = 0; i < series_count + 1; i++) {
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
    hb.displayFooter(getAccountNames());

    hb.end();

    return hb.getHTMLText();
}
