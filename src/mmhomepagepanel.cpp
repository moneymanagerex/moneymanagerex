/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#include "reports/htmlbuilder.h"
#include "billsdepositspanel.h"
#include "reports/mmgraphincexpensesmonth.h"
#include <algorithm>

#include "model/Model_Setting.h"
#include "model/Model_Asset.h"
#include "model/Model_Payee.h"
#include "model/Model_Billsdeposits.h"


BEGIN_EVENT_TABLE( mmHomePagePanel, wxPanel )
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(mmHtmlWindow, wxHtmlWindow)
END_EVENT_TABLE()

mmHomePagePanel::mmHomePagePanel(wxWindow *parent,
            wxWindowID winid,
            const wxPoint& pos,
            const wxSize& size,
            long style,
            const wxString& name )
        : frame_(wxGetApp().m_frame)
        , countFollowUp_(0)
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

wxString mmHomePagePanel::GetHomePageText()
{
    return html_text_;
}

bool mmHomePagePanel::Create( wxWindow *parent,
            wxWindowID winid,
            const wxPoint& pos,
            const wxSize& size,
            long style,
            const wxString& name  )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent, winid, pos, size, style, name);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    createFrames();

    return TRUE;
}

void mmHomePagePanel::createFrames()
{

    if (mmIniOptions::instance().ignoreFutureTransactions_)
        date_range_ = new mmCurrentMonthToDate;
    else
        date_range_ = new mmCurrentMonth;

    vAccts_ = Model_Setting::instance().GetStringSetting("VIEWACCOUNTS", VIEW_ACCOUNTS_ALL_STR);

    double tBalance = 0.0;
    wxString stocks="", assets="", grand_total="", top="", rightFrame="";


    std::map<int, std::pair<double, double> > accountStats;
    get_account_stats(accountStats);

    wxString leftFrame = displayAccounts(tBalance, accountStats);
    if (Model_Account::hasActiveTermAccount())
    {
        double termBalance = 0.0;
        leftFrame += displayAccounts(termBalance, accountStats, Model_Account::TERM);
        tBalance += termBalance;
    }

    htmlWidgetStocks stocks_widget;
    stocks_widget.enable_detailes(frame_->expandedStockAccounts());
    if (Model_Account::investment_account_num())
    {
        stocks = stocks_widget.getHTMLText();
    }
    tBalance += stocks_widget.get_total();

    leftFrame << stocks;
    leftFrame << displayAssets(tBalance);
    leftFrame << displayGrandTotals(tBalance);

    mmDateRange* date_range = new mmLast30Days();
    htmlWidgetTop7Categories top_trx(date_range);
    leftFrame += top_trx.getHTMLText();

    //Also displays the Income vs Expenses graph.
    rightFrame << displayIncomeVsExpenses();

    htmlWidgetBillsAndDeposits bills_and_deposits(_("Upcoming Transactions"));
    rightFrame += bills_and_deposits.getHTMLText();

    rightFrame << getStatWidget();

    html_text_ = prepareTemplate(leftFrame, rightFrame);
    htmlWindow_->SetPage(html_text_);
}

wxString mmHomePagePanel::prepareTemplate(const wxString& left, const wxString& right)
{
    mmHTMLBuilder hb;
    hb.init();
    hb.startCenter();

    hb.startTable("100%", "top");
    hb.startTableRow();
    hb.startTableCell("100%\" colspan=\"2\" valign=\"middle\" align=\"center");
    hb.addText(getCalendarWidget());
    hb.endTableCell();
    hb.endTableRow();

    hb.startTableRow();
    hb.endTableRow();
    hb.startTableRow();

    hb.startTableCell("50%\" valign=\"top\" align=\"center");

    hb.addText(left);

    hb.endTableCell();
    hb.startTableCell("50%\" valign=\"top\" align=\"center");

    hb.addText(right);

    hb.endTableCell();
    hb.endTableRow();
    hb.endTable();
    hb.end();
    return hb.getHTMLText();
}

wxString mmHomePagePanel::displaySummaryHeader(const wxString& summaryTitle)
{
    mmHTMLBuilder hb;
    hb.startTableRow();
    hb.addTableHeaderCell(summaryTitle, false);
    hb.addTableHeaderCell(_("Reconciled"), true);
    hb.addTableHeaderCell(_("Balance"), true);
    hb.endTableRow();
    return hb.getHTMLText();
}

