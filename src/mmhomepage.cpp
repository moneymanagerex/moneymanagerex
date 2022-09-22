/*******************************************************
Copyright (C) 2014 - 2021 Nikolay Akimov
Copyright (C) 2021-2022 Mark Whalley (mark@ipx.co.uk)

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

#include "mmhomepage.h"
#include "html_template.h"
#include "billsdepositspanel.h"
#include "option.h"
#include "optionsettingshome.h"
#include "constants.h"
#include <algorithm>
#include <cmath>

#include "model/Model_Stock.h"
#include "model/Model_StockHistory.h"
#include "model/Model_Category.h"
#include "model/Model_Currency.h"
#include "model/Model_CurrencyHistory.h"
#include "model/Model_Payee.h"
#include "model/Model_Asset.h"
#include "model/Model_Setting.h"

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


htmlWidgetStocks::htmlWidgetStocks()
    : title_(_("Stocks"))
{
    grand_gain_lost_ = 0.0;
    grand_total_ = 0.0;
}

htmlWidgetStocks::~htmlWidgetStocks()
{
}

const wxString htmlWidgetStocks::getHTMLText()
{
    wxString output = "";
    std::map<int, std::pair<double, double> > stockStats;
    calculate_stats(stockStats);
    if (!stockStats.empty())
    {
        output = R"(<div class="shadow">)";
        output += "<table class ='sortable table'><col style='width: 50%'><col style='width: 25%'><col style='width: 25%'><thead><tr class='active'><th>\n";
        output += _("Stocks") + "</th><th class = 'text-right'>" + _("Gain/Loss");
        output += "</th>\n<th class='text-right'>" + _("Total") + "</th>\n";
        output += wxString::Format("<th nowrap class='text-right sorttable_nosort'><a id='%s_label' onclick='toggleTable(\"%s\");' href='#%s' oncontextmenu='return false;'>[-]</a></th>\n"
            , "INVEST", "INVEST", "INVEST");
        output += "</tr></thead><tbody id='INVEST'>\n";
        const auto &accounts = Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME);
        wxString body = "";
        for (const auto& account : accounts)
        {
            if (Model_Account::type(account) != Model_Account::INVESTMENT) continue;
            if (Model_Account::status(account) != Model_Account::OPEN) continue;
            body += "<tr>";
            body += wxString::Format("<td sorttable_customkey='*%s*'><a href='stock:%i' oncontextmenu='return false;' target='_blank'>%s</a>%s</td>\n"
                , account.ACCOUNTNAME, account.ACCOUNTID, account.ACCOUNTNAME,
                account.WEBSITE.empty() ? "" : wxString::Format("&nbsp;&nbsp;&nbsp;&nbsp;(<a href='%s' oncontextmenu='return false;' target='_blank'>WWW</a>)", account.WEBSITE));
            body += wxString::Format("<td class='money' sorttable_customkey='%f'>%s</td>\n"
                , stockStats[account.ACCOUNTID].first
                , Model_Account::toCurrency(stockStats[account.ACCOUNTID].first, &account));
            body += wxString::Format("<td colspan='2' class='money' sorttable_customkey='%f'>%s</td>"
                , stockStats[account.ACCOUNTID].second
                , Model_Account::toCurrency(stockStats[account.ACCOUNTID].second, &account));
            body += "</tr>";
        }

        if (!body.empty())
        {
            output += body;
            output += "</tbody><tfoot><tr class = 'total'><td>" + _("Total:") + "</td>";
            output += wxString::Format("<td class='money'>%s</td>"
                , Model_Currency::toCurrency(grand_gain_lost_));
            output += wxString::Format("<td colspan='2' class='money'>%s</td></tr></tfoot></table>\n"
                , Model_Currency::toCurrency(grand_total_));
            output += "</div>";
        }
    }
    return output;
}

void htmlWidgetStocks::calculate_stats(std::map<int, std::pair<double, double> > &stockStats)
{
    this->grand_total_ = 0;
    this->grand_gain_lost_ = 0;
    const auto &stocks = Model_Stock::instance().all();
    const wxDate today = wxDate::Today();
    for (const auto& stock : stocks)
    {
        double conv_rate = 1;
        Model_Account::Data *account = Model_Account::instance().get(stock.HELDAT);
        if (account)
        {
            conv_rate = Model_CurrencyHistory::getDayRate(account->CURRENCYID, today);
        }
        std::pair<double, double>& values = stockStats[stock.HELDAT];
        double current_value = Model_Stock::CurrentValue(stock);
        double gain_lost = current_value - Model_Stock::InvestmentValue(stock);
        values.first += gain_lost;
        values.second += current_value;
        if (account && account->STATUS == VIEW_ACCOUNTS_OPEN_STR)
        {
            grand_total_ += current_value * conv_rate;
            grand_gain_lost_ += gain_lost * conv_rate;
        }
    }
}

double htmlWidgetStocks::get_total()
{
    return grand_total_;
}

double htmlWidgetStocks::get_total_gein_lost()
{
    return grand_gain_lost_;
}

////////////////////////////////////////////////////////


htmlWidgetTop7Categories::htmlWidgetTop7Categories()
{
    date_range_ = new mmLast30Days();
    title_ = wxString::Format(_("Top Withdrawals: %s"), date_range_->local_title());
}

htmlWidgetTop7Categories::~htmlWidgetTop7Categories()
{
    if (date_range_) delete date_range_;
}

const wxString htmlWidgetTop7Categories::getHTMLText()
{

    std::vector<std::pair<wxString, double> > topCategoryStats;
    getTopCategoryStats(topCategoryStats, date_range_);
    wxString output, data;

    if (!topCategoryStats.empty())
    {
        output = R"(<div class="shadow">)";
        for (const auto& i : topCategoryStats)
        {
            data += "<tr>";
            data += wxString::Format("<td>%s</td>", (i.first.IsEmpty() ? "..." : i.first));
            data += wxString::Format("<td class='money' sorttable_customkey='%f'>%s</td>\n"
                , i.second
                , Model_Currency::toCurrency(i.second));
            data += "</tr>\n";
        }
        const wxString idStr = "TOP_CATEGORIES";
        output += wxString::Format(TOP_CATEGS, title_, idStr, idStr, idStr, idStr, _("Category"), _("Summary"), data);
        output += "</div>";
    }

    return output;
}

void htmlWidgetTop7Categories::getTopCategoryStats(
    std::vector<std::pair<wxString, double> > &categoryStats
    , const mmDateRange* date_range) const
{
    //Get base currency rates for all accounts
    std::map<int, double> acc_conv_rates;
    const wxDate today = wxDate::Today();
    for (const auto& account : Model_Account::instance().all())
    {
        acc_conv_rates[account.ACCOUNTID] = Model_CurrencyHistory::getDayRate(account.CURRENCYID, today);
    }
    //Temporary map
    std::map<std::pair<int /*category*/, int /*sub category*/>, double> stat;

    const auto split = Model_Splittransaction::instance().get_all();
    const auto &transactions = Model_Checking::instance().find(
        Model_Checking::TRANSDATE(date_range->start_date(), GREATER_OR_EQUAL)
        , Model_Checking::TRANSDATE(date_range->end_date(), LESS_OR_EQUAL)
        , Model_Checking::STATUS(Model_Checking::VOID_, NOT_EQUAL)
        , Model_Checking::TRANSCODE(Model_Checking::TRANSFER, NOT_EQUAL));

    for (const auto &trx : transactions)
    {
        // Do not include asset or stock transfers in income expense calculations.
        if (Model_Checking::foreignTransactionAsTransfer(trx))
            continue;

        bool withdrawal = Model_Checking::type(trx) == Model_Checking::WITHDRAWAL;
        const auto it = split.find(trx.TRANSID);

        if (it == split.end())
        {
            std::pair<int, int> category = std::make_pair(trx.CATEGID, trx.SUBCATEGID);
            if (withdrawal)
                stat[category] -= trx.TRANSAMOUNT * (acc_conv_rates[trx.ACCOUNTID]);
            else
                stat[category] += trx.TRANSAMOUNT * (acc_conv_rates[trx.ACCOUNTID]);
        }
        else
        {
            for (const auto& entry : it->second)
            {
                std::pair<int, int> category = std::make_pair(entry.CATEGID, entry.SUBCATEGID);
                double val = entry.SPLITTRANSAMOUNT
                    * (acc_conv_rates[trx.ACCOUNTID])
                    * (withdrawal ? -1 : 1);
                stat[category] += val;
            }
        }
    }

    categoryStats.clear();
    for (const auto& i : stat)
    {
        if (i.second < 0)
        {
            std::pair <wxString, double> stat_pair;
            stat_pair.first = Model_Category::full_name(i.first.first, i.first.second);
            stat_pair.second = i.second;
            categoryStats.push_back(stat_pair);
        }
    }

    std::stable_sort(categoryStats.begin(), categoryStats.end()
        , [](const std::pair<wxString, double> x, const std::pair<wxString, double> y)
    { return x.second < y.second; }
    );

    int counter = 0;
    std::vector<std::pair<wxString, double> >::iterator iter;
    for (iter = categoryStats.begin(); iter != categoryStats.end(); )
    {
        counter++;
        if (counter > 7)
            iter = categoryStats.erase(iter);
        else
            ++iter;
    }
}

