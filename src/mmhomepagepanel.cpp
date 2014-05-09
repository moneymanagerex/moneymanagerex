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
#include "reports/html_widget_top_categories.h"
#include "reports/html_widget_bills_and_deposits.h"
#include "reports/html_widget_stocks.h"
#include "mmex.h"
#include "mmframe.h"
#include "paths.h"

#include "html_template.h"
#include "billsdepositspanel.h"
#include "reports/mmgraphincexpensesmonth.h"
#include <algorithm>

#include "model/Model_Setting.h"
#include "model/Model_Asset.h"
#include "model/Model_Payee.h"
#include "model/Model_Stock.h"
#include "model/Model_Billsdeposits.h"

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
        mmGUIFrame* frame = wxGetApp().m_frame;
        wxString sData;
        if (uri.Upper().StartsWith("ASSETS", &sData))
        {
            frame->setNavTreeSection(_("Assets"));
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_ASSETS);
            frame->GetEventHandler()->AddPendingEvent(evt);
        }
        else if (uri.Upper().StartsWith("BILLSDEPOSITS", &sData))
        {
            frame->setNavTreeSection(_("Repeating Transactions"));
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_BILLSDEPOSITS);
            frame->GetEventHandler()->AddPendingEvent(evt);
        }
        else if (uri.Upper().StartsWith("ACCT:", &sData))
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
        else if (uri.Upper().StartsWith("STOCK:", &sData))
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

BEGIN_EVENT_TABLE(mmHomePagePanel, wxPanel)
END_EVENT_TABLE()

mmHomePagePanel::mmHomePagePanel(wxWindow *parent
    , wxWindowID winid
    , const wxPoint& pos
    , const wxSize& size
    , long style
    , const wxString& name)
    : frame_(wxGetApp().m_frame)
    , countFollowUp_(0)
    , date_range_(0)
    , browser_(0)
{
    Create(parent, winid, pos, size, style, name);
    frame_->setHomePageActive();
    frame_->menuPrintingEnable(true);
}

