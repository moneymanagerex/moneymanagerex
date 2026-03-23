/*******************************************************
Copyright (C) 2014 - 2021 Nikolay Akimov
Copyright (C) 2021-2023 Mark Whalley (mark@ipx.co.uk)
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
#include <cmath>
#include <html_template.h>

#include "base/constants.h"
#include "util/mmDate.h"

#include "model/AssetModel.h"
#include "model/CategoryModel.h"
#include "model/CurrencyHistoryModel.h"
#include "model/CurrencyModel.h"
#include "model/PayeeModel.h"
#include "model/PrefModel.h"
#include "model/SettingModel.h"
#include "model/StockHistoryModel.h"
#include "model/StockModel.h"

#include "DashboardWidget.h"
#include "SchedPanel.h"

#include "pref/DashboardPref.h"
#include "report/_ReportBase.h"
#include "uicontrols/navigatortypes.h"

static const wxString TOP_CATEGS = R"(
<table class = 'table'>
  <tr class='active'>
    <th>%s</th>
    <th nowrap class='text-right sorttable_nosort'>
      <a id='%s_label' onclick='toggleTable("%s"); ' href='#%s' oncontextmenu='return false;'>[-]</a>
    </th>
  </tr>
  <tr>
    <td style='padding: 0px; padding-left: 0px; padding-right: 0px; width: 100%%;' colspan='2'>
    <table class = 'sortable table' id='%s'>
    <thead>
      <tr><th>%s</th><th class='text-right'>%s</th></tr>
    </thead>
   <tbody>
%s
   </tbody>
</table>
</td></tr>
</table>
)";


htmlWidgetStocks::htmlWidgetStocks() :
    m_title(_t("Stocks")),
    m_total(0.0)
{
}

htmlWidgetStocks::~htmlWidgetStocks()
{
}

const wxString htmlWidgetStocks::getHTMLText()
{
    double grand_gain_lost    = 0;
    double grand_market_value = 0;  // Track the grand total of market values
    double grand_cash_balance = 0;  // Track the grand total of cash balances

    wxString output = "";
    AccountModel::DataA account_a = AccountModel::instance().find(
        AccountCol::ACCOUNTTYPE(OP_EQ, NavigatorTypes::instance().getInvestmentAccountStr())
    );
    if (account_a.empty())
        return output;

    std::stable_sort(account_a.begin(), account_a.end(), AccountData::SorterByName());

    output = R"(<div class="shadow">)";
    output += "<table class ='sortable table'><col style='width: 50%'><col style='width: 12.5%'><col style='width: 12.5%'><col style='width: 12.5%'><col style='width: 12.5%'><thead><tr class='active'><th>\n";
    output += _t("Stocks") + "</th><th class = 'text-right'>" + _t("Gain/Loss") + "</th>\n";
    output += "<th class='text-right'>" + _t("Market Value") + "</th>\n";
    output += "<th class='text-right'>" + _t("Cash Balance") + "</th>\n";
    output += "<th class='text-right'>" + _t("Total") + "</th>\n";
    output += wxString::Format("<th nowrap class='text-right sorttable_nosort'><a id='%s_label' onclick='toggleTable(\"%s\");' href='#%s' oncontextmenu='return false;'>[-]</a></th>\n"
        , "INVEST", "INVEST", "INVEST");
    output += "</tr></thead><tbody id='INVEST'>\n";
    wxString body = "";

    bool btoday = PrefModel::instance().getIgnoreFutureTransactionsHomePage();
    double cash_bal;

    for (const auto& account_d : account_a) {
        if (!account_d.is_open())
            continue;

        double conv_rate = CurrencyHistoryModel::instance().get_id_date_rate(
            account_d.m_currency_id
        );
        auto inv_bal = AccountModel::instance().get_data_investment_balance(account_d);
        if (btoday) {
            cash_bal = AccountModel::instance().get_data_balance_to_date(
                account_d, mmDate::today()
            );
        }
        else {
            cash_bal = AccountModel::instance().get_data_balance(account_d);
        }

        grand_gain_lost    += (inv_bal.first - inv_bal.second) * conv_rate;
        grand_market_value += inv_bal.first * conv_rate;
        grand_cash_balance += cash_bal * conv_rate;
        m_total            += (inv_bal.first + cash_bal) * conv_rate;

        body += "<tr>";
        body += wxString::Format("<td sorttable_customkey='*%s*'><a href='stock:%lld' oncontextmenu='return false;' target='_blank'>%s</a>%s</td>\n",
            account_d.m_name, account_d.m_id, account_d.m_name,
            account_d.m_website.empty() ? "" : wxString::Format("&nbsp;&nbsp;&nbsp;&nbsp;(<a href='%s' oncontextmenu='return false;' target='_blank'>WWW</a>)",
                account_d.m_website
            )
        );
        body += wxString::Format("<td class='money' sorttable_customkey='%f'>%s</td>\n",
            inv_bal.first - inv_bal.second,
            AccountModel::instance().value_number_currency(
                account_d, inv_bal.first - inv_bal.second
            )
        );
        body += wxString::Format("<td class='money' sorttable_customkey='%f'>%s</td>\n",
            inv_bal.first,
            AccountModel::instance().value_number_currency(account_d, inv_bal.first)
        );
        body += wxString::Format("<td class='money' sorttable_customkey='%f'>%s</td>\n",
            cash_bal,
            AccountModel::instance().value_number_currency(account_d, cash_bal)
        );
        body += wxString::Format("<td colspan='2' class='money' sorttable_customkey='%f'>%s</td>",
            inv_bal.first + cash_bal,
            AccountModel::instance().value_number_currency(account_d, inv_bal.first + cash_bal)
        );
        body += "</tr>";
    }

    if (!body.empty()) {
        output += body;
        output += "</tbody><tfoot><tr class = 'total'><td>" + _t("Total:") + "</td>";
        output += wxString::Format("<td class='money'>%s</td>",
            CurrencyModel::instance().toCurrency(grand_gain_lost)
        );
        output += wxString::Format("<td class='money'>%s</td>",
            CurrencyModel::instance().toCurrency(grand_market_value)
        );
        output += wxString::Format("<td class='money'>%s</td>",
            CurrencyModel::instance().toCurrency(grand_cash_balance)
        );
        output += wxString::Format("<td colspan='2' class='money'>%s</td></tr></tfoot></table>\n",
            CurrencyModel::instance().toCurrency(m_total)
        );
        output += "</div>";
    }
    return output;
}

htmlWidgetTop7Categories::htmlWidgetTop7Categories()
{
    m_date_range = new mmLast30Days();
    m_title = wxString::Format(_t("Top Withdrawals: %s"), m_date_range->local_title());
}

htmlWidgetTop7Categories::~htmlWidgetTop7Categories()
{
    if (m_date_range)
        delete m_date_range;
}

const wxString htmlWidgetTop7Categories::getHTMLText()
{

    std::vector<std::pair<wxString, double> > topCategoryStats;
    getTopCategoryStats(topCategoryStats, m_date_range);
    wxString output, data;

    if (!topCategoryStats.empty())
    {
        output = R"(<div class="shadow">)";
        for (const auto& i : topCategoryStats)
        {
            data += "<tr>";
            data += wxString::Format("<td>%s</td>", (i.first.IsEmpty() ? wxString::FromUTF8Unchecked("…") : i.first));
            data += wxString::Format("<td class='money' sorttable_customkey='%f'>%s</td>\n"
                , i.second
                , CurrencyModel::instance().toCurrency(i.second));
            data += "</tr>\n";
        }
        const wxString idStr = "TOP_CATEGORIES";
        output += wxString::Format(TOP_CATEGS, m_title, idStr, idStr, idStr, idStr, _t("Category"), _t("Summary"), data);
        output += "</div>";
    }

    return output;
}

void htmlWidgetTop7Categories::getTopCategoryStats(
    std::vector<std::pair<wxString, double> > &categoryStats,
    const mmDateRange* date_range
) const
{
    // Temporary map
    std::map<int64 /*category_id*/, double> stat;

    const auto trxId_tpA_m = TrxSplitModel::instance().find_all_mTrxId();
    const auto& trx_a = TrxModel::instance().find(
        TrxModel::DATE(OP_GE, mmDate(date_range->start_date())),
        TrxModel::DATE(OP_LE, mmDate(date_range->end_date())),
        TrxModel::TYPE(OP_NE, TrxType(TrxType::e_transfer)),
        TrxModel::IS_VOID(false)
    );

    for (const auto& trx_d : trx_a) {
        // Do not include asset or stock transfers or deleted transactions in income expense calculations.
        if (TrxModel::is_foreignAsTransfer(trx_d) || trx_d.is_deleted())
            continue;

        bool withdrawal = (trx_d.is_withdrawal());
        double convRate = CurrencyHistoryModel::instance().get_id_date_rate(
            AccountModel::instance().get_id_data_n(trx_d.m_account_id)->m_currency_id,
            trx_d.m_date()
        );

        if (const auto trxId_tpA = trxId_tpA_m.find(trx_d.m_id); trxId_tpA == trxId_tpA_m.end()) {
            int64 category_id = trx_d.m_category_id_n;
            if (withdrawal)
                stat[category_id] -= trx_d.m_amount * convRate;
            else
                stat[category_id] += trx_d.m_amount * convRate;
        }
        else {
            for (const auto& tp_d : trxId_tpA->second) {
                int64 category_id = tp_d.m_category_id;
                double val = tp_d.m_amount * convRate * (withdrawal ? -1 : 1);
                stat[category_id] += val;
            }
        }
    }

    categoryStats.clear();
    for (const auto& i : stat) {
        if (i.second < 0) {
            std::pair <wxString, double> stat_pair;
            stat_pair.first = CategoryModel::instance().get_id_fullname(i.first);
            stat_pair.second = i.second;
            categoryStats.push_back(stat_pair);
        }
    }

    std::stable_sort(categoryStats.begin(), categoryStats.end(),
        [](const std::pair<wxString, double> x, const std::pair<wxString, double> y) {
            return x.second < y.second;
        }
    );

    int counter = 0;
    std::vector<std::pair<wxString, double> >::iterator iter;
    for (iter = categoryStats.begin(); iter != categoryStats.end(); ) {
        counter++;
        if (counter > 7)
            iter = categoryStats.erase(iter);
        else
            ++iter;
    }
}