////////////////////////////////////////////////////////


htmlWidgetBillsAndDeposits::htmlWidgetBillsAndDeposits(const wxString& title, mmDateRange* date_range)
    : title_(title)
    , date_range_(date_range)
{}

htmlWidgetBillsAndDeposits::~htmlWidgetBillsAndDeposits()
{
    if (date_range_) delete date_range_;
}

const wxString htmlWidgetBillsAndDeposits::getHTMLText()
{
    wxString output = "";
    wxDate today = wxDate::Today();

    //                    days, payee, description, amount, account, notes
    std::vector< std::tuple<int, wxString, wxString, double, const Model_Account::Data*, wxString> > bd_days;
    for (const auto& entry : Model_Billsdeposits::instance().all(Model_Billsdeposits::COL_TRANSDATE))
    {
        int daysPayment = Model_Billsdeposits::TRANSDATE(&entry)
            .Subtract(today).GetDays();
        if (daysPayment > 14)
            break; // Done searching for all to include

        int repeats = entry.REPEATS;
        // DeMultiplex the Auto Executable fields.
        if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute User Acknowlegement required
            repeats -= BD_REPEATS_MULTIPLEX_BASE;
        if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute Silent mode
            repeats -= BD_REPEATS_MULTIPLEX_BASE;

        if (daysPayment == 0 && repeats > 10 && repeats < 15 && entry.NUMOCCURRENCES < 0) {
            continue; // Inactive
        }

        int daysOverdue = Model_Billsdeposits::NEXTOCCURRENCEDATE(&entry)
            .Subtract(today).GetDays();
        wxString daysRemainingStr = (daysPayment > 0
            ? wxString::Format(wxPLURAL("%d day remaining", "%d days remaining", daysPayment), daysPayment)
            : wxString::Format(wxPLURAL("%d day delay!", "%d days delay!", -daysPayment), -daysPayment));
        if (daysOverdue < 0)
            daysRemainingStr = wxString::Format(wxPLURAL("%d day overdue!", "%d days overdue!", std::abs(daysOverdue)), std::abs(daysOverdue));

        wxString accountStr = "";
        const auto *account = Model_Account::instance().get(entry.ACCOUNTID);
        if (account) accountStr = account->ACCOUNTNAME;

        wxString payeeStr = "";
        if (Model_Billsdeposits::type(entry) == Model_Billsdeposits::TRANSFER)
        {
            const Model_Account::Data *to_account = Model_Account::instance().get(entry.TOACCOUNTID);
            if (to_account) payeeStr = to_account->ACCOUNTNAME;
            payeeStr += " &larr; " + accountStr;
        }
        else
        {
            const Model_Payee::Data* payee = Model_Payee::instance().get(entry.PAYEEID);
            payeeStr = accountStr;
            payeeStr += (Model_Billsdeposits::type(entry) == Model_Billsdeposits::WITHDRAWAL ? " &rarr; " : " &larr; ");
            if (payee) payeeStr += payee->PAYEENAME;
        }
        double amount = (Model_Billsdeposits::type(entry) == Model_Billsdeposits::WITHDRAWAL ? -entry.TRANSAMOUNT : entry.TRANSAMOUNT);
        wxString notes = HTMLEncode(entry.NOTES);
        bd_days.push_back(std::make_tuple(daysPayment, payeeStr, daysRemainingStr, amount, account, notes));
    }

    //std::sort(bd_days.begin(), bd_days.end());
    //std::reverse(bd_days.begin(), bd_days.end());
    ////////////////////////////////////

    if (!bd_days.empty())
    {
        static const wxString idStr = "BILLS_AND_DEPOSITS";

        output = R"(<div class="shadow">)";
        output += "<table class='table'>\n<thead>\n<tr class='active'><th>";
        output += wxString::Format("<a href=\"billsdeposits:\" oncontextmenu=\"return false;\" target=\"_blank\">%s</a></th>\n<th></th>\n", title_);
        output += wxString::Format("<th nowrap class='text-right sorttable_nosort'>%i <a id='%s_label' onclick=\"toggleTable('%s'); \" href='#%s' oncontextmenu='return false;'>[-]</a></th></tr>\n"
            , int(bd_days.size()), idStr, idStr, idStr);
        output += "</thead></table>\n";

        output += wxString::Format("<table class='table' id='%s'>\n", idStr);
        output += wxString::Format("<thead><tr><th>%s</th>\n<th class='text-right'>%s</th>\n<th class='text-right'>%s</th></tr></thead>\n"
            , _("Account / Payee"), _("Amount"), _("Payment"));

        for (const auto& item : bd_days)
        {
            output += wxString::Format("<tr %s>\n", std::get<0>(item) < 0 ? "class='danger'" : "");
            output += "<td>" + std::get<1>(item);
            wxString notes = std::get<5>(item);
            if (notes.Length() > 150)
                notes = notes.Left(150) + "...";
            if (!notes.IsEmpty())
                output += wxString::Format("<br><i>%s</i>", notes);

            output += "</td>";
            output += wxString::Format("<td class='money'>%s</td>\n"
                , Model_Account::toCurrency(std::get<3>(item), std::get<4>(item)));
            output += "<td  class='money'>" + std::get<2>(item) + "</td></tr>\n";
        }
        output += "</table>\n";
        output += "</div>";
    }
    return output;
}

