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
#include <algorithm>
#include <cmath>

#include "constants.h"
#include "option.h"
#include "util.h"

#include "model/allmodel.h"

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
        output = "<table class ='sortable table'><col style='width: 50%'><col style='width: 25%'><col style='width: 25%'><thead><tr class='active'><th>\n";
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
            body += wxString::Format("<td sorttable_customkey='*%s*'><a href='stock:%i' oncontextmenu='return false;'>%s</a></td>\n"
                , account.ACCOUNTNAME, account.ACCOUNTID, account.ACCOUNTNAME);
            body += wxString::Format("<td class='money' sorttable_customkey='%f'>%s</td>\n"
                , stockStats[account.ACCOUNTID].first
                , Model_Account::toCurrency(stockStats[account.ACCOUNTID].first, &account));
            body += wxString::Format("<td colspan='2' class='money' sorttable_customkey='%f'>%s</td>"
                , stockStats[account.ACCOUNTID].second
                , Model_Account::toCurrency(stockStats[account.ACCOUNTID].second, &account));
            body += "</tr>";
        }

        output += body;
        output += "</tbody><tfoot><tr class = 'total'><td>" + _("Total:") + "</td>";
        output += wxString::Format("<td class='money'>%s</td>"
            , Model_Currency::toCurrency(grand_gain_lost_));
        output += wxString::Format("<td colspan='2' class='money'>%s</td></tr></tfoot></table>"
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
        double gain_lost = (current_value - stock.VALUE - stock.COMMISSION);
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
    title_ = wxString::Format(_("Top Withdrawals: %s"), date_range_->local_title());
}

htmlWidgetTop7Categories::~htmlWidgetTop7Categories()
{
    if (date_range_) delete date_range_;
}

wxString htmlWidgetTop7Categories::getHTMLText()
{
    std::vector<std::pair<wxString, double> > topCategoryStats;
    getTopCategoryStats(topCategoryStats, date_range_);
    wxString output = "", data;
    
    if (!topCategoryStats.empty()) 
    {
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
    for (const auto& account: Model_Account::instance().all())
    {
        acc_conv_rates[account.ACCOUNTID] = Model_CurrencyHistory::getDayRate(account.CURRENCYID, today);
    }
    //Temporary map
    std::map<std::pair<int /*category*/, int /*sub category*/>, double> stat;

    const auto splits = Model_Splittransaction::instance().get_all();
    const auto &transactions = Model_Checking::instance().find(
            Model_Checking::TRANSDATE(date_range->start_date(), GREATER_OR_EQUAL)
            , Model_Checking::TRANSDATE(date_range->end_date(), LESS_OR_EQUAL)
            , Model_Checking::STATUS(Model_Checking::VOID_, NOT_EQUAL)
            , Model_Checking::TRANSCODE(Model_Checking::TRANSFER, NOT_EQUAL));

    for (const auto &trx : transactions)
    {
        bool withdrawal = Model_Checking::type(trx) == Model_Checking::WITHDRAWAL;
        const auto it = splits.find(trx.TRANSID);

        if (it == splits.end())
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
    wxDate today = wxDate::Today();

    //                    days, payee, description, amount, account
    std::vector< std::tuple<int, wxString, wxString, double, const Model_Account::Data*> > bd_days;
    for (const auto& entry : Model_Billsdeposits::instance().all(Model_Billsdeposits::COL_NEXTOCCURRENCEDATE))
    {   
        int daysPayment = Model_Billsdeposits::NEXTOCCURRENCEDATE(&entry)
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

        int daysOverdue = Model_Billsdeposits::TRANSDATE(&entry)
            .Subtract(today).GetDays();
        wxString daysRemainingStr = (daysPayment > 0
            ? wxString::Format(wxPLURAL("%d day remaining", "%d days remaining", daysPayment), daysPayment)
            : wxString::Format(wxPLURAL("%d day delay!", "%d days delay!", -daysPayment), -daysPayment));
        if (daysOverdue < 0)
            daysRemainingStr = wxString::Format(wxPLURAL("%d day overdue!", "%d days overdue!", std::abs(daysOverdue)), std::abs(daysOverdue));

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
        bd_days.push_back(std::make_tuple(daysPayment, payeeStr, daysRemainingStr, amount, account));
    }   

    //std::sort(bd_days.begin(), bd_days.end());
    //std::reverse(bd_days.begin(), bd_days.end());
    ////////////////////////////////////

    if (!bd_days.empty())
    {   
        static const wxString idStr = "BILLS_AND_DEPOSITS";

        output = "<table class='table'>\n<thead>\n<tr class='active'><th>";
        output += wxString::Format("<a href=\"billsdeposits:\" oncontextmenu='return false;'>%s</a></th>\n<th></th>\n", title_);
        output += wxString::Format("<th nowrap class='text-right sorttable_nosort'>%i <a id='%s_label' onclick=\"toggleTable('%s'); \" href='#%s' oncontextmenu='return false;'>[-]</a></th></tr>\n"
            , int(bd_days.size()), idStr, idStr, idStr);
        output += "</thead>\n";

        output += wxString::Format("<tbody id='%s'>\n", idStr);
        output += wxString::Format("<tr style='background-color: #d8ebf0'><th>%s</th>\n<th class='text-right'>%s</th>\n<th class='text-right'>%s</th></tr>\n"
            , _("Payee"), _("Amount"), _("Payment"));

        for (const auto& item : bd_days)
        {
            output += wxString::Format("<tr %s>\n", std::get<0>(item) < 0 ? "class='danger'" : "");
            output += "<td>" + std::get<1>(item) +"</td>"; //payee
            output += wxString::Format("<td class='money'>%s</td>\n"
                , Model_Account::toCurrency(std::get<3>(item), std::get<4>(item)));
            output += "<td  class='money'>" + std::get<2>(item) + "</td></tr>\n";
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
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_ASSETS);
            frame->GetEventHandler()->AddPendingEvent(evt);
        }
        else if (uri.StartsWith("billsdeposits:", &sData))
        {
            frame->setNavTreeSection(_("Recurring Transactions"));
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_BILLSDEPOSITS);
            frame->GetEventHandler()->AddPendingEvent(evt);
        }
        else if (uri.StartsWith("acct:", &sData))
        {
            long id = -1;
            sData.ToLong(&id);
            const Model_Account::Data* account = Model_Account::instance().get(id);
            if (account) {
                frame->setGotoAccountID(id);
                frame->setAccountNavTreeSection(account->ACCOUNTNAME);
                wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_GOTOACCOUNT);
                frame->GetEventHandler()->AddPendingEvent(evt);
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
                wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_STOCKS);
                frame->GetEventHandler()->AddPendingEvent(evt);
            }
        }

        return nullptr;
    }