wxString mmHomePagePanel::displaySectionTotal(const wxString& totalsTitle, double tRecBalance, double& tBalance)
{
    mmHTMLBuilder hb;
    // format the totals for display

    std::vector<double> data;
    data.push_back(tRecBalance);
    data.push_back(tBalance);

    hb.startTableRow();
    hb.addTotalRow(totalsTitle, 3, data);
    hb.endTableRow();
    return hb.getHTMLText();
}

void mmHomePagePanel::get_account_stats(std::map<int, std::pair<double, double> > &accountStats)
{
    bool ignoreFuture = mmIniOptions::instance().ignoreFutureTransactions_;

    Model_Checking::Data_Set transactions = Model_Checking::instance().all();
    this->total_transactions_ = transactions.size();
    for (const auto& trx : transactions)
    {
        if (ignoreFuture)
        {
            if (Model_Checking::TRANSDATE(trx).IsLaterThan(wxDateTime::Today()))
                continue; //skip future dated transactions
        }
        if (Model_Checking::status(trx) == Model_Checking::FOLLOWUP)++this->countFollowUp_;
        if (Model_Checking::status(trx) != Model_Checking::VOID_)
        {
            double amount = (Model_Checking::type(trx) == Model_Checking::DEPOSIT ? trx.TRANSAMOUNT : -trx.TRANSAMOUNT);
            double reconciled_amount = (Model_Checking::status(trx) == Model_Checking::RECONCILED ? amount : 0);

            accountStats[trx.ACCOUNTID].first += reconciled_amount;
            accountStats[trx.ACCOUNTID].second += amount;

            if (Model_Checking::type(trx) == Model_Checking::TRANSFER)
            {
                reconciled_amount = (Model_Checking::status(trx) == Model_Checking::RECONCILED ? trx.TOTRANSAMOUNT : 0);
                accountStats[trx.TOACCOUNTID].first += reconciled_amount;
                accountStats[trx.TOACCOUNTID].second += trx.TOTRANSAMOUNT;
            }
        }
    }
}