////////////////////////////////////////////////////////

//* Income vs Expenses *//
const wxString htmlWidgetIncomeVsExpenses::getHTMLText()
{
    OptionSettingsHome home_options;
    wxSharedPtr<mmDateRange> date_range(home_options.get_inc_vs_exp_date_range());

    double tIncome = 0.0, tExpenses = 0.0;
    std::map<int, std::pair<double, double> > incomeExpensesStats;

    //Calculations
    const auto &transactions = Model_Checking::instance().find(
        Model_Checking::TRANSDATE(date_range.get()->start_date(), GREATER_OR_EQUAL)
        , Model_Checking::TRANSDATE(date_range.get()->end_date(), LESS_OR_EQUAL)
        , Model_Checking::STATUS(Model_Checking::VOID_, NOT_EQUAL)
        , Model_Checking::TRANSCODE(Model_Checking::TRANSFER, NOT_EQUAL)
    );

    for (const auto& pBankTransaction : transactions)
    {

        // Do not include asset or stock transfers in income expense calculations.
        if (Model_Checking::foreignTransactionAsTransfer(pBankTransaction))
            continue;

        double convRate = Model_CurrencyHistory::getDayRate(Model_Account::instance().get(pBankTransaction.ACCOUNTID)->CURRENCYID, pBankTransaction.TRANSDATE);

        int idx = pBankTransaction.ACCOUNTID;
        if (Model_Checking::type(pBankTransaction) == Model_Checking::DEPOSIT)
            incomeExpensesStats[idx].first += pBankTransaction.TRANSAMOUNT * convRate;
        else
            incomeExpensesStats[idx].second += pBankTransaction.TRANSAMOUNT * convRate;
    }

    for (const auto& account : Model_Account::instance().all())
    {
        int idx = account.ACCOUNTID;
        tIncome += incomeExpensesStats[idx].first;
        tExpenses += incomeExpensesStats[idx].second;
    }


    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();
    json_writer.Key("0");
    json_writer.String(wxString::Format(_("Income vs Expenses: %s"), date_range.get()->local_title()).utf8_str());
    json_writer.Key("1");
    json_writer.String(_("Type").utf8_str());
    json_writer.Key("2");
    json_writer.String(_("Amount").utf8_str());
    json_writer.Key("3");
    json_writer.String(_("Income").utf8_str());
    json_writer.Key("4");
    json_writer.String(Model_Currency::toCurrency(tIncome).utf8_str());
    json_writer.Key("5");
    json_writer.String(_("Expenses").utf8_str());
    json_writer.Key("6");
    json_writer.String(Model_Currency::toCurrency(tExpenses).utf8_str());
    json_writer.Key("7");
    json_writer.String(_("Difference:").utf8_str());
    json_writer.Key("8");
    json_writer.String(Model_Currency::toCurrency(tIncome - tExpenses).utf8_str());
    json_writer.Key("9");
    json_writer.String(_("Income/Expenses").utf8_str());
    json_writer.Key("10");
    json_writer.String(wxString::FromCDouble(tIncome, 2).utf8_str());
    json_writer.Key("11");
    json_writer.String(wxString::FromCDouble(tExpenses, 2).utf8_str());
    json_writer.EndObject();

    wxLogDebug("======= mmHomePagePanel::getIncomeVsExpensesJSON =======");
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
    json_writer.String(_("Transaction Statistics").utf8_str());

    wxSharedPtr<mmDateRange> date_range;
    if (Option::instance().getIgnoreFutureTransactions())
        date_range = new mmCurrentMonthToDate;
    else
        date_range = new mmCurrentMonth;

    Model_Checking::Data_Set all_trans;
    if (Option::instance().getIgnoreFutureTransactions()) {
        all_trans = Model_Checking::instance().find(
            DB_Table_CHECKINGACCOUNT_V1::TRANSDATE(date_range->today().FormatISODate(), LESS_OR_EQUAL));
    }
    else {
        all_trans = Model_Checking::instance().all();
    }
    int countFollowUp = 0;
    int total_transactions = all_trans.size();

    std::map<int, std::pair<double, double> > accountStats;
    for (const auto& trx : all_trans)
    {
        // Do not include asset or stock transfers in income expense calculations.
        if (Model_Checking::foreignTransactionAsTransfer(trx))
            continue;

        if (Model_Checking::status(trx) == Model_Checking::FOLLOWUP)
            countFollowUp++;

        accountStats[trx.ACCOUNTID].first += Model_Checking::reconciled(trx, trx.ACCOUNTID);
        accountStats[trx.ACCOUNTID].second += Model_Checking::balance(trx, trx.ACCOUNTID);

        if (Model_Checking::type(trx) == Model_Checking::TRANSFER)
        {
            accountStats[trx.TOACCOUNTID].first += Model_Checking::reconciled(trx, trx.TOACCOUNTID);
            accountStats[trx.TOACCOUNTID].second += Model_Checking::balance(trx, trx.TOACCOUNTID);
        }
    }


    if (countFollowUp > 0)
    {
        json_writer.Key(_("Follow Up On Transactions: ").utf8_str());
        json_writer.Double(countFollowUp);
    }

    json_writer.Key(_("Total Transactions: ").utf8_str());
    json_writer.Int(total_transactions);
    json_writer.EndObject();

    wxLogDebug("======= mmHomePagePanel::getStatWidget =======");
    wxLogDebug("RapidJson\n%s", wxString::FromUTF8(json_buffer.GetString()));

    return wxString::FromUTF8(json_buffer.GetString());
}