private:
    mmHomePagePanel *m_reportPanel;
};

wxBEGIN_EVENT_TABLE(mmHomePagePanel, wxPanel)
EVT_WEBVIEW_NAVIGATING(wxID_ANY, mmHomePagePanel::OnLinkClicked)
wxEND_EVENT_TABLE()

mmHomePagePanel::mmHomePagePanel(wxWindow *parent, mmGUIFrame *frame
    , wxWindowID winid
    , const wxPoint& pos
    , const wxSize& size
    , long style
    , const wxString& name)
    : m_frame(frame)
    , countFollowUp_(0)
    , date_range_(nullptr)
    , browser_(nullptr)
{
    Create(parent, winid, pos, size, style, name);
    m_frame->menuPrintingEnable(true);
}

mmHomePagePanel::~mmHomePagePanel()
{
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
    wxPanelBase::Create(parent, winid, pos, size, style, name);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    createHTML();

    Model_Usage::instance().pageview(this);

    return TRUE;
}

void  mmHomePagePanel::createHTML()
{
    getTemplate();
    getData();
    fillData();
}

void mmHomePagePanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    browser_ = wxWebView::New(this, mmID_BROWSER);
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
    wxTextInputStream text(input, "\x09", wxConvUTF8);
    while (input.IsOk() && !input.Eof())
    {
        m_templateText += text.ReadLine() + "\n";
    }
}