htmlWidgetBillsAndDeposits::htmlWidgetBillsAndDeposits(
    const wxString& title,
    mmDateRange* date_range
) :
    m_title(title),
    m_date_range(date_range)
{}

htmlWidgetBillsAndDeposits::~htmlWidgetBillsAndDeposits()
{
    if (m_date_range)
        delete m_date_range;
}

const wxString htmlWidgetBillsAndDeposits::getHTMLText()
{
    wxString output = "";
    mmDate today = mmDate::today();

    std::vector<std::tuple<
        int                /* 0: days */,
        wxString           /* 1: payee_name */,
        wxString           /* 2: description */,
        double             /* 3: amount */,
        const AccountData* /* 4: account_n */,
        wxString           /* 5: notes */
    >> sched_info_a;
    for (const auto& sched_d : SchedModel::instance().find_all(
        SchedCol::COL_ID_TRANSDATE
    )) {
        int payment_days = sched_d.m_date().daysSince(today);
        // Stop searching
        if (payment_days > 14)
            break;
        int due_days = sched_d.m_due_date.daysSince(today);

        wxString description = (payment_days > 0)
            ? wxString::Format(wxPLURAL("%d day", "%d days", payment_days), payment_days)
            : "*" + wxString::Format(
                wxPLURAL("%d day delay", "%d days delay", -payment_days),
                -payment_days
            );
        if (due_days < 0)
            description = "*" + wxString::Format(
                wxPLURAL("%d day overdue", "%d days overdue", -due_days),
                -due_days
            );

        const auto* account_n = AccountModel::instance().get_id_data_n(sched_d.m_account_id);
        wxString account_name_n = account_n ? account_n->m_name : "";

        wxString payee_name_n = "";
        if (sched_d.is_transfer()) {
            const AccountData* to_account_n = AccountModel::instance().get_id_data_n(
                sched_d.m_to_account_id_n
            );
            if (to_account_n)
                payee_name_n = to_account_n->m_name;
            payee_name_n += " &larr; " + account_name_n;
        }
        else {
            const PayeeData* payee_n = PayeeModel::instance().get_id_data_n(
                sched_d.m_payee_id_n
            );
            payee_name_n = account_name_n;
            payee_name_n += (sched_d.is_withdrawal() ? " &rarr; " : " &larr; ");
            if (payee_n)
                payee_name_n += payee_n->m_name;
        }
        double amount = (sched_d.is_withdrawal() ? -sched_d.m_amount : sched_d.m_amount);
        wxString notes = HTMLEncode(sched_d.m_notes);
        sched_info_a.push_back(std::make_tuple(
            payment_days, payee_name_n, description, amount, account_n, notes
        ));
    }

    //std::sort(sched_info_a.begin(), sched_info_a.end());
    //std::reverse(sched_info_a.begin(), sched_info_a.end());

    if (!sched_info_a.empty()) {
        static const wxString idStr = "BILLS_AND_DEPOSITS";

        output = R"(<div class="shadow">)";
        output += "<table class='table'>\n<thead>\n<tr class='active'><th>";
        output += wxString::Format("<a href=\"billsdeposits:\" oncontextmenu=\"return false;\" target=\"_blank\">%s</a></th>\n<th></th>\n",
            m_title
        );
        output += wxString::Format("<th nowrap class='text-right sorttable_nosort'>%i <a id='%s_label' onclick=\"toggleTable('%s'); \" href='#%s' oncontextmenu='return false;'>[-]</a></th></tr>\n",
            static_cast<int>(sched_info_a.size()), idStr, idStr, idStr
        );
        output += "</thead></table>\n";

        output += wxString::Format("<table class='table' id='%s'>\n", idStr);
        output += wxString::Format("<thead><tr><th>%s</th>\n<th class='text-right'>%s</th>\n<th class='text-right'>%s</th></tr></thead>\n",
            _t("Account/Payee"), _t("Amount"), _t("Remaining")
        );

        for (const auto& sched_info : sched_info_a) {
            int                days        = std::get<0>(sched_info);
            wxString           payee_name  = std::get<1>(sched_info);
            wxString           description = std::get<2>(sched_info);
            double             amount      = std::get<3>(sched_info);
            const AccountData* account_n   = std::get<4>(sched_info);
            wxString           notes       = std::get<5>(sched_info);

            output += wxString::Format("<tr %s>\n",
                days < 0 ? "class='danger'" : ""
            );
            output += "<td>" + payee_name;
            if (notes.Length() > 150)
                notes = notes.Left(150) + wxString::FromUTF8Unchecked("…");
            if (!notes.IsEmpty())
                output += wxString::Format("<br><i>%s</i>", notes);
            output += "</td>";
            output += wxString::Format("<td class='money'>%s</td>\n",
                AccountModel::instance().value_number_currency(
                    *account_n, amount
                )
            );
            output += "<td class='money'>" + description + "</td></tr>\n";
        }
        output += "</table>\n";
        output += "</div>";
    }
    return output;
}

