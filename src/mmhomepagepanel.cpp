/*******************************************************
Copyright (C) 2006 Madhan Kanagavel
Copyright (C) 2014 Nikolay

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

#include "mmhomepagepanel.h"
#include "mmex.h"
#include "mmframe.h"
#include "paths.h"

#include "html_template.h"
#include "billsdepositspanel.h"
#include "reports/mmgraphincexpensesmonth.h"
#include <algorithm>

#include "constants.h"
#include "util.h"
#include "tinyxml2/tinyxml2.h"

#include "model/Model_Setting.h"
#include "model/Model_Asset.h"
#include "model/Model_Payee.h"
#include "model/Model_Stock.h"
#include "model/Model_Billsdeposits.h"
#include "model/Model_Category.h"

class htmlWidgetStocks
{
public:
    ~htmlWidgetStocks();
    htmlWidgetStocks();
    double get_total();
    double get_total_gein_lost();

    wxString getHTMLText();

protected:

    wxString title_;
    double grand_total_;
    double grand_gain_lost_;
    void calculate_stats(std::map<int, std::pair<double, double> > &stockStats);
};

htmlWidgetStocks::htmlWidgetStocks()
: title_(_("Stocks"))
{
    grand_gain_lost_ = 0.0;
    grand_total_ = 0.0;
}

htmlWidgetStocks::~htmlWidgetStocks()
{
}

wxString htmlWidgetStocks::getHTMLText()
{
    wxString output = "";
    std::map<int, std::pair<double, double> > stockStats;
    calculate_stats(stockStats);
    if (!stockStats.empty())
    {
        output = "<table class ='table'><thead><tr class='active'><th>";
        output += _("Stocks") + "</th><th class = 'text-right'>" + _("Gain/Loss");
        output += "</th><th class = 'text-right'>" + _("Total") + "</th></tr></thead><tbody id='INVEST'>";
        const auto &accounts = Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME);
        wxString body = "";
        for (const auto& account : accounts)
        {
            if (Model_Account::type(account) != Model_Account::INVESTMENT) continue;
            if (Model_Account::status(account) != Model_Account::OPEN) continue;
            body += "<tr>";
            body += wxString::Format("<td><a href=\"stock:%d\">%s</a></td>"
                , account.ACCOUNTID, account.ACCOUNTNAME);
            body += wxString::Format("<td class = \"text-right\">%s</td>"
                , Model_Account::toCurrency(stockStats[account.ACCOUNTID].first, &account));
            body += wxString::Format("<td class = \"text-right\">%s</td>"
                , Model_Account::toCurrency(stockStats[account.ACCOUNTID].second, &account));
            body += "</tr>";
        }

        output += body;
        output += "</tbody><tfoot><tr class = \"total\"><td>" + _("Total:") + "</td>";
        output += wxString::Format("<td class =\"money, text-right\">%s</td>"
            , Model_Currency::toCurrency(grand_gain_lost_));
        output += wxString::Format("<td class =\"money, text-right\">%s</td></tr></tfoot></table>"
            , Model_Currency::toCurrency(grand_total_));
        if (body.empty()) output.clear();
    }
    return output;
}

void htmlWidgetStocks::calculate_stats(std::map<int, std::pair<double, double> > &stockStats)
{
    this->grand_total_ = 0;
    this->grand_gain_lost_ = 0;
    const auto &stocks = Model_Stock::instance().all();
    for (const auto& stock : stocks)
    {   
        double conv_rate = 1;
        Model_Account::Data *account = Model_Account::instance().get(stock.HELDAT);
        if (account)
        {   
            Model_Currency::Data *currency = Model_Currency::instance().get(account->CURRENCYID);
            conv_rate = currency->BASECONVRATE;
        }   
        std::pair<double, double>& values = stockStats[stock.HELDAT];
        double gain_lost = (stock.VALUE - (stock.PURCHASEPRICE * stock.NUMSHARES) - stock.COMMISSION);
        values.first += gain_lost;
        values.second += stock.VALUE;
        if (account && account->STATUS == "Open")
        {   
            grand_total_ += stock.VALUE * conv_rate;
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

class htmlWidgetTop7Categories
{
public:
    ~htmlWidgetTop7Categories();
    explicit htmlWidgetTop7Categories(
        mmDateRange* date_range);

    wxString getHTMLText();

protected:
    mmDateRange* date_range_;
    wxString title_;
    void getTopCategoryStats(
        std::vector<std::pair<wxString, double> > &categoryStats
        , const mmDateRange* date_range) const;
};

htmlWidgetTop7Categories::htmlWidgetTop7Categories(mmDateRange* date_range)
    : date_range_(date_range)
{
    title_ = wxString::Format(_("Top Withdrawals: %s"), date_range_->title());
}

htmlWidgetTop7Categories::~htmlWidgetTop7Categories()
{
    if (date_range_) delete date_range_;
}

wxString htmlWidgetTop7Categories::getHTMLText()
{
    const wxString idStr = "TOP_CATEGORIES";

    wxString output = "<table class = 'table'><thead><tr class='active'><th>\n";
    output += title_ + wxString::Format("</th><th class='text-right'><a id=\"%s_label\" onclick=\"toggleTable('%s'); \" href='#'>[-]</a></th></tr></thead>\n", idStr, idStr);
    output += wxString::Format("<tbody id='%s'>", idStr);
    output += "<tr style='background-color: #d8ebf0'><td>";
    output += _("Category") + "</td><td class='text-right'>" + _("Summary") + "</td></tr>";
    std::vector<std::pair<wxString, double> > topCategoryStats;
    getTopCategoryStats(topCategoryStats, date_range_);

    for (const auto& i : topCategoryStats)
    {
        output += "<tr>";
        output += wxString::Format("<td>%s</td>", (i.first.IsEmpty() ? "..." : i.first));
        output += wxString::Format("<td class='text-right'>%s</td>", Model_Currency::toCurrency(i.second));
        output += "</tr>";
    }
    output += "</tbody></table>\n";

    return output;
}

void htmlWidgetTop7Categories::getTopCategoryStats(
    std::vector<std::pair<wxString, double> > &categoryStats
    , const mmDateRange* date_range) const
{
    //Get base currency rates for all accounts
    std::map<int, double> acc_conv_rates;
    for (const auto& account: Model_Account::instance().all())
    {
        Model_Currency::Data* currency = Model_Account::currency(account);
        acc_conv_rates[account.ACCOUNTID] = currency->BASECONVRATE;
    }
    //Temporary map
    std::map<std::pair<int /*category*/, int /*sub category*/>, double> stat;

    const auto &transactions = Model_Checking::instance().find(
            Model_Checking::TRANSDATE(date_range->start_date(), GREATER_OR_EQUAL)
            , Model_Checking::TRANSDATE(date_range->end_date(), LESS_OR_EQUAL)
            , Model_Checking::STATUS(Model_Checking::VOID_, NOT_EQUAL)
            , Model_Checking::TRANSCODE(Model_Checking::TRANSFER, NOT_EQUAL));

    for (const auto &trx : transactions)
    {
        bool withdrawal = Model_Checking::type(trx) == Model_Checking::WITHDRAWAL;
        if (Model_Checking::splittransaction(trx).empty())
        {
            std::pair<int, int> category = std::make_pair(trx.CATEGID, trx.SUBCATEGID);
            if (withdrawal)
                stat[category] -= trx.TRANSAMOUNT * (acc_conv_rates[trx.ACCOUNTID]);
            else
                stat[category] += trx.TRANSAMOUNT * (acc_conv_rates[trx.ACCOUNTID]);
        }
        else
        {
            const auto &splits = Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(trx.TRANSID));
            for (const auto& entry : splits)
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
        , [] (const std::pair<wxString, double> x, const std::pair<wxString, double> y)
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
class htmlWidgetBillsAndDeposits
{
public:
    ~htmlWidgetBillsAndDeposits();
    explicit htmlWidgetBillsAndDeposits(const wxString& title
        , mmDateRange* date_range = new mmAllTime());

    wxString getHTMLText();

protected:
    mmDateRange* date_range_;
    wxString title_;
};

htmlWidgetBillsAndDeposits::htmlWidgetBillsAndDeposits(const wxString& title, mmDateRange* date_range)
    : title_(title)
    , date_range_(date_range)
{}

htmlWidgetBillsAndDeposits::~htmlWidgetBillsAndDeposits()
{
    if (date_range_) delete date_range_;
}

wxString htmlWidgetBillsAndDeposits::getHTMLText()
{
    wxString output = ""; 

    //                    days, payee, description, amount, account
    std::vector< std::tuple<int, wxString, wxString, double, const Model_Account::Data*> > bd_days;
    for (const auto& entry : Model_Billsdeposits::instance().all(Model_Billsdeposits::COL_NEXTOCCURRENCEDATE))
    {   
        int daysRemaining = Model_Billsdeposits::instance().daysRemaining(&entry);
        if (daysRemaining > 14) 
            break; // Done searching for all to include

        int repeats = entry.REPEATS;
        // DeMultiplex the Auto Executable fields.
        if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute User Acknowlegement required
            repeats -= BD_REPEATS_MULTIPLEX_BASE;
        if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute Silent mode
            repeats -= BD_REPEATS_MULTIPLEX_BASE;

        if (daysRemaining == 0 && repeats > 10 && repeats < 15 && entry.NUMOCCURRENCES < 0) {
            continue; // Inactive
        }

        wxString daysRemainingStr = (daysRemaining > 0 
            ? wxString::Format(_("%d days remaining"), daysRemaining) 
            : wxString::Format(_("%d days overdue!"), abs(daysRemaining)));
        wxString payeeStr = "";
        if (Model_Billsdeposits::type(entry) == Model_Billsdeposits::TRANSFER)
        {   
            const Model_Account::Data *account = Model_Account::instance().get(entry.TOACCOUNTID);
            if (account) payeeStr = account->ACCOUNTNAME;
        }   
        else
        {   
            const Model_Payee::Data* payee = Model_Payee::instance().get(entry.PAYEEID);
            if (payee) payeeStr = payee->PAYEENAME;
        }   
        const auto *account = Model_Account::instance().get(entry.ACCOUNTID);
        double amount = (Model_Billsdeposits::type(entry) == Model_Billsdeposits::DEPOSIT ? entry.TRANSAMOUNT : -entry.TRANSAMOUNT);
        bd_days.push_back(std::make_tuple(daysRemaining, payeeStr, daysRemainingStr, amount, account));
    }   

    //std::sort(bd_days.begin(), bd_days.end());
    //std::reverse(bd_days.begin(), bd_days.end());
    ////////////////////////////////////

    if (!bd_days.empty())
    {   
        const wxString idStr = "BILLS_AND_DEPOSITS";

        output = "<table class='table'><thead><tr class='active'><th>";
        output += wxString::Format("<a href=\"billsdeposits:\">%s</a></th>\n<th></th>", title_);
        output += wxString::Format("<th class='text-right'>%i <a id=\"%s_label\" onclick=\"toggleTable('%s'); \" href='#'>[-]</a></th></tr>\n"
            , int(bd_days.size()), idStr, idStr);
        output += "</thead>";

        output += wxString::Format("<tbody id='%s'>", idStr);
        output += wxString::Format("<tr style='background-color: #d8ebf0'><th>%s</th><th class='text-right'>%s</th><th class='text-right'>%s</th></tr>"
            , _("Payee"), _("Amount"), _("Days"));

        for (const auto& item : bd_days)
        {
            output += wxString::Format("<tr %s>\n", std::get<0>(item) < 0 ? "class='danger'" : "");
            output += "<td>" + std::get<1>(item) +"</td>"; //payee
            output += wxString::Format("<td class='text-right'>%s</td>"
                , Model_Account::toCurrency(std::get<3>(item), std::get<4>(item)));
            output += "<td  class='text-right'>" + std::get<2>(item) + "</td></tr>\n";
        }
        output += "</tbody></table>\n";
    }
    return output;
}

////////////////////////////////////////////////////////

class WebViewHandlerHomePage : public wxWebViewHandler
{
public:
    WebViewHandlerHomePage(mmHomePagePanel *panel, const wxString& protocol)
        : wxWebViewHandler(protocol)
    {
        m_reportPanel = panel;
    }

    virtual ~WebViewHandlerHomePage()
    {
    }

    virtual wxFSFile* GetFile(const wxString &uri)
    {
        mmGUIFrame* frame = m_reportPanel->m_frame;
        wxString sData;
        if (uri.StartsWith("assets:", &sData))
        {
            frame->setNavTreeSection(_("Assets"));
            //wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_ASSETS);
            //frame->GetEventHandler()->AddPendingEvent(evt);
        }
        else if (uri.StartsWith("billsdeposits:", &sData))
        {
            frame->setNavTreeSection(_("Repeating Transactions"));
            //wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_BILLSDEPOSITS);
            //frame->GetEventHandler()->AddPendingEvent(evt);
        }
        else if (uri.StartsWith("acct:", &sData))
        {
            long id = -1;
            sData.ToLong(&id);
            const Model_Account::Data* account = Model_Account::instance().get(id);
            if (account) {
                frame->setGotoAccountID(id);
                frame->setAccountNavTreeSection(account->ACCOUNTNAME);
                //wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_GOTOACCOUNT);
                //frame->GetEventHandler()->AddPendingEvent(evt);
            }
        }
        else if (uri.StartsWith("stock:", &sData))
        {
            long id = -1;
            sData.ToLong(&id);
            const Model_Account::Data* account = Model_Account::instance().get(id);
            if (account) {
                frame->setGotoAccountID(id);
                frame->setAccountNavTreeSection(account->ACCOUNTNAME);
                //wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_STOCKS);
                //frame->GetEventHandler()->AddPendingEvent(evt);
            }
        }

        return nullptr;
    }
private:
    mmHomePagePanel *m_reportPanel;
};

BEGIN_EVENT_TABLE(mmHomePagePanel, wxPanel)
END_EVENT_TABLE()

mmHomePagePanel::mmHomePagePanel(wxWindow *parent, mmGUIFrame *frame
    , wxWindowID winid
    , const wxPoint& pos
    , const wxSize& size
    , long style
    , const wxString& name)
    : m_frame(frame)
    , countFollowUp_(0)
    , date_range_(0)
    , browser_(0)
{
    Create(parent, winid, pos, size, style, name);
    m_frame->setHomePageActive(false);
    m_frame->menuPrintingEnable(true);
}

mmHomePagePanel::~mmHomePagePanel()
{
    m_frame->setHomePageActive(false);
    m_frame->menuPrintingEnable(false);
    if (date_range_)
        delete date_range_;
}

wxString mmHomePagePanel::GetHomePageText() const
{
    return m_templateText;
}

bool mmHomePagePanel::Create(wxWindow *parent
    , wxWindowID winid
    , const wxPoint& pos
    , const wxSize& size
    , long style
    , const wxString& name)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent, winid, pos, size, style, name);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    getTemplate();
    getData();
    fillData();

    return TRUE;
}

void mmHomePagePanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    browser_ = wxWebView::New(this, wxID_ANY);
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandlerHomePage(this, "assets")));
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandlerHomePage(this, "billsdeposits")));
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandlerHomePage(this, "acct")));
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandlerHomePage(this, "stock")));
    itemBoxSizer2->Add(browser_, 1, wxGROW | wxALL, 0);
}

void mmHomePagePanel::PrintPage()
{
    browser_->Print();
}

void mmHomePagePanel::getTemplate()
{
    m_templateText.clear();
    const wxString template_path = mmex::getPathResource(mmex::HOME_PAGE_TEMPLATE);
    wxFileInputStream input(template_path);
    wxTextInputStream text(input, wxT("\x09"), wxConvUTF8);
    while (input.IsOk() && !input.Eof())
    {
        m_templateText += text.ReadLine() + "\n";
    }
}

void mmHomePagePanel::getData()
{
    vAccts_ = Model_Setting::instance().GetStringSetting("VIEWACCOUNTS", VIEW_ACCOUNTS_ALL_STR);
    if (mmIniOptions::instance().ignoreFutureTransactions_)
        date_range_ = new mmCurrentMonthToDate;
    else
        date_range_ = new mmCurrentMonth;

    double tBalance = 0.0, cardBalance = 0.0;

    std::map<int, std::pair<double, double> > accountStats;
    get_account_stats(accountStats);

    m_frames["ACCOUNTS_INFO"] = displayAccounts(tBalance, accountStats);
    m_frames["CARD_ACCOUNTS_INFO"] = displayAccounts(cardBalance, accountStats, Model_Account::CREDIT_CARD);
    tBalance += cardBalance;
    if (Model_Account::hasActiveTermAccount())
    {
        double termBalance = 0.0;
        m_frames["TERM_ACCOUNTS_INFO"] = displayAccounts(termBalance, accountStats, Model_Account::TERM);
        tBalance += termBalance;
    }

    //Stocks
    wxString stocks = "";
    htmlWidgetStocks stocks_widget;
    if (!Model_Stock::instance().all().empty())
    {
        stocks = stocks_widget.getHTMLText();
    }
    tBalance += stocks_widget.get_total();
    m_frames["STOCKS_INFO"] = stocks;

    m_frames["ASSETS_INFO"] = displayAssets(tBalance);
    m_frames["GRAND_TOTAL"] = displayGrandTotals(tBalance);

    //
    m_frames["INCOME_VS_EXPENSES"] = displayIncomeVsExpenses();

    htmlWidgetBillsAndDeposits bills_and_deposits(_("Upcoming Transactions"));
    m_frames["BILLS_AND_DEPOSITS"] = bills_and_deposits.getHTMLText();

    mmDateRange* date_range = new mmLast30Days();
    htmlWidgetTop7Categories top_trx(date_range);
    m_frames["TOP_CATEGORIES"] = top_trx.getHTMLText();

    m_frames["STATISTICS"] = getStatWidget();
    m_frames["TOGGLES"] = getToggles();

}
const wxString mmHomePagePanel::getToggles()
{
    wxString output = "<script>toggleTable('BILLS_AND_DEPOSITS'); </script>\n";
    if (!m_frame->expandedBankAccounts())
        output += "<script>toggleTable('ACCOUNTS_INFO'); </script>\n";
    if (!m_frame->expandedBankAccounts())
        output += "<script>toggleTable('CARD_ACCOUNTS_INFO'); </script>\n";
    if (!m_frame->expandedTermAccounts())
        output += "<script>toggleTable('TERM_ACCOUNTS_INFO'); </script>\n";
    if (!m_frame->expandedStockAccounts())
        output += "<script>toggleTable('INVEST'); </script>\n";
    return output;
}