mmHomePagePanel::~mmHomePagePanel()
{
    frame_->setHomePageActive(false);
    frame_->menuPrintingEnable(false);
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
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandlerHomePage(this, "Assets")));
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandlerHomePage(this, "billsdeposits")));
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandlerHomePage(this, "ACCT")));
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandlerHomePage(this, "STOCK")));
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

    double tBalance = 0.0;

    std::map<int, std::pair<double, double> > accountStats;
    get_account_stats(accountStats);

    m_frames["ACCOUNTS_INFO"] = displayAccounts(tBalance, accountStats);
    if (Model_Account::hasActiveTermAccount())
    {
        double termBalance = 0.0;
        m_frames["TERM_ACCOUNTS_INFO"] = displayAccounts(termBalance, accountStats, Model_Account::TERM);
        tBalance += termBalance;
    }

    //Stocks
    wxString stocks = "";
    htmlWidgetStocks stocks_widget;
    stocks_widget.enable_detailes(frame_->expandedStockAccounts());
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
    bool type_is_bank = type == Model_Account::CHECKING;
    double tReconciled = 0;

    wxString output = wxString::Format("<table class = \"table\" id = \"%s\">", (type_is_bank ? "ACCOUNTS_INFO" : "TERM_ACCOUNTS_INFO"));
    output += "<thead><tr><th>";
    output += (type_is_bank ? _("Bank Account") : _("Term Account"));
    output += "</th><th class = 'text-right'>" + _("Reconciled") + "</th><th class = 'text-right'>" + _("Balance") + "</th></tr></thead>";
    output += wxString::Format("<tbody id = \"%s\">", "");
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

        // Display the individual account links if we want to display them
        if (((type_is_bank) ? frame_->expandedBankAccounts() : frame_->expandedTermAccounts())
            || (!frame_->expandedBankAccounts() && !frame_->expandedTermAccounts()))
        {

            // show the actual amount in that account
            if (((vAccts_ == "Open" && Model_Account::status(account) == Model_Account::OPEN) ||
                (vAccts_ == "Favorites" && Model_Account::FAVORITEACCT(account)) ||
                (vAccts_ == VIEW_ACCOUNTS_ALL_STR))
                && ((type_is_bank) ? frame_->expandedBankAccounts() : frame_->expandedTermAccounts()))
            {
                body += "<tr>";
                body += wxString::Format("<td><a href=\"ACCT:%d\">%s</a></td>", account.ACCOUNTID, account.ACCOUNTNAME);
                body += wxString::Format("<td class = \"text-right\">%s</td>", Model_Currency::toCurrency(reconciledBal, currency));
                body += wxString::Format("<td class = \"text-right\">%s</td>", Model_Currency::toCurrency(bal, currency));
                body += "</tr>\n";
            }
        }
    }
    output += body;
    output += "</tbody><tfoot><tr class = \"total\"><td>" + _("Total:") + "</td>";
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

    bool show_nothing = !frame_->expandedBankAccounts() && !frame_->expandedTermAccounts();
    bool show_all = (frame_->expandedBankAccounts() && frame_->expandedTermAccounts()) || show_nothing;
    bool show_bank = frame_->expandedBankAccounts();
    for (const auto& account : Model_Account::instance().all())
    {
        if (!show_all)
        {
            if (show_bank && Model_Account::type(account) != Model_Account::CHECKING) continue;
            if (frame_->expandedTermAccounts() && Model_Account::type(account) == Model_Account::TERM) continue;
        }
        int idx = account.ACCOUNTID;
        tIncome += incomeExpensesStats[idx].first;
        tExpenses += incomeExpensesStats[idx].second;
    }

    //Type, Amount, Income, Expences, Difference:, Income/Expences, income, expemces
    static const wxString INCOME_VS_EXPENCES_HTML =
        "<table class = \"table\">\n"
        "<tbody>\n"
        "    <tr valign=\"center\">\n"
        "        <td><canvas id=\"reportChart\" width=\"312\" height=\"256\"></canvas></td>\n"
        "        <td>\n"
        "            <table class= \"table\">\n"
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
        "    };\n"
        "    var ctx = document.getElementById(\"reportChart\").getContext(\"2d\");\n"
        "    var reportChart = new Chart(ctx).Bar(data, options);\n"

        "</script>\n";
    wxString output = wxString::Format(INCOME_VS_EXPENCES_HTML
        , _("Type"), _("Amount")
        , _("Income"), Model_Currency::toCurrency(tIncome)
        , _("Expences"), Model_Currency::toCurrency(tExpenses)
        , _("Difference:"), Model_Currency::toCurrency(tIncome - tExpenses)
        , _("Income/Expences")
        , tIncome, tExpenses);
    return output;
}

//* Assets *//
const wxString mmHomePagePanel::displayAssets(double& tBalance)
{
    wxString output = "";

    if (mmIniOptions::instance().enableAssets_)
    {
        double asset_balance = Model_Asset::instance().balance();
        tBalance += asset_balance;
        output = "<table class = \"table\"><tfoot><tr class = \"total\">";
        output += wxString::Format("<td><a href = \"Assets:\">%s</a></td>", _("Assets"));
        output += wxString::Format("<td class = \"text-right\">%s</td></tr>", Model_Currency::toCurrency(asset_balance));
        output += "</tfoot></table>";
    }
    return output;
}

const wxString mmHomePagePanel::getStatWidget()
{
    wxString output = "<table class = \"table\"><thead><tr>";
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
    wxString output = "<table class = \"table\">";
    //  Display the grand total from all sections
    wxString tBalanceStr = Model_Currency::toCurrency(tBalance);

    output += "<tfoot><tr class = \"success\" style = \"font-weight:bold\"><td>" + _("Grand Total:") + "</td>";
    output += "<td class ='text-right'>" + tBalanceStr + "</td>";
    output += "<td class ='text-right'>[-] [-] [-]</td>";
    output += "</tfoot></table>";

    return output;
}