const wxString htmlWidgetIncomeVsExpenses::getHTMLText()
{
    DashboardPref home_options;
    wxSharedPtr<mmDateRange> date_range(home_options.get_inc_vs_exp_date_range());

    double tIncome = 0.0, tExpenses = 0.0;
    std::map<int64, std::pair<double, double> > incomeExpensesStats;

    // Calculations
    for (const auto& trx_d : TrxModel::instance().find(
        TrxModel::DATE(OP_GE, mmDate(date_range.get()->start_date())),
        TrxModel::DATE(OP_LE, mmDate(date_range.get()->end_date())),
        TrxModel::TYPE(OP_NE, TrxType(TrxType::e_transfer)),
        TrxModel::IS_VOID(false)
    )) {
        // Do not include asset or stock transfers or deleted transactions
        // in income expense calculations.
        if (TrxModel::is_foreignAsTransfer(trx_d) || trx_d.is_deleted())
            continue;

        double convRate = CurrencyHistoryModel::instance().get_id_date_rate(
            AccountModel::instance().get_id_data_n(trx_d.m_account_id)->m_currency_id,
            trx_d.m_date()
        );

        int64 idx = trx_d.m_account_id;
        if (trx_d.is_deposit())
            incomeExpensesStats[idx].first += trx_d.m_amount * convRate;
        else
            incomeExpensesStats[idx].second += trx_d.m_amount * convRate;
    }

    for (const auto& account_d : AccountModel::instance().find_all()) {
        int64 idx = account_d.m_id;
        tIncome += incomeExpensesStats[idx].first;
        tExpenses += incomeExpensesStats[idx].second;
    }


    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();
    json_writer.Key("0");
    json_writer.String(wxString::Format(_t("Income vs. Expenses: %s"),
        date_range.get()->local_title()).utf8_str()
    );
    json_writer.Key("1");
    json_writer.String(_t("Type").utf8_str());
    json_writer.Key("2");
    json_writer.String(_t("Amount").utf8_str());
    json_writer.Key("3");
    json_writer.String(_t("Income").utf8_str());
    json_writer.Key("4");
    json_writer.String(CurrencyModel::instance().toCurrency(tIncome).utf8_str());
    json_writer.Key("5");
    json_writer.String(_t("Expenses").utf8_str());
    json_writer.Key("6");
    json_writer.String(CurrencyModel::instance().toCurrency(tExpenses).utf8_str());
    json_writer.Key("7");
    json_writer.String(_t("Difference:").utf8_str());
    json_writer.Key("8");
    json_writer.String(CurrencyModel::instance().toCurrency(tIncome - tExpenses).utf8_str());
    json_writer.Key("9");
    json_writer.String(_t("Income/Expenses").utf8_str());
    json_writer.Key("10");
    json_writer.String(wxString::FromCDouble(tIncome, 2).utf8_str());
    json_writer.Key("11");
    json_writer.String(wxString::FromCDouble(tExpenses, 2).utf8_str());
    json_writer.EndObject();

    wxLogDebug("======= DashboardPanel::getIncomeVsExpensesJSON =======");
    wxLogDebug("RapidJson\n%s", wxString::FromUTF8(json_buffer.GetString()));

    return wxString::FromUTF8(json_buffer.GetString());
}