htmlWidgetStatistics::~htmlWidgetStatistics()
{
}

const wxString htmlWidgetGrandTotals::getHTMLText(double tBalance, double tReconciled, double tAssets, double tStocks)
{

    const wxString tReconciledStr  = wxString::Format("%s: <span class='money'>%s</span>"
                                        , _("Reconciled")
                                        , Model_Currency::toCurrency(tReconciled));
    const wxString tAssetStr  = wxString::Format("%s: <span class='money'>%s</span>"
                                        , _("Assets")
                                        , Model_Currency::toCurrency(tAssets));
    const wxString tStockStr  = wxString::Format("%s: <span class='money'>%s</span>"
                                        , _("Stock")
                                        , Model_Currency::toCurrency(tStocks));
    const wxString tBalanceStr  = wxString::Format("%s: <span class='money'>%s</span>"
                                        , _("Balance")
                                        , Model_Currency::toCurrency(tBalance));

    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();
    json_writer.Key("NAME");
    json_writer.String(_("Total Net Worth").utf8_str());
    json_writer.Key("RECONVALUE");
    json_writer.String(tReconciledStr.utf8_str());
    json_writer.Key("ASSETVALUE");
    json_writer.String(tAssetStr.utf8_str());
    json_writer.Key("STOCKVALUE");
    json_writer.String(tStockStr.utf8_str());
    json_writer.Key("BALVALUE");
    json_writer.String(tBalanceStr.utf8_str());


    json_writer.EndObject();

    wxLogDebug("======= mmHomePagePanel::getGrandTotalsJSON =======");
    wxLogDebug("RapidJson\n%s", wxString::FromUTF8(json_buffer.GetString()));

    return wxString::FromUTF8(json_buffer.GetString());
}