void mmHomePagePanel::fillData()
{
    for (const auto& entry : m_frames)
    {
        m_templateText.Replace(wxString::Format("<TMPL_VAR \"%s\">", entry.first), entry.second);
    }
    Model_Report::outputReportFile(m_templateText);
    browser_->LoadURL(getURL(mmex::getReportIndex()));
    wxLogDebug("Loading file:%s", mmex::getReportIndex());
}

void mmHomePagePanel::get_account_stats(std::map<int, std::pair<double, double> > &accountStats)
{
    bool ignoreFuture = mmIniOptions::instance().ignoreFutureTransactions_;

    const auto &transactions = Model_Checking::instance().all();
    this->total_transactions_ = transactions.size();
    const wxDateTime today = date_range_->today();
    for (const auto& trx : transactions)
    {
        if (ignoreFuture && Model_Checking::TRANSDATE(trx).IsLaterThan(today))
            continue; //skip future dated transactions

        if (Model_Checking::status(trx) == Model_Checking::FOLLOWUP) this->countFollowUp_++;

        accountStats[trx.ACCOUNTID].first += Model_Checking::reconciled(trx, trx.ACCOUNTID);
        accountStats[trx.ACCOUNTID].second += Model_Checking::balance(trx, trx.ACCOUNTID);

        if (Model_Checking::type(trx) == Model_Checking::TRANSFER)
        {
            accountStats[trx.TOACCOUNTID].first += Model_Checking::reconciled(trx, trx.TOACCOUNTID);
            accountStats[trx.TOACCOUNTID].second += Model_Checking::balance(trx, trx.TOACCOUNTID);
        }
    }
}