htmlWidgetIncomeVsExpenses::~htmlWidgetIncomeVsExpenses()
{
}

const wxString htmlWidgetStatistics::getHTMLText()
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();

    json_writer.Key("NAME");
    json_writer.String(_t("Transaction Statistics").utf8_str());

    wxSharedPtr<mmDateRange> date_range;
    /*if (PrefModel::instance().getIgnoreFutureTransactions())
        date_range = new mmCurrentMonthToDate;
    else
        date_range = new mmCurrentMonth;*/

    TrxModel::DataA trx_a;
    if (PrefModel::instance().getIgnoreFutureTransactionsHomePage()) {
        date_range = new mmCurrentMonthToDate;
        trx_a = TrxModel::instance().find(
            TrxModel::DATE(OP_LE, mmDate::today()));
    }
    else {
        date_range = new mmCurrentMonth;
        trx_a = TrxModel::instance().find_all();
    }
    int countFollowUp = 0;
    int total_transactions = 0;

    std::map<int64, std::pair<double, double> > accountStats;
    for (const auto& trx_d : trx_a) {
        if (trx_d.is_deleted())
            continue;

        total_transactions++;

        // Do not include asset or stock transfers in income expense calculations.
        if (TrxModel::is_foreignAsTransfer(trx_d))
            continue;

        if (trx_d.m_status.id() == TrxStatus::e_followup)
            countFollowUp++;

        accountStats[trx_d.m_account_id].first +=
            trx_d.account_recflow(trx_d.m_account_id);
        accountStats[trx_d.m_account_id].second +=
            trx_d.account_flow(trx_d.m_account_id);

        if (trx_d.is_transfer()) {
            accountStats[trx_d.m_to_account_id_n].first +=
                trx_d.account_recflow(trx_d.m_to_account_id_n);
            accountStats[trx_d.m_to_account_id_n].second +=
                trx_d.account_flow(trx_d.m_to_account_id_n);
        }
    }


    if (countFollowUp > 0) {
        json_writer.Key(_t("Follow Up On Transactions: ").utf8_str());
        json_writer.Double(countFollowUp);
    }

    json_writer.Key(_t("Total Transactions: ").utf8_str());
    json_writer.Int(total_transactions);
    json_writer.EndObject();

    wxLogDebug("======= DashboardPanel::getStatWidget =======");
    wxLogDebug("RapidJson\n%s", wxString::FromUTF8(json_buffer.GetString()));

    return wxString::FromUTF8(json_buffer.GetString());
}