htmlWidgetGrandTotals::~htmlWidgetGrandTotals()
{
}

const wxString htmlWidgetAssets::getHTMLText()
{
    Model_Asset::Data_Set assets = Model_Asset::instance().all();
    if (assets.empty())
        return wxEmptyString;
    std::stable_sort(assets.begin(), assets.end(), SorterByVALUE());
    std::reverse(assets.begin(), assets.end());

    static const int MAX_ASSETS = 10;
    wxString output = "";
    output = R"(<div class="shadow">)";
    output += "<table class ='sortable table'><col style='width: 50%'><col style='width: 25%'><col style='width: 25%'><thead><tr class='active'>\n";
    output += "<th>" + _("Assets") + "</th>";
    output += "<th class='text-right'>" + _("Initial Value") + "</th>\n";
    output += "<th class='text-right'>" + _("Current Value") + "</th>\n";
    output += wxString::Format("<th nowrap class='text-right sorttable_nosort'><a id='%s_label' onclick='toggleTable(\"%s\");' href='#%s' oncontextmenu='return false;'>[-]</a></th>\n"
        , "ASSETS", "ASSETS", "ASSETS");
    output += "</tr></thead><tbody id='ASSETS'>\n";

    int rows = 0;
    double initialDisplayed = 0.0;
    double initialTotal = 0.0;
    double currentDisplayed = 0.0;
    double currentTotal = 0.0;
    for (const auto& asset : assets)
    {
        double initial = asset.VALUE;
        double current = Model_Asset::value(asset);
        initialTotal += initial;
        currentTotal += current;
        if (rows++ < MAX_ASSETS)
        {
            initialDisplayed += initial;
            currentDisplayed += current;
            output += "<tr>";
            output += wxString::Format("<td sorttable_customkey='*%s*'>%s</td>\n"
                , asset.ASSETNAME, asset.ASSETNAME);
            output += wxString::Format("<td class='money' sorttable_customkey='%f'>%s</td>\n"
                , initial, Model_Currency::toCurrency(initial));
            output += wxString::Format("<td colspan='2' class='money' sorttable_customkey='%f'>%s</td>\n"
                , current, Model_Currency::toCurrency(current));
            output += "</tr>";
        }
    }
    if (rows > MAX_ASSETS)
    {       
            output += "<tr>";
            output += wxString::Format("<td sorttable_customkey='*%s*'>%s (%i)</td>\n"
                , _("Other Assets"), _("Other Assets"), rows - MAX_ASSETS);
            output += wxString::Format("<td class='money' sorttable_customkey='%f'>%s</td>\n"
                , initialTotal - initialDisplayed, Model_Currency::toCurrency(initialTotal - initialDisplayed));
            output += wxString::Format("<td class='money' sorttable_customkey='%f'>%s</td>\n"
                , currentTotal - currentDisplayed, Model_Currency::toCurrency(currentTotal - currentDisplayed));
            output += "</tr>";
    }

    output += "</tbody><tfoot><tr class = 'total'><td>" + _("Total:") + "</td>";
    output += wxString::Format("<td class='money'>%s</td>\n"
        , Model_Currency::toCurrency(initialTotal));
    output += wxString::Format("<td colspan='2' class='money'>%s</td></tr></tfoot></table>\n"
        , Model_Currency::toCurrency(currentTotal));

    output += "</div>";

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
    if (Option::instance().getIgnoreFutureTransactions())
        date_range = new mmCurrentMonthToDate;
    else
        date_range = new mmCurrentMonth;

    Model_Checking::Data_Set all_trans;
    if (Option::instance().getIgnoreFutureTransactions())
    {
        all_trans = Model_Checking::instance().find(
            DB_Table_CHECKINGACCOUNT_V1::TRANSDATE(date_range->today().FormatISODate(), LESS_OR_EQUAL));
    }
    else
    {
        all_trans = Model_Checking::instance().all();
    }

    for (const auto& trx : all_trans)
    {
        // Do not include asset or stock transfers in income expense calculations.
        if (Model_Checking::foreignTransactionAsTransfer(trx))
            continue;

        accountStats_[trx.ACCOUNTID].first += Model_Checking::reconciled(trx, trx.ACCOUNTID);
        accountStats_[trx.ACCOUNTID].second += Model_Checking::balance(trx, trx.ACCOUNTID);

        if (Model_Checking::type(trx) == Model_Checking::TRANSFER)
        {
            accountStats_[trx.TOACCOUNTID].first += Model_Checking::reconciled(trx, trx.TOACCOUNTID);
            accountStats_[trx.TOACCOUNTID].second += Model_Checking::balance(trx, trx.TOACCOUNTID);
        }
    }

}