void mmHomePagePanel::getExpensesIncomeStats(std::map<int, std::pair<double, double> > &incomeExpensesStats
    , mmDateRange* date_range)const
{
    //Initialization
    bool ignoreFuture = mmIniOptions::instance().ignoreFutureTransactions_;
    wxDateTime start_date = wxDateTime(date_range->end_date()).SetDay(1);

    //Calculations
    const auto &transactions = Model_Checking::instance().find(
        Model_Checking::TRANSDATE(date_range->start_date(), GREATER_OR_EQUAL)
        , Model_Checking::TRANSDATE(date_range->end_date(), LESS_OR_EQUAL)
        , Model_Checking::STATUS(Model_Checking::VOID_, NOT_EQUAL)
        , Model_Checking::TRANSCODE(Model_Checking::TRANSFER, NOT_EQUAL)
        );

    for (const auto& pBankTransaction : transactions)
    {
        if (ignoreFuture)
        {
            if (Model_Checking::TRANSDATE(pBankTransaction).IsLaterThan(date_range->today()))
                continue; //skip future dated transactions
        }

        // We got this far, get the currency conversion rate for this account
        Model_Account::Data *account = Model_Account::instance().get(pBankTransaction.ACCOUNTID);
        double convRate = (account ? Model_Account::currency(account)->BASECONVRATE : 1);

        int idx = pBankTransaction.ACCOUNTID;
        if (Model_Checking::type(pBankTransaction) == Model_Checking::DEPOSIT)
            incomeExpensesStats[idx].first += pBankTransaction.TRANSAMOUNT * convRate;
        else
            incomeExpensesStats[idx].second += pBankTransaction.TRANSAMOUNT * convRate;
    }
}