void mmHomePagePanel::getData()
{
    m_frames["HTMLSCALE"] = wxString::Format("%d", Option::instance().HtmlFontSize());

    vAccts_ = Model_Setting::instance().ViewAccounts();
    
    if (date_range_)
        date_range_->destroy();
 
    if (Option::instance().getIgnoreFutureTransactions())
        date_range_ = new mmCurrentMonthToDate;
    else
        date_range_ = new mmCurrentMonth;

    double tBalance = 0.0, cardBalance = 0.0, termBalance = 0.0, cashBalance = 0.0, loanBalance = 0.0;

    std::map<int, std::pair<double, double> > accountStats;
    get_account_stats(accountStats);

    m_frames["ACCOUNTS_INFO"] = displayAccounts(tBalance, accountStats);
    m_frames["CARD_ACCOUNTS_INFO"] = displayAccounts(cardBalance, accountStats, Model_Account::CREDIT_CARD);
    tBalance += cardBalance;

    m_frames["CASH_ACCOUNTS_INFO"] = displayAccounts(cashBalance, accountStats, Model_Account::CASH);
    tBalance += cashBalance;

    m_frames["LOAN_ACCOUNTS_INFO"] = displayAccounts(loanBalance, accountStats, Model_Account::LOAN);
    tBalance += loanBalance;

    m_frames["TERM_ACCOUNTS_INFO"] = displayAccounts(termBalance, accountStats, Model_Account::TERM);
    tBalance += termBalance;

    //Stocks
    htmlWidgetStocks stocks_widget;
    m_frames["STOCKS_INFO"] = stocks_widget.getHTMLText();
    tBalance += stocks_widget.get_total();

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
    const wxString json = Model_Infotable::instance().GetStringInfo("HOME_PAGE_STATUS", "{}");
    return json;
}

void mmHomePagePanel::fillData()
{
	for (const auto& entry : m_frames)
	{
		m_templateText.Replace(wxString::Format("<TMPL_VAR %s>", entry.first), entry.second);
	}
	Model_Report::outputReportFile(m_templateText, "index");
	browser_->LoadURL(getURL(mmex::getReportFullFileName("index")));
}