htmlWidgetStatistics::~htmlWidgetStatistics()
{
}

const wxString htmlWidgetGrandTotals::getHTMLText(double tBalance, double tReconciled, double tAssets, double tStocks)
{
    wxString output = "<th>" + _t("Total Net Worth") + "</th>";

    output += wxString::Format("<th>%s: <span class='money'>%s</span></th>"
                                        , ( PrefModel::instance().getShowReconciledInHomePage() ? _t("Reconciled") : _t("Accounts"))
                                        , CurrencyModel::instance().toCurrency(tReconciled));
    output +=  wxString::Format("<th>%s: <span class='money'>%s</span></th>"
                                        , _t("Assets")
                                        , CurrencyModel::instance().toCurrency(tAssets));
    output +=  wxString::Format("<th>%s: <span class='money'>%s</span></th>"
                                        , _t("Stock")
                                        , CurrencyModel::instance().toCurrency(tStocks));
    output += wxString::Format("<th>%s: <span class='money'>%s</span></th>"
                                        , _t("Balance")
                                        , CurrencyModel::instance().toCurrency(tBalance));

    return output;
}

htmlWidgetGrandTotals::~htmlWidgetGrandTotals()
{
}

const wxString htmlWidgetAssets::getHTMLText()
{
    AccountModel::DataA asset_account_a = AccountModel::instance().find(
        AccountCol::ACCOUNTTYPE(NavigatorTypes::instance().getAssetAccountStr())
    );
    if (asset_account_a.empty())
        return wxEmptyString;

    std::stable_sort(asset_account_a.begin(), asset_account_a.end(),
        AccountData::SorterByName()
    );

    static const int MAX_ASSETS = 10;
    wxString output;
    output << R"(<div class="shadow">)"
           << R"(<table class='sortable table'><col style='width: 50%'><col style='width: 12.5%'><col style='width: 12.5%'><col style='width: 12.5%'><col style='width: 12.5%'>)"
           << "<thead><tr class='active'>\n"
           << "<th>" << _t("Assets") << "</th>"
           << "<th class='text-right'>" << _t("Initial Value") << "</th>\n"
           << "<th class='text-right'>" << _t("Current Value") << "</th>\n"
           << "<th class='text-right'>" << _t("Cash Balance") << "</th>\n"
           << "<th class='text-right'>" << _t("Total") << "</th>\n"
           << wxString::Format("<th nowrap class='text-right sorttable_nosort'><a id='%s_label' onclick='toggleTable(\"%s\");' href='#%s' oncontextmenu='return false;'>[-]</a></th>\n",
                               "ASSETS", "ASSETS", "ASSETS")
           << "</tr></thead><tbody id='ASSETS'>\n";

    int rows = 0;
    double initialDisplayed = 0.0, initialTotal = 0.0;
    double currentDisplayed = 0.0, currentTotal = 0.0;
    double cashDisplayed = 0.0, cashTotal = 0.0;

    auto renderRow = [](const wxString& name, double initial, double current, double cash) -> wxString {
        wxString row;
        row << "<tr>";
        row << wxString::Format("<td sorttable_customkey='*%s*'>%s</td>\n", name, name);
        row << wxString::Format("<td class='money' sorttable_customkey='%.2f'>%s</td>\n",
            initial, CurrencyModel::instance().toCurrency(initial)
        );
        row << wxString::Format("<td class='money' sorttable_customkey='%.2f'>%s</td>\n",
            current, CurrencyModel::instance().toCurrency(current)
        );
        row << wxString::Format("<td class='money' sorttable_customkey='%.2f'>%s</td>\n",
            cash, CurrencyModel::instance().toCurrency(cash)
        );
        row << wxString::Format("<td colspan='2' class='money' sorttable_customkey='%.2f'>%s</td>\n",
            current + cash, CurrencyModel::instance().toCurrency(current + cash)
        );
        row << "</tr>\n";
        return row;
    };

    for (const auto& asset_account_d : asset_account_a) {
        if (!asset_account_d.is_open())
            continue;

        double cash = AccountModel::instance().get_data_balance(asset_account_d);
        auto inv = AccountModel::instance().get_data_investment_balance(asset_account_d);
        double current = inv.first;
        double initial = inv.second;

        initialTotal += initial;
        currentTotal += current;
        cashTotal += cash;

        if (rows++ < MAX_ASSETS) {
            initialDisplayed += initial;
            currentDisplayed += current;
            cashDisplayed += cash;
            output << renderRow(asset_account_d.m_name, initial, current, cash);
        }
    }

    if (rows > MAX_ASSETS) {
        wxString otherAssets = _t("Other Assets");
        output << renderRow(
            wxString::Format("%s (%d)", otherAssets, rows - MAX_ASSETS),
            initialTotal - initialDisplayed,
            currentTotal - currentDisplayed,
            cashTotal - cashDisplayed
        );
    }

    output << "<tfoot><tr class='total'><td>" << _t("Total:") << "</td>\n"
           << wxString::Format("<td class='money'>%s</td>\n", CurrencyModel::instance().toCurrency(initialTotal))
           << wxString::Format("<td class='money'>%s</td>\n", CurrencyModel::instance().toCurrency(currentTotal))
           << wxString::Format("<td class='money'>%s</td>\n", CurrencyModel::instance().toCurrency(cashTotal))
           << wxString::Format("<td colspan='2' class='money'>%s</td></tr></tfoot></table>\n",
                               CurrencyModel::instance().toCurrency(currentTotal + cashTotal))
           << "</div>";

    return output;
}