/* Accounts */
wxString mmHomePagePanel::displayAccounts(double& tBalance, std::map<int, std::pair<double, double> > &accountStats, int type)
{
    bool type_is_bank = type == Model_Account::CHECKING;

    mmHTMLBuilder hb;
    hb.startTable("100%");

    // Only Show the account titles if we want to display Bank accounts.
    if ( frame_->expandedBankAccounts() && type_is_bank)
        hb.addText(displaySummaryHeader(_("Bank Account")));
    else if (frame_->expandedTermAccounts() && !type_is_bank)
        hb.addText(displaySummaryHeader(_("Term account")));

    // Get account balances and display accounts if we want them displayed
    double tReconciled = 0;
    for (const auto& account: Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
    {
        if (Model_Account::type(account)!= type || Model_Account::status(account) == Model_Account::CLOSED) continue;

        Model_Currency::Data* currency = Model_Account::currency(account);

        double currency_rate = 1;
        if (currency) currency_rate = currency->BASECONVRATE;
        double bal = account.INITIALBAL + accountStats[account.ACCOUNTID].second; //Model_Account::balance(account);
        double reconciledBal = account.INITIALBAL + accountStats[account.ACCOUNTID].first;
        tBalance += bal * currency_rate;
        tReconciled += reconciledBal * currency_rate;

        // Display the individual account links if we want to display them
        if ( ((type_is_bank) ? frame_->expandedBankAccounts() : frame_->expandedTermAccounts())
            || (!frame_->expandedBankAccounts() && !frame_->expandedTermAccounts()) )
        {

            // show the actual amount in that account
            if (((vAccts_ == "Open" && Model_Account::status(account) == Model_Account::OPEN) ||
                (vAccts_ == "Favorites" && Model_Account::FAVORITEACCT(account)) ||
                (vAccts_ == VIEW_ACCOUNTS_ALL_STR))
                && ((type_is_bank) ? frame_->expandedBankAccounts() : frame_->expandedTermAccounts()))
            {
                hb.startTableRow();
                hb.addTableCellLink(wxString::Format("ACCT:%d", account.ACCOUNTID), account.ACCOUNTNAME, false, true);
                hb.addCurrencyCell(reconciledBal, currency);
                hb.addCurrencyCell(bal, currency);
                hb.endTableRow();
            }
        }
    }
    const wxString totalStr = (type_is_bank) ? _("Bank Accounts Total:") : _("Term Accounts Total:");
    hb.addText(displaySectionTotal(totalStr, tReconciled, tBalance));
    hb.endTable();

    return hb.getHTMLinTableWraper();
}

//* Assets *//
wxString mmHomePagePanel::displayAssets(double& tBalance)
{
    mmHTMLBuilder hb;

    if (mmIniOptions::instance().enableAssets_)
    {
        hb.startTable("100%");
        hb.startTableRow();
        hb.addTableCellLink("Assets", _("Assets"), false, true);
        hb.addTableCell("", true);
        hb.addCurrencyCell(Model_Asset::instance().balance());
        hb.endTableRow();
        hb.endTable();

        tBalance += Model_Asset::instance().balance();
    }
    return hb.getHTMLinTableWraper();
}

void mmHomePagePanel::getExpensesIncomeStats(std::map<int, std::pair<double, double> > &incomeExpensesStats
                                             , mmDateRange* date_range)const
{
    //Initialization
    bool ignoreFuture = mmIniOptions::instance().ignoreFutureTransactions_;
    wxDateTime start_date = wxDateTime(date_range->end_date()).SetDay(1);

    //Calculations
    Model_Checking::Data_Set transactions = Model_Checking::instance().find(
        Model_Checking::TRANSDATE(date_range->start_date(), GREATER_OR_EQUAL)
        , Model_Checking::TRANSDATE(date_range->end_date(), LESS_OR_EQUAL)
        , Model_Checking::STATUS(Model_Checking::VOID_, NOT_EQUAL)
        , Model_Checking::TRANSCODE(Model_Checking::TRANSFER, NOT_EQUAL)
    );

    for (const auto& pBankTransaction : transactions)
    {
        if (ignoreFuture)
        {
            if (Model_Checking::TRANSDATE(pBankTransaction).IsLaterThan(wxDateTime::Today()))
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

//* Income vs Expenses *//
wxString mmHomePagePanel::displayIncomeVsExpenses()
{
    double tIncome = 0.0, tExpenses = 0.0;
    std::map<int, std::pair<double, double> > incomeExpensesStats;
    getExpensesIncomeStats( incomeExpensesStats
        , date_range_
    );

    bool show_nothing = !frame_->expandedBankAccounts() && !frame_->expandedTermAccounts();
    bool show_all = (frame_->expandedBankAccounts() && frame_->expandedTermAccounts()) || show_nothing;
    bool show_bank = frame_->expandedBankAccounts();
    for (const auto& account: Model_Account::instance().all())
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

    mmHTMLBuilder hb;

    mmGraphIncExpensesMonth gg;
    gg.init(tIncome, tExpenses);
    gg.Generate("");

        wxString monthHeading = date_range_->title();
        hb.startTable("100%");
        hb.addTableHeaderRow(wxString::Format(_("Income vs Expenses: %s"), monthHeading), 2);

            hb.startTableRow();
            hb.startTableCell();
            hb.addImage(gg.getOutputFileName());
            hb.endTableCell();

            hb.startTableCell();

            hb.startCenter();
            hb.startTable();
            hb.startTableRow();
            hb.addTableHeaderCell(_("Type"));
            hb.addTableHeaderCell(_("Amount"), true);
            hb.endTableRow();

            hb.startTableRow();
            hb.addTableCell(_("Income:"), false, true);
            hb.addCurrencyCell(tIncome);
            hb.endTableRow();

            hb.startTableRow();
            hb.addTableCell(_("Expenses:"), false, true);
            hb.addCurrencyCell(tExpenses);
            hb.endTableRow();

            hb.addRowSeparator(2);
            hb.startTableRow();
            hb.addTableCell(_("Difference:"), false, true, true);
            hb.addCurrencyCell(tIncome - tExpenses);
            hb.endTableRow();

            if (!show_all)
            {
                wxString accounts_type = show_bank ? _("Bank Accounts") : _("Term Accounts");
                hb.addRowSeparator(2);
                hb.startTableRow();
                hb.addTableCell(_("Accounts: "), false, true, false);
                hb.addTableCell(accounts_type, false, true, false);
                hb.endTableRow();
            }
            hb.endTable();
            hb.endCenter();

        hb.endTableCell();
        hb.endTableRow();
        hb.endTable();

    return hb.getHTMLinTableWraper();
}

wxString mmHomePagePanel::getCalendarWidget()
{
    const wxDateTime &today = date_range_->today();
    mmHTMLBuilder hb;
    hb.startTable("100%", "left\" cellpadding=\"1\" cellspacing=\"0", "0");
    hb.startTableRow();
    hb.startTableCell();
    hb.font_settings(hb.font_size());
    hb.bold(wxGetTranslation(wxDateTime::GetEnglishMonthName(today.GetMonth())));
    hb.font_end();
    hb.endTableCell();
    hb.addTableCell("");
    wxDateTime selectedMonthDay = date_range_->start_date();
    for (int d = 1; d <= selectedMonthDay.GetLastMonthDay().GetDay(); d++)
    {
        selectedMonthDay.SetDay(d);
        wxString sColor = "", sBgColor = "";
        if (d == today.GetDay()) sBgColor = "YELLOW";
        hb.startTableCell(wxString::Format("0\" bgcolor=\"%s", sBgColor));
        if (wxDateTime::GetWeekDayName(selectedMonthDay.GetWeekDay())=="Sunday") sColor = "#FF0000";
        else if (wxDateTime::GetWeekDayName(selectedMonthDay.GetWeekDay())=="Saturday") sColor = "#FF0000";
        hb.font_settings(hb.font_size(), sColor);
        hb.addText(wxString()<<d);
        hb.font_end();

        hb.endTableCell();
    }
    /*hb.addTableCell(wxString::Format(_("Week#%d")
        , today.GetWeekOfYear())
        , false, true, false);*/
    hb.addTableCell("");
    hb.addTableCell(wxString() << wxDateTime::Now().GetYear(), false, false, true);

    hb.endTableRow();
    hb.endTable();

    return hb.getHTMLinTableWraper();
}

wxString mmHomePagePanel::getStatWidget()
{
    mmHTMLBuilder hb;

    hb.startTable("100%");
    hb.addTableHeaderRow(_("Transaction Statistics"), 2);

    if (this->countFollowUp_ > 0)
    {
        hb.startTableRow();
        hb.addTableCell(_("Follow Up On Transactions: "));
        hb.addTableCell(wxString::Format("%d", this->countFollowUp_), true, true, true);
        hb.endTableRow();
    }

    hb.startTableRow();
    hb.addTableCell( _("Total Transactions: "));
    hb.addTableCell(wxString::Format("%d", this->total_transactions_), true, true, true);
    hb.endTableRow();
    hb.endTable();

    return hb.getHTMLinTableWraper(true);
}

wxString mmHomePagePanel::displayGrandTotals(double& tBalance)
{
    mmHTMLBuilder hb;
    //  Display the grand total from all sections
    wxString tBalanceStr = Model_Currency::toCurrency(tBalance);

    hb.startTable("100%");
    hb.addTotalRow(_("Grand Total:"), 2, tBalanceStr);
    hb.endTable();

    return hb.getHTMLinTableWraper();
}

void mmHomePagePanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    htmlWindow_ = new mmHtmlWindow(this,
        ID_PANEL_HOMEPAGE_HTMLWINDOW,
        wxDefaultPosition, wxDefaultSize,
        wxHW_SCROLLBAR_AUTO|wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemBoxSizer2->Add(htmlWindow_, 1, wxGROW|wxALL, 0);
}

void mmHomePagePanel::sortTable()
{
}

void mmHtmlWindow::OnLinkClicked(const wxHtmlLinkInfo& link)
{
    mmGUIFrame* frame = wxGetApp().m_frame;
    wxString href = link.GetHref();
    wxString number;
    bool isAcct = href.StartsWith("ACCT:", &number);
    bool isStock = href.StartsWith("STOCK:", &number);
    if (href == "billsdeposits")
    {
        frame->setNavTreeSection(_("Repeating Transactions"));
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_BILLSDEPOSITS);
        frame->GetEventHandler()->AddPendingEvent(evt);
    }
    else if (href == "Assets")
    {
        frame->setNavTreeSection(_("Assets"));
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_ASSETS);
        frame->GetEventHandler()->AddPendingEvent(evt);
    }
    else if (isAcct)
    {
        long id = -1;
        number.ToLong(&id);
        frame->setGotoAccountID(id);
        Model_Account::Data* account = Model_Account::instance().get(id);
        frame->setAccountNavTreeSection(account->ACCOUNTNAME);
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_GOTOACCOUNT);
        frame->GetEventHandler()->AddPendingEvent(evt);
    }
    else if (isStock)
    {
        long id = -1;
        number.ToLong(&id);
        frame->setGotoAccountID(id);
        Model_Account::Data* account = Model_Account::instance().get(id);
        frame->setAccountNavTreeSection(account->ACCOUNTNAME);
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_STOCKS);
        frame->GetEventHandler()->AddPendingEvent(evt);
    }
}

//----------------------------------------------------------------------------