void mmHomePagePanel::get_account_stats(std::map<int, std::pair<double, double> > &accountStats)
{
    Model_Checking::Data_Set all_trans;
    if (Option::instance().getIgnoreFutureTransactions())
    {
        all_trans = Model_Checking::instance().find(
            DB_Table_CHECKINGACCOUNT_V1::TRANSDATE(date_range_->today().FormatISODate(), LESS_OR_EQUAL));
    }
    else
    {
        all_trans = Model_Checking::instance().all();
    }

    this->total_transactions_ = all_trans.size();

    for (const auto& trx : all_trans)
    {
        // Do not include asset or stock transfers in income expense calculations.
        if (Model_Checking::foreignTransactionAsTransfer(trx))
            continue;

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
    bool ignoreFuture = Option::instance().getIgnoreFutureTransactions();
    wxDateTime start_date = date_range->end_date();
    start_date = start_date.SetDay(1);

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
        
        // Do not include asset or stock transfers in income expense calculations.
        if (Model_Checking::foreignTransactionAsTransfer(pBankTransaction))
            continue;

        // We got this far, get the currency conversion rate for this account
        double convRate = Model_CurrencyHistory::getDayRate(Model_Account::instance().get(pBankTransaction.ACCOUNTID)->CURRENCYID, pBankTransaction.TRANSDATE);

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
    static const std::vector < std::pair <wxString, wxString> > typeStr
    {
        { "CASH_ACCOUNTS_INFO", _("Cash Accounts") },
        { "ACCOUNTS_INFO", _("Bank Accounts") },
        { "CARD_ACCOUNTS_INFO", _("Credit Card Accounts") },
        { "LOAN_ACCOUNTS_INFO", _("Loan Accounts") },
        { "TERM_ACCOUNTS_INFO", _("Term Accounts") },
    };

    const wxString idStr = typeStr[type].first;
    wxString output = "<table class = 'sortable table'>\n";
    output += "<col style=\"width:50%\"><col style=\"width:25%\"><col style=\"width:25%\">\n";
    output += "<thead><tr><th nowrap>";
    output += typeStr[type].second;

    output += "</th><th class = 'text-right'>" + _("Reconciled") + "</th>\n";
    output += "<th class = 'text-right'>" + _("Balance") + "</th>\n";
    output += wxString::Format("<th nowrap class='text-right sorttable_nosort'><a id='%s_label' onclick=\"toggleTable('%s'); \" href='#%s' oncontextmenu='return false;'>[-]</a></th>\n"
        , idStr, idStr, idStr);
    output += "</tr></thead>\n";
    output += wxString::Format("<tbody id = '%s'>\n", idStr);

    double tReconciled = 0;
    wxString body = "";
    const wxDate today = wxDate::Today();
    for (const auto& account : Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
    {
        if (Model_Account::type(account) != type || Model_Account::status(account) == Model_Account::CLOSED) continue;

        Model_Currency::Data* currency = Model_Account::currency(account);

        double currency_rate = Model_CurrencyHistory::getDayRate(account.CURRENCYID, today);
        double bal = account.INITIALBAL + accountStats[account.ACCOUNTID].second; //Model_Account::balance(account);
        double reconciledBal = account.INITIALBAL + accountStats[account.ACCOUNTID].first;
        tBalance += bal * currency_rate;
        tReconciled += reconciledBal * currency_rate;

        // show the actual amount in that account
        if (((vAccts_ == VIEW_ACCOUNTS_OPEN_STR && Model_Account::status(account) == Model_Account::OPEN) ||
            (vAccts_ == VIEW_ACCOUNTS_FAVORITES_STR && Model_Account::FAVORITEACCT(account)) ||
            (vAccts_ == VIEW_ACCOUNTS_ALL_STR)))
        {
            body += "<tr>";
            body += wxString::Format("<td sorttable_customkey='*%s*' nowrap><a href='acct:%i' oncontextmenu='return false;'>%s</a></td>\n"
                , account.ACCOUNTNAME, account.ACCOUNTID, account.ACCOUNTNAME);
            body += wxString::Format("<td class='money' sorttable_customkey='%f' nowrap>%s</td>\n", reconciledBal, Model_Currency::toCurrency(reconciledBal, currency));
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

//* Income vs Expenses *//
const wxString mmHomePagePanel::displayIncomeVsExpenses()
{
	double tIncome = 0.0, tExpenses = 0.0;
	std::map<int, std::pair<double, double> > incomeExpensesStats;
	setExpensesIncomeStatsData(incomeExpensesStats, date_range_);

	for (const auto& account : Model_Account::instance().all())
	{
		int idx = account.ACCOUNTID;
		tIncome += incomeExpensesStats[idx].first;
		tExpenses += incomeExpensesStats[idx].second;
	}
	// Compute chart spacing and interval (chart forced to start at zero)
	double steps = 10.0;
	double scaleStepWidth = ceil(std::max(tIncome, tExpenses) / steps);
	if (scaleStepWidth <= 1.0)
		scaleStepWidth = 1.0;
	else {
		double s = (pow(10, ceil(log10(scaleStepWidth)) - 1.0));
		if (s > 0) scaleStepWidth = ceil(scaleStepWidth / s)*s;
	}

	StringBuffer json_buffer;
	PrettyWriter<StringBuffer> json_writer(json_buffer);
	json_writer.StartObject();
	json_writer.Key("0");
	json_writer.String(wxString::Format(_("Income vs Expenses: %s"), date_range_->local_title()).c_str());
	json_writer.Key("1");
	json_writer.String(_("Type").c_str());
	json_writer.Key("2");
	json_writer.String(_("Amount").c_str());
	json_writer.Key("3");
	json_writer.String(_("Income").c_str());
	json_writer.Key("4");
	json_writer.String(Model_Currency::toCurrency(tIncome).c_str());
	json_writer.Key("5");
	json_writer.String(_("Expenses").c_str());
	json_writer.Key("6");
	json_writer.String(Model_Currency::toCurrency(tExpenses).c_str());
	json_writer.Key("7");
	json_writer.String(_("Difference:").c_str());
	json_writer.Key("8");
	json_writer.String(Model_Currency::toCurrency(tIncome - tExpenses).c_str());
	json_writer.Key("9");
	json_writer.String(_("Income/Expenses").c_str());
	json_writer.Key("10");
	json_writer.String(wxString::FromCDouble(tIncome, 2).c_str());
	json_writer.Key("11");
	json_writer.String(wxString::FromCDouble(tExpenses, 2).c_str());
	json_writer.Key("12");
	json_writer.Int(steps);
	json_writer.Key("13");
	json_writer.Int(scaleStepWidth);
	json_writer.EndObject();

	wxLogDebug("======= mmHomePagePanel::getIncomeVsExpensesJSON =======");
	wxLogDebug("RapidJson\n%s", json_buffer.GetString());

	return json_buffer.GetString();
}

void mmHomePagePanel::setExpensesIncomeStatsData(std::map<int, std::pair<double, double> > &incomeExpensesStats
	, mmDateRange* date_range) const
{
	//Initialization
	bool ignoreFuture = Option::instance().getIgnoreFutureTransactions();

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
}

//* Assets *//
const wxString mmHomePagePanel::displayAssets(double& tBalance)
{
	double asset_balance = Model_Asset::instance().balance();
	tBalance += asset_balance;

	StringBuffer json_buffer;
	PrettyWriter<StringBuffer> json_writer(json_buffer);
	json_writer.StartObject();
	json_writer.Key("NAME");
	json_writer.String(_("Assets").c_str());
	json_writer.Key("VALUE");
	json_writer.String(Model_Currency::toCurrency(asset_balance).c_str());
	json_writer.EndObject();

	wxLogDebug("======= mmHomePagePanel::getAssetsJSON =======");
	wxLogDebug("RapidJson\n%s", json_buffer.GetString());

	return json_buffer.GetString();
}

const wxString mmHomePagePanel::getStatWidget()
{
	StringBuffer json_buffer;
	PrettyWriter<StringBuffer> json_writer(json_buffer);
	json_writer.StartObject();

	json_writer.Key("NAME");
	json_writer.String(_("Transaction Statistics").c_str());

	if (this->countFollowUp_ > 0)
	{
		json_writer.Key(_("Follow Up On Transactions: ").c_str());
		json_writer.Double(this->countFollowUp_);
	}

	json_writer.Key(_("Total Transactions: ").c_str());
	json_writer.Int(this->total_transactions_);
	json_writer.EndObject();

	wxLogDebug("======= mmHomePagePanel::getStatWidget =======");
	wxLogDebug("RapidJson\n%s", json_buffer.GetString());

	return json_buffer.GetString();
}

const wxString mmHomePagePanel::displayGrandTotals(double& tBalance)
{
	const wxString tBalanceStr = Model_Currency::toCurrency(tBalance);

	StringBuffer json_buffer;
	PrettyWriter<StringBuffer> json_writer(json_buffer);
	json_writer.StartObject();
	json_writer.Key("NAME");
	json_writer.String(_("Grand Total:").c_str());
	json_writer.Key("VALUE");
	json_writer.String(tBalanceStr.c_str());
	json_writer.EndObject();

	wxLogDebug("======= mmHomePagePanel::getGrandTotalsJSON =======");
	wxLogDebug("RapidJson\n%s", json_buffer.GetString());

	return json_buffer.GetString();
}

void mmHomePagePanel::OnLinkClicked(wxWebViewEvent& event)
{
	const wxString& url = event.GetURL();

	if (url.Contains("#"))
	{
		wxString name = url.AfterLast('#');

		//Convert the JSON string from database to a json object
		wxString str = Model_Infotable::instance().GetStringInfo("HOME_PAGE_STATUS", "{}");

		wxLogDebug("======= mmHomePagePanel::OnLinkClicked =======");
		wxLogDebug("Name = %s", name);

		Document json_doc;
		if (json_doc.Parse(str.c_str()).HasParseError())
			return;

		Document::AllocatorType& json_allocator = json_doc.GetAllocator();
		wxLogDebug("RapidJson Input\n%s", JSON_PrettyFormated(json_doc));

        const wxString type[] = { "TOP_CATEGORIES", "INVEST", "ACCOUNTS_INFO", "CARD_ACCOUNTS_INFO", "CASH_ACCOUNTS_INFO", "LOAN_ACCOUNTS_INFO", "TERM_ACCOUNTS_INFO" };

		for (const auto& entry : type)
		{
			if (name != entry) continue;

			Value v_type(entry.c_str(), json_allocator);
			if (json_doc.HasMember(v_type) && json_doc[v_type].IsBool())
			{
				json_doc[v_type] = !json_doc[v_type].GetBool();
			}
			else
			{
				json_doc.AddMember(v_type, true, json_allocator);
			}
		}

		wxLogDebug("Saving updated RapidJson\n%s", JSON_PrettyFormated(json_doc));
		wxLogDebug("======= mmHomePagePanel::OnLinkClicked =======");

		Model_Infotable::instance().Set("HOME_PAGE_STATUS", JSON_PrettyFormated(json_doc));
	}
}