/* Accounts */
const wxString mmHomePagePanel::displayAccounts(double& tBalance, std::map<int, std::pair<double, double> > &accountStats, int type)
{
    bool type_is_bank = type == Model_Account::CHECKING || type == Model_Account::CREDIT_CARD,
         credit_card = type == Model_Account::CREDIT_CARD;
    double tReconciled = 0;

    wxString output = "<table class = 'table'>";
    output += "<thead><tr><th>";
    if (type_is_bank && !credit_card)
        output += _("Bank Account");
    else if (type_is_bank && credit_card)
        output += _("Credit Card Accounts");
    else if (!type_is_bank)
        output += _("Term account");
    output += "</th><th class = 'text-right'>" + _("Reconciled") + "</th><th class = 'text-right'>" + _("Balance") + "</th></tr></thead>";
    output += wxString::Format("<tbody id = '%s'>", (type_is_bank ? (credit_card?"CARD_ACCOUNTS_INFO":"ACCOUNTS_INFO") : "TERM_ACCOUNTS_INFO"));
    wxString body = "";
    for (const auto& account : Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
    {
        if (Model_Account::type(account) != type || Model_Account::status(account) == Model_Account::CLOSED) continue;

        Model_Currency::Data* currency = Model_Account::currency(account);
        if (!currency) currency = Model_Currency::GetBaseCurrency();
        double currency_rate = currency->BASECONVRATE;
        double bal = account.INITIALBAL + accountStats[account.ACCOUNTID].second; //Model_Account::balance(account);
        double reconciledBal = account.INITIALBAL + accountStats[account.ACCOUNTID].first;
        tBalance += bal * currency_rate;
        tReconciled += reconciledBal * currency_rate;

        // show the actual amount in that account
        if (((vAccts_ == "Open" && Model_Account::status(account) == Model_Account::OPEN) ||
            (vAccts_ == "Favorites" && Model_Account::FAVORITEACCT(account)) ||
            (vAccts_ == VIEW_ACCOUNTS_ALL_STR))
            && ((type_is_bank) ? m_frame->expandedBankAccounts() : m_frame->expandedTermAccounts()))
        {
            body += "<tr>";
            body += wxString::Format("<td><a href=\"acct:%i\">%s</a></td>", account.ACCOUNTID, account.ACCOUNTNAME);
            body += wxString::Format("<td class = \"text-right\">%s</td>", Model_Currency::toCurrency(reconciledBal, currency));
            body += wxString::Format("<td class = \"text-right\">%s</td>", Model_Currency::toCurrency(bal, currency));
            body += "</tr>\n";
        }
    }
    output += body;
    output += "</tbody><tfoot><tr class ='total'><td>" + _("Total:") + "</td>";
    output += "<td class =\"money, text-right\">" + Model_Currency::toCurrency(tReconciled) + "</td>";
    output += "<td class =\"money, text-right\">" + Model_Currency::toCurrency(tBalance) + "</td></tr></tfoot></table>";
    if (body.empty()) output.clear();

    return output;
}

//* Income vs Expenses *//
const wxString mmHomePagePanel::displayIncomeVsExpenses()
{
    double tIncome = 0.0, tExpenses = 0.0;
    std::map<int, std::pair<double, double> > incomeExpensesStats;
    getExpensesIncomeStats(incomeExpensesStats, date_range_);

    bool show_nothing = !m_frame->expandedBankAccounts() && !m_frame->expandedTermAccounts();
    bool show_all = (m_frame->expandedBankAccounts() && m_frame->expandedTermAccounts()) || show_nothing;
    bool show_bank = m_frame->expandedBankAccounts();
    for (const auto& account : Model_Account::instance().all())
    {
        if (!show_all)
        {
            if (show_bank && Model_Account::type(account) != Model_Account::CHECKING && Model_Account::type(account) != Model_Account::CREDIT_CARD) continue;
            if (m_frame->expandedTermAccounts() && Model_Account::type(account) == Model_Account::TERM) continue;
        }
        int idx = account.ACCOUNTID;
        tIncome += incomeExpensesStats[idx].first;
        tExpenses += incomeExpensesStats[idx].second;
    }
    // Compute chart spacing and interval (chart forced to start at zero)
    double steps = 10;
    double stepWidth = ceil(std::max(tIncome,tExpenses)*1.1/steps);
    //Type, Amount, Income, Expences, Difference:, Income/Expences, income, expemces
    static const wxString INCOME_VS_EXPENCES_HTML =
        "<table class = 'table'>\n"
        "<thead><tr class='active'><th>%s</th><th></th></tr></thead>"
        "<tbody>\n"
        "    <tr valign=\"center\">\n"
        "        <td><canvas id=\"reportChart\" width=\"312\" height=\"256\"></canvas></td>\n"
        "        <td  style='vertical-align:middle'>\n"
        "            <table class= 'table'>\n"
        "            <thead>\n"
        "                <tr>"
        "                    <th>%s</th>"
        "                    <th class = \"text-right\">%s</th>"
        "                </tr>"
        "            </thead>\n"
        "            <tbody>"
        "                <tr>"
        "                    <td>%s</td>"
        "                    <td class = \"text-right\">%s</td>"
        "                </tr>"
        "                <tr>"
        "                    <td>%s</td>"
        "                    <td class = \"text-right\">%s</td>"
        "                </tr>"
        "            </tbody>\n"
        "            <tfoot>"
        "                <tr class=\"total\">"
        "                    <td>%s</td>"
        "                    <td class = \"text-right\">%s</td>"
        "                </tr>"
        "            </tfoot>"
        "            </table>"
        "        </td>"
        "        </tr>"
        "</tbody>"
        "</table>\n"
        "\n"
        "<script>\n"
        "    <!-- Chart -->\n"
        "    var data = {\n"
        "    labels : [\"%s\"],\n"
        "    datasets : [\n"
        "        {\n"
        "            fillColor : \"rgba(151,187,205,0.5)\",\n"
        "            strokeColor : \"rgba(151,187,205,1)\",\n"
        "            data : [%f],\n"
        "        },\n"
        "        {\n"
        "            fillColor : \"rgba(220,66,66,0.5)\",\n"
        "            strokeColor : \"rgba(220,220,220,1)\",\n"
        "            data : [%f],\n"
        "        },\n"
        "    ]\n"
        "    };\n"
        "    var options = {\n"
        "        animationEasing: \"easeOutQuint\",\n"
        "        barValueSpacing : 10,\n"
        "        scaleOverride: true,\n"
        "        scaleStartValue: 0,\n"
        "        scaleSteps: [%f],\n"
        "        scaleStepWidth: [%f]\n"
        "    };\n"
        "    var ctx = document.getElementById(\"reportChart\").getContext(\"2d\");\n"
        "    var reportChart = new Chart(ctx).Bar(data, options);\n"

        "</script>\n";
    wxString output = wxString::Format(INCOME_VS_EXPENCES_HTML
        , wxString::Format(_("Income vs Expenses: %s"), date_range_->title())
        , _("Type"), _("Amount")
        , _("Income"), Model_Currency::toCurrency(tIncome)
        , _("Expences"), Model_Currency::toCurrency(tExpenses)
        , _("Difference:"), Model_Currency::toCurrency(tIncome - tExpenses)
        , _("Income/Expences")
        , tIncome, tExpenses, steps, stepWidth);
    return output;
}

//* Assets *//
const wxString mmHomePagePanel::displayAssets(double& tBalance)
{
    wxString output = "";

    double asset_balance = Model_Asset::instance().balance();
    tBalance += asset_balance;
    output = "<table class = 'table'><tfoot><tr class = \"total\">";
    output += wxString::Format("<td><a href = \"Assets:\">%s</a></td>", _("Assets"));
    output += wxString::Format("<td class = \"text-right\">%s</td></tr>", Model_Currency::toCurrency(asset_balance));
    output += "</tfoot></table>";

    return output;
}

const wxString mmHomePagePanel::getStatWidget()
{
    wxString output = "<table class = 'table'><thead><tr class = 'active'>";
    output += "<th>" + _("Transaction Statistics") + "</th><th></th><tbody>";

    if (this->countFollowUp_ > 0)
    {
        output += "<tr><td>";
        output += _("Follow Up On Transactions: ") + "</td>";
        output += wxString::Format("<td class = 'text-right'>%i</td></tr>", this->countFollowUp_);
    }

    output += "<tr><td>";
    output += _("Total Transactions: ") + "</td>";
    output += wxString::Format("<td class = 'text-right'>%d</td></tr></table>", this->total_transactions_);

    return output;
}

const wxString mmHomePagePanel::displayGrandTotals(double& tBalance)
{
    wxString output = "<table class ='table'>";
    //  Display the grand total from all sections
    wxString tBalanceStr = Model_Currency::toCurrency(tBalance);

    output += "<tfoot><tr class ='success' style ='font-weight:bold'><td>" + _("Grand Total:") + "</td>";
    output += "<td class ='text-right'>" + tBalanceStr + "</td>";
    output += "<td class='text-right'>";
    output += wxString::Format("<a id='%s_label' onclick=\"toggleTable('%s'); \" href='#'>[-]</a>"
        , "ACCOUNTS_INFO", "ACCOUNTS_INFO");
    if (Model_Account::hasActiveTermAccount())
        output += wxString::Format("<a id=\"%s_label\" onclick=\"toggleTable('%s'); \" href='#'>[-]</a>"
        , "TERM_ACCOUNTS_INFO", "TERM_ACCOUNTS_INFO");
    output += wxString::Format("<a id=\"%s_label\" onclick=\"toggleTable('%s'); \" href='#'>[-]</a>"
        , "INVEST", "INVEST");
    output += "</td>\n";
    output += "</tfoot></table>";

    return output;
}

const bool mmHomePagePanel::getNewsRSS()
{
    wxString RssContentWX;
    if (site_content(mmex::getProgramWebSiteRSS(), RssContentWX) != wxURL_NOERR)
        return false;
    const char * RssContent = RssContentWX.mb_str();

    tinyxml2::XMLDocument RssDocument;
    if (RssDocument.Parse((const char*)RssContent) != tinyxml2::XML_NO_ERROR)
        return false;

    tinyxml2::XMLHandle RssDocumentHandle(&RssDocument);
    tinyxml2::XMLElement* RssElement;
    tinyxml2::XMLHandle RssDocumentRootHandle(0);

    RssElement = RssDocumentHandle.FirstChildElement("rss").FirstChildElement("channel").ToElement();
    if (!RssElement)
        return false;

    std::vector<WebsiteNews> WebisteNewsList;
    RssDocumentRootHandle = tinyxml2::XMLHandle(RssElement);
    RssElement = RssDocumentRootHandle.FirstChildElement("item").ToElement();
    for (RssElement; RssElement; RssElement = RssElement->NextSiblingElement())
    {
        WebsiteNews website_news;
        website_news.Date = RssElement->FirstChildElement("pubDate")->GetText();
        website_news.Title = RssElement->FirstChildElement("title")->GetText();
        website_news.Link = RssElement->FirstChildElement("link")->GetText();
        website_news.Description = RssElement->FirstChildElement("description")->GetText();
        WebisteNewsList.push_back(website_news);
    }
    return true;
}