htmlWidgetAssets::~htmlWidgetAssets()
{
}

//

htmlWidgetAccounts::htmlWidgetAccounts()
{
    get_account_stats();
}

void htmlWidgetAccounts::get_account_stats()
{

    wxSharedPtr<mmDateRange> date_range;
    /*if (PrefModel::instance().getIgnoreFutureTransactions())
        date_range = new mmCurrentMonthToDate;
    else
        date_range = new mmCurrentMonth;*/

    TrxModel::DataA trx_a;
    if (PrefModel::instance().getIgnoreFutureTransactionsHomePage()) {
        date_range = new mmCurrentMonthToDate;
        trx_a = TrxModel::instance().find(
            TrxModel::DATE(OP_LE, mmDate::today())
        );
    }
    else {
        date_range = new mmCurrentMonth;
        trx_a = TrxModel::instance().find_all();
    }

    for (const auto& trx_d : trx_a) {
        accountStats_[trx_d.m_account_id].first +=
            trx_d.account_recflow(trx_d.m_account_id);
        accountStats_[trx_d.m_account_id].second +=
            trx_d.account_flow(trx_d.m_account_id);

        if (trx_d.is_transfer()) {
            accountStats_[trx_d.m_to_account_id_n].first +=
                trx_d.account_recflow(trx_d.m_to_account_id_n);
            accountStats_[trx_d.m_to_account_id_n].second +=
                trx_d.account_flow(trx_d.m_to_account_id_n);
        }
    }
}