const wxString htmlWidgetAccounts::displayAccounts(double& tBalance, double& tReconciled, int type = Model_Account::CHECKING)
{
    static const std::vector < std::pair <wxString, wxString> > typeStr
    {
        { "CASH_ACCOUNTS_INFO",   _("Cash Accounts") },
        { "ACCOUNTS_INFO",        _("Bank Accounts") },
        { "CARD_ACCOUNTS_INFO",   _("Credit Card Accounts") },
        { "LOAN_ACCOUNTS_INFO",   _("Loan Accounts") },
        { "TERM_ACCOUNTS_INFO",   _("Term Accounts") },
        { "INVEST_ACCOUNTS_INFO", _("Investment Accounts") },
        { "ASSET_ACCOUNTS_INFO",  _("Asset Accounts") },
        { "SHARE_ACCOUNTS_INFO",  _("Share Accounts") },
    };

    const wxString idStr = typeStr[type].first;
    wxString output = "<table class = 'sortable table'>\n";
    output += R"(<col style="width:50%"><col style="width:25%"><col style="width:25%">)";
    output += "<thead><tr><th nowrap>\n";
    output += typeStr[type].second;

    output += "</th><th class = 'text-right'>" + _("Reconciled") + "</th>\n";
    output += "<th class = 'text-right'>" + _("Balance") + "</th>\n";
    output += wxString::Format("<th nowrap class='text-right sorttable_nosort'><a id='%s_label' onclick=\"toggleTable('%s'); \" href='#%s' oncontextmenu='return false;'>[-]</a></th>\n"
        , idStr, idStr, idStr);
    output += "</tr></thead>\n";
    output += wxString::Format("<tbody id = '%s'>\n", idStr);

    wxString body = "";
    const wxDate today = wxDate::Today();
    wxString vAccts = Model_Setting::instance().GetViewAccounts();
    auto accounts = Model_Account::instance().find(
        Model_Account::ACCOUNTTYPE(Model_Account::all_type()[type])
        , Model_Account::STATUS(Model_Account::CLOSED, NOT_EQUAL));
    std::stable_sort(accounts.begin(), accounts.end(), SorterByACCOUNTNAME());
    for (const auto& account : accounts)
    {
        Model_Currency::Data* currency = Model_Account::currency(account);

        double currency_rate = Model_CurrencyHistory::getDayRate(account.CURRENCYID, today);
        double bal = account.INITIALBAL + accountStats_[account.ACCOUNTID].second; //Model_Account::balance(account);
        double reconciledBal = account.INITIALBAL + accountStats_[account.ACCOUNTID].first;
        tBalance += bal * currency_rate;
        tReconciled += reconciledBal * currency_rate;

        // show the actual amount in that account
        if (((vAccts == VIEW_ACCOUNTS_OPEN_STR && Model_Account::status(account) == Model_Account::OPEN) ||
            (vAccts == VIEW_ACCOUNTS_FAVORITES_STR && Model_Account::FAVORITEACCT(account)) ||
            (vAccts == VIEW_ACCOUNTS_ALL_STR)))
        {
            body += "<tr>";
            body += wxString::Format(R"(<td sorttable_customkey="*%s*" nowrap><a href="acct:%i" oncontextmenu="return false;" target="_blank">%s</a>%s</td>)"
                , account.ACCOUNTNAME, account.ACCOUNTID, account.ACCOUNTNAME,
                account.WEBSITE.empty() ? "" : wxString::Format(R"(&nbsp;&nbsp;&nbsp;&nbsp;(<a href="%s" oncontextmenu="return false;" target="_blank">WWW</a>))", account.WEBSITE));
            body += wxString::Format("\n<td class='money' sorttable_customkey='%f' nowrap>%s</td>\n", reconciledBal, Model_Currency::toCurrency(reconciledBal, currency));
            body += wxString::Format("<td class='money' sorttable_customkey='%f' colspan='2' nowrap>%s</td>\n", bal, Model_Currency::toCurrency(bal, currency));
            body += "</tr>\n";
        }
    }
    output += body;
    output += "</tbody><tfoot><tr class ='total'><td>" + _("Total:") + "</td>\n";
    output += "<td class='money'>" + Model_Currency::toCurrency(tReconciled) + "</td>\n";
    output += "<td class='money' colspan='2'>" + Model_Currency::toCurrency(tBalance) + "</td></tr></tfoot></table>\n";
    if (body.empty()) output.clear();

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


    const wxString today = wxDate::Today().FormatISODate();
    const wxString baseCurrencySymbol = Model_Currency::GetBaseCurrency()->CURRENCY_SYMBOL;
    std::map<wxString, double> usedRates;
    const auto currencies = Model_Currency::instance().all();

    for (const auto currency : currencies)
    {
        if (Model_Account::is_used(currency)) {

            double convertionRate = Model_CurrencyHistory::getDayRate(currency.CURRENCYID
                , today);
            usedRates[currency.CURRENCY_SYMBOL] = convertionRate;

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
    for (const auto& i : usedRates)
    {
        row_t r;
        r(L"CURRENCY_SYMBOL") = i.first;
        wxString row;
        for (const auto& j : usedRates)
        {
            double value = j.second / i.second;
            row += wxString::Format("<td %s>%s</td>"
                , j.first == i.first ? "class ='active'" : "class='money'"
                , j.first == i.first ? "" : Model_Currency::toString(value, nullptr, 4)
            );
        }
        header += wxString::Format("<th class='text-center'>%s</th>", i.first);
        r(L"CONVERSION_RATE") = row;

        contents += r;
    }
    mm_html_template report(currencyRatesTemplate);
    report(L"CONTENTS") = contents;
    report(L"FRAME_NAME") = _("Currency Exchange Rates");
    report(L"HEADER") = header;

    wxString out = wxEmptyString;
    try
    {
        out = report.Process();
    }
    catch (const syntax_ex& e)
    {
        return e.what();
    }
    catch (...)
    {
        return _("Caught exception");
    }

    return out;

}

htmlWidgetCurrency::~htmlWidgetCurrency()
{
}