const wxString htmlWidgetAccounts::displayAccounts(
    double& tBalance,
    double& tReconciled,
    int type = NavigatorTypes::TYPE_ID_CHECKING
) {
    NavigatorTypesInfo* ninfo = NavigatorTypes::instance().FindEntry(type);
    bool showReconciled = PrefModel::instance().getShowReconciledInHomePage();

    wxString idStr = ninfo->choice;
    wxString output = "<table class = 'sortable table'>\n";
    if (showReconciled) {
        output += R"(<col style="width:50%"><col style="width:25%"><col style="width:25%">)";
    }
    else {
        output += R"(<col style="width:67%"><col style="width:33%">)";
    }
    output += "<thead><tr><th nowrap>\n";
    output += wxGetTranslation(ninfo->name);
    if (showReconciled) {
        output += "</th><th class = 'text-right'>" + _t("Reconciled") + "</th>\n";
    }
    output += "<th class = 'text-right'>" + _t("Balance") + "</th>\n";
    output += wxString::Format("<th nowrap class='text-right sorttable_nosort'><a id='%s_label' onclick=\"toggleTable('%s'); \" href='#%s' oncontextmenu='return false;'>[-]</a></th>\n"
        , idStr, idStr, idStr);
    output += "</tr></thead>\n";
    output += wxString::Format("<tbody id = '%s'>\n", idStr);

    wxString body = "";
    double tabBalance = 0.0, tabReconciled = 0.0;
    wxString vAccts = SettingModel::instance().getViewAccounts();
    auto account_a = AccountModel::instance().find(
        AccountCol::ACCOUNTTYPE(NavigatorTypes::instance().type_name(type)),
        AccountModel::STATUS(OP_NE, AccountStatus(AccountStatus::e_closed))
    );
    std::stable_sort(account_a.begin(), account_a.end(), AccountData::SorterByName());
    for (const auto& account_d : account_a) {
        const CurrencyData* currency = AccountModel::instance().get_data_currency_p(account_d);

        double currency_rate = CurrencyHistoryModel::instance().get_id_date_rate(
            account_d.m_currency_id
        );
        double bal = account_d.m_open_balance + accountStats_[account_d.m_id].second;
        //AccountModel::instance().get_data_balance(account_d);
        double reconciledBal = account_d.m_open_balance + accountStats_[account_d.m_id].first;
        tabBalance += bal * currency_rate;
        tabReconciled += reconciledBal * currency_rate;

        // show the actual amount in that account
        if ((vAccts == VIEW_ACCOUNTS_OPEN_STR && account_d.is_open()) ||
            (vAccts == VIEW_ACCOUNTS_FAVORITES_STR && account_d.is_favorite()) ||
            (vAccts == VIEW_ACCOUNTS_ALL_STR)
        ) {
            body += "<tr>";
            body += wxString::Format(R"(<td sorttable_customkey="*%s*" nowrap><a href="acct:%lld" oncontextmenu="return false;" target="_blank">%s</a>%s</td>)",
                account_d.m_name, account_d.m_id, account_d.m_name,
                account_d.m_website.empty() ? "" : wxString::Format(R"(&nbsp;&nbsp;&nbsp;&nbsp;(<a href="%s" oncontextmenu="return false;" target="_blank">WWW</a>))",
                    account_d.m_website
                )
            );
            if (showReconciled) {
                body += wxString::Format("\n<td class='money' sorttable_customkey='%f' nowrap>%s</td>\n",
                    reconciledBal,
                    CurrencyModel::instance().toCurrency(reconciledBal, currency)
                );
            }
            body += wxString::Format("<td class='money' sorttable_customkey='%f' colspan='2' nowrap>%s</td>\n", bal, CurrencyModel::instance().toCurrency(bal, currency));
            body += "</tr>\n";
        }
    }
    output += body;
    output += "</tbody><tfoot><tr class ='total'><td>" + _t("Total:") + "</td>\n";

    if (showReconciled) {
        output += "<td class='money'>" + CurrencyModel::instance().toCurrency(tabReconciled) + "</td>\n";
    }
    output += "<td class='money' colspan='2'>" + CurrencyModel::instance().toCurrency(tabBalance) + "</td></tr></tfoot></table>\n";
    if (body.empty()) output.clear();

    tBalance += tabBalance;
    tReconciled += tabReconciled;

    return output;
}

htmlWidgetAccounts::~htmlWidgetAccounts()
{
}

// Currency exchange rates
const wxString htmlWidgetCurrency::getHtmlText()
{

    const char* currencyRatesTemplate = R"(
<div class = "shadow">
<table class="table">
<thead>
<tr class='active'><th><TMPL_VAR FRAME_NAME></th>
<th nowrap class='text-right sorttable_nosort'>
<a id='CURRENCY_RATES_label' onclick='toggleTable("CURRENCY_RATES");' href='#CURRENCY_RATES' oncontextmenu='return false;'>[-]</a>
</th></tr>
<tbody id='CURRENCY_RATES'>
<tr>
<td style='padding: 0px; padding-left: 0px; padding-right: 0px; width: 100%;' colspan='2'>

<table class="table">
<thead>
<tr><th nowrap class="text-right sorttable_nosort"></th><TMPL_VAR HEADER></tr>
</thead>
<tbody>
<TMPL_LOOP NAME=CONTENTS>
<tr><td class ='success'><TMPL_VAR CURRENCY_SYMBOL></td><TMPL_VAR CONVERSION_RATE></tr>
</TMPL_LOOP>
</tbody>
</table>

</td>
</tr>
</tbody>
</table>
</div>
)";


    std::map<wxString, double> usedRates;

    for (const auto& currency_d : CurrencyModel::instance().find_all()) {
        if (CurrencyModel::instance().find_id_dep_c(currency_d.m_id) > 0) {

            double convertionRate = CurrencyHistoryModel::instance().get_id_date_rate(
                currency_d.m_id
            );
            usedRates[currency_d.m_symbol] = convertionRate;

            if (usedRates.size() >= 10) {
                break;
            }
        }
    }

    if (usedRates.size() == 1) {
        return "";
    }
    wxString header;
    loop_t contents;
    for (const auto& i : usedRates) {
        row_t r;
        r(L"CURRENCY_SYMBOL") = i.first;
        wxString row;
        for (const auto& j : usedRates) {
            double value = j.second / i.second;
            row += wxString::Format("<td %s>%s</td>"
                , j.first == i.first ? "class ='active'" : "class='money'"
                , j.first == i.first ? "" : CurrencyModel::instance().toString(value, nullptr, 4)
            );
        }
        header += wxString::Format("<th class='text-center'>%s</th>", i.first);
        r(L"CONVERSION_RATE") = row;

        contents += r;
    }
    mm_html_template report(currencyRatesTemplate);
    report(L"CONTENTS") = contents;
    report(L"FRAME_NAME") = _t("Currency Exchange Rates");
    report(L"HEADER") = header;

    wxString out = wxEmptyString;
    try {
        out = report.Process();
    }
    catch (const syntax_ex& e) {
        return e.what();
    }
    catch (...) {
        return _t("Caught exception");
    }

    return out;

}

htmlWidgetCurrency::~htmlWidgetCurrency()
{
}
