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
#include "mmex.h"

#include "reports/htmlbuilder.h"
#include "billsdepositspanel.h"
#include "reports/mmgraphincexpensesmonth.h"
#include "mmCurrencyFormatter.h"
#include <algorithm>
#include "model/Model_Setting.h"
#include "model/Model_Asset.h"
#include "model/Model_Payee.h"
#include "model/Model_Account.h"
#include "model/Model_Billsdeposits.h"


BEGIN_EVENT_TABLE( mmHomePagePanel, wxPanel )
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(mmHtmlWindow, wxHtmlWindow)
END_EVENT_TABLE()

mmHomePagePanel::mmHomePagePanel(mmCoreDB* core,
            wxWindow *parent,
            wxWindowID winid,
            const wxPoint& pos,
            const wxSize& size,
            long style,
            const wxString& name )
        : mmPanelBase(core)
        , frame_(wxGetApp().m_frame)
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
    if (!core_->db_.get()) return;

    if (mmIniOptions::instance().ignoreFutureTransactions_)
        date_range_ = new mmCurrentMonthToDate;
    else
        date_range_ = new mmCurrentMonth;
    vAccts_ = Model_Setting::instance().GetStringSetting("VIEWACCOUNTS", VIEW_ACCOUNTS_ALL_STR);


    double tBalance = 0.0, termBalance = 0.0;
    wxString acc, term ="", stocks="", assets="", grand_total="", curr="", top="", leftFrame="", rightFrame="";

    acc = displayAccounts(tBalance);

    if ( frame_->hasActiveTermAccounts())
    {
        term = displayAccounts(termBalance, Model_Account::TERM);
        tBalance += termBalance;
    }

    if (Model_Account::investment_account_num())
         stocks = displayStocks(tBalance);

    leftFrame << acc << term << stocks;
    leftFrame << displayAssets(tBalance);
    leftFrame << displayGrandTotals(tBalance);
    curr = displayCurrencies();
    leftFrame << displayCurrencies();

    htmlWidgetTop7Categories top_trx(core_);
    leftFrame += top_trx.getHTMLText();

    //Also displays the Income vs Expenses graph.
    rightFrame << displayIncomeVsExpenses();

    htmlWidgetBillsAndDeposits bills_and_deposits(core_, _("Upcoming Transactions"));
    rightFrame += bills_and_deposits.getHTMLText();

    rightFrame << getStatWidget();

    wxString pageHTML = prepareTemplate(leftFrame, rightFrame);
    htmlWindow_->SetPage(pageHTML);

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
    core_->currencyList_.LoadBaseCurrencySettings();

    std::vector<double> data;
    data.push_back(tRecBalance);
    data.push_back(tBalance);

    hb.startTableRow();
    hb.addTotalRow(totalsTitle, 3, data);
    hb.endTableRow();
    return hb.getHTMLText();
}

/* Accounts */
wxString mmHomePagePanel::displayAccounts(double& tBalance, int type)
{
    bool type_is_bank = type == Model_Account::CHECKING;
    double tRecBalance = 0.0;

    mmHTMLBuilder hb;
    hb.startTable("100%");
    // Only Show the account titles if we want to display Bank accounts.
    if ( frame_->expandedBankAccounts() && type_is_bank)
        hb.addText(displaySummaryHeader(_("Bank Account")));
    else if (frame_->expandedTermAccounts() && !type_is_bank)
        hb.addText(displaySummaryHeader(_("Term account")));

    // Get account balances and display accounts if we want them displayed
    for (const auto& account: Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
    {
        if (Model_Account::type(account)!= type || Model_Account::status(account) == Model_Account::CLOSED) continue;

        Model_Currency::Data* currency = Model_Account::currency(account);

        double bal = Model_Account::balance(account);
        double reconciledBal = account.INITIALBAL + core_->bTransactionList_.getReconciledBalance(account.ACCOUNTID
            , mmIniOptions::instance().ignoreFutureTransactions_);
        double rate = currency->BASECONVRATE;
        tBalance += bal * rate; // actual amount in that account in the original rate
        tRecBalance += reconciledBal * rate;

        // Display the individual account links if we want to display them
        if ( ((type_is_bank) ? frame_->expandedBankAccounts() : frame_->expandedTermAccounts())
            || (!frame_->expandedBankAccounts() && !frame_->expandedTermAccounts()) )
        {

            // show the actual amount in that account
            if (((vAccts_ == "Open" && Model_Account::status(account) == Model_Account::OPEN) ||
                (vAccts_ == "Favorites" && account.FAVORITEACCT == "TRUE") ||
                (vAccts_ == VIEW_ACCOUNTS_ALL_STR))
                && ((type_is_bank) ? frame_->expandedBankAccounts() : frame_->expandedTermAccounts()))
            {
                hb.startTableRow();
                hb.addTableCellLink(wxString::Format("ACCT:%d", account.ACCOUNTID), account.ACCOUNTNAME, false, true);
                hb.addMoneyCell(reconciledBal, true);
                hb.addMoneyCell(bal);
                hb.endTableRow();
            }
        }
    }
    const wxString totalStr = (type_is_bank) ? _("Bank Accounts Total:") : _("Term Accounts Total:");
    hb.addText(displaySectionTotal(totalStr, tRecBalance, tBalance));
    hb.endTable();

    return hb.getHTMLinTableWraper();
}

//* Stocks *//
wxString mmHomePagePanel::displayStocks(double& tBalance /*, double& tIncome, double& tExpenses */)
{
    mmHTMLBuilder hb;
    double stTotalBalance = 0.0, stTotalGain = 0.0;
    wxString tBalanceStr, tGainStr;

    hb.startTable("100%");
    if (frame_->expandedStockAccounts())
    {
        hb.startTableRow();
        hb.addTableHeaderCell(_("Stocks"), false);
        hb.addTableHeaderCell(_("Gain/Loss"), true);
        hb.addTableHeaderCell(_("Total"), true);
        hb.endTableRow();
    }

    static const char sql[] =
    "select "
    "c.BASECONVRATE, "
    "st.heldat as ACCOUNTID, a.accountname as ACCOUNTNAME, "
    "a.initialbal + "
    "total((st.CURRENTPRICE)*st.NUMSHARES) as BALANCE, "
    "total ((st.CURRENTPRICE-st.PURCHASEPRICE)*st.NUMSHARES-st.COMMISSION) as GAIN "
    "from  stock_v1 st "
    "left join accountlist_v1 a on a.accountid=st.heldat "
    "left join currencyformats_v1 c on c.currencyid=a.currencyid "
    "    where st.purchasedate<=date ('now','localtime') "
    "and a.status='Open' "
    "group by st.heldat ";

    wxSQLite3ResultSet q1 = core_->db_.get()->ExecuteQuery(sql);
    while(q1.NextRow())
    {
        int stockaccountId = q1.GetInt("ACCOUNTID");
        double stockBalance = q1.GetDouble("BALANCE");
        wxString stocknameStr = q1.GetString("ACCOUNTNAME");
        //double income = q1.GetDouble("INCOME");
        //double expenses = q1.GetDouble("EXPENCES");
        double baseconvrate = q1.GetDouble("BASECONVRATE");
        double stockGain = q1.GetDouble("GAIN");

        Model_Account::Data* account = Model_Account::instance().get(stockaccountId);
        Model_Currency::Data* currency = Model_Account::currency(account);

        // if Stock accounts being displayed, include income/expense totals on home page.
        //tIncome += income * baseconvrate;
        //tExpenses += expenses * baseconvrate;
        stTotalBalance += stockBalance * baseconvrate;
        stTotalGain += stockGain * baseconvrate;
        //We can hide or show Stocks on Home Page
        if (frame_->expandedStockAccounts())
        {
            hb.startTableRow();
            //////
            //hb.addTableCell(stocknameStr, false,true);
            hb.addTableCellLink(wxString::Format("STOCK:%d"
                , stockaccountId), stocknameStr, false, true);
            hb.addMoneyCell(stockGain, true);
            hb.addMoneyCell(stockBalance, true);
            hb.endTableRow();
        }
    }
    q1.Finalize();

    hb.addText(displaySectionTotal(_("Stocks Total:"), stTotalGain, stTotalBalance));
    hb.endTable();

    // Add Stock balance to Grand Total balance
    tBalance += stTotalBalance;
    return hb.getHTMLinTableWraper();
}

//* Assets *//
wxString mmHomePagePanel::displayAssets(double& tBalance)
{
    mmHTMLBuilder hb;
    core_->currencyList_.LoadBaseCurrencySettings();

    if (mmIniOptions::instance().enableAssets_)
    {
        hb.startTable("100%");
        hb.startTableRow();
        hb.addTableCellLink("Assets", _("Assets"), false, true);
        hb.addTableCell("", true);
        hb.addMoneyCell(Model_Asset::instance().balance());
        hb.endTableRow();
        hb.endTable();

        tBalance += Model_Asset::instance().balance();
    }
    return hb.getHTMLinTableWraper();
}

//* Currencies *//
wxString mmHomePagePanel::displayCurrencies()
{
    static const char sql[] =
        "select ACCOUNTID, CURRENCYNAME, BALANCE, BASECONVRATE from ( "
        "select t.accountid as ACCOUNTID, c.currencyname as CURRENCYNAME, "
        "total (t.BALANCE) as BALANCE, "
        "c.BASECONVRATE as BASECONVRATE "
        "from ( "
        "select  acc.accountid as ACCOUNTID, acc.INITIALBAL as BALANCE "
        "from ACCOUNTLIST_V1 ACC "
        "where ACC.STATUS='Open' "
        "group by acc.accountid  "
        "union all "
        "select  "
        "st.heldat as ACCOUNTID, "
        "total((st.CURRENTPRICE)*st.NUMSHARES-st.COMMISSION) as BALANCE "
        "from  stock_v1 st "
        "where st.purchasedate<=date ('now','localtime') "
        "group by st.heldat "
        "union all "
        "select ca.toaccountid,  total(ca.totransamount) "
        "from checkingaccount_v1 ca "
        "where ca.transcode ='Transfer' and ca.STATUS<>'V' and ca.transdate<=date ('now','localtime') "
        "group by ca.toaccountid "
        "union all "
        "select ca.accountid,  total(case ca.transcode when 'Deposit' then ca.transamount else -ca.transamount end)  "
        "from checkingaccount_v1 ca "
        "where ca.STATUS<>'V' and ca.transdate<=date ('now','localtime') "
        "group by ca.accountid) t "
        "left join accountlist_v1 a on a.accountid=t.accountid "
        "left join  currencyformats_v1 c on c.currencyid=a.currencyid "
        "where a.status='Open' and balance<>0 "
        "group by c.currencyid) order by CURRENCYNAME ";

    wxSQLite3ResultSet q1 = core_->db_.get()->ExecuteQuery(sql);
    q1 = core_->db_.get()->ExecuteQuery(sql);

    //Determine how many currencies used
    int curnumber = 0;
    while(q1.NextRow())
        curnumber+=1;

    mmHTMLBuilder hb;
    if (curnumber > 1 )
    {
        // display the currency header
        hb.startTable("100%");
        hb.startTableRow();
        hb.addTableHeaderCell(_("Currency"), false);
        hb.addTableHeaderCell(_("Base Rate"), true);
        hb.addTableHeaderCell(_("Summary"), true);
        hb.endTableRow();

        // display the totals for each currency value
        while(q1.NextRow())
        {
            int accountId = q1.GetInt("ACCOUNTID");
            double currBalance = q1.GetDouble("BALANCE");
            wxString currencyStr = q1.GetString("CURRENCYNAME");
            double convRate = q1.GetDouble("BASECONVRATE");
            wxString convRateStr;

            Model_Account::Data* account = Model_Account::instance().get(accountId);
            Model_Currency::Data* currency = Model_Account::currency(account);

            wxString tBalanceStr;

            hb.startTableRow();
            hb.addTableCell(currencyStr, false, false, true);
            hb.addTableCell(CurrencyFormatter::float2String(convRate), true);
            hb.addMoneyCell(currBalance);
            hb.endTableRow();
        }
        hb.endTable();
        q1.Finalize();
    }

    return hb.getHTMLinTableWraper(true);
}

//* Income vs Expenses *//
wxString mmHomePagePanel::displayIncomeVsExpenses()
{
    bool group_by_account = true;
    double tIncome = 0.0, tExpenses = 0.0;
    std::map<int, std::pair<double, double> > incomeExpensesStats;
    core_->bTransactionList_.getExpensesIncomeStats(incomeExpensesStats
        , date_range_
        , -1
        , group_by_account
    );
    core_->currencyList_.LoadBaseCurrencySettings();

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
        int idx = group_by_account ? (1000000 * account.ACCOUNTID) : 0;
        tIncome += incomeExpensesStats[idx].first;
        tExpenses += incomeExpensesStats[idx].second;
        if (!group_by_account) break;
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
            hb.addMoneyCell(tIncome);
            hb.endTableRow();

            hb.startTableRow();
            hb.addTableCell(_("Expenses:"), false, true);
            hb.addMoneyCell(tExpenses);
            hb.endTableRow();

            hb.addRowSeparator(2);
            hb.startTableRow();
            hb.addTableCell(_("Difference:"), false, true, true);
            hb.addMoneyCell(tIncome - tExpenses);
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
    hb.addTableCell(wxString::Format(_("Week#%d")
        , today.GetWeekOfYear())
        , false, false, true);
    hb.addTableCell(wxString()<<wxDateTime::Now().GetYear(), false, false, true);

    hb.endTableRow();
    hb.endTable();

    return hb.getHTMLinTableWraper();
}

wxString mmHomePagePanel::getStatWidget()
{
    mmHTMLBuilder hb;
    int countFollowUp = core_->bTransactionList_.countFollowupTransactions();

    hb.startTable("100%");
    hb.addTableHeaderRow(_("Transaction Statistics"), 2);

    if (countFollowUp > 0)
    {
        hb.startTableRow();
        hb.addTableCell(_("Follow Up On Transactions: "));
        hb.addTableCell(wxString::Format("%d", countFollowUp), true, true, true);
        hb.endTableRow();
    }

    hb.startTableRow();
    hb.addTableCell( _("Total Transactions: "));
    hb.addTableCell(wxString::Format("%ld", core_->bTransactionList_.transactions_.size()), true, true, true);
    hb.endTableRow();
    hb.endTable();

    return hb.getHTMLinTableWraper(true);
}

wxString mmHomePagePanel::displayGrandTotals(double& tBalance)
{
    mmHTMLBuilder hb;
    //  Display the grand total from all sections
    wxString tBalanceStr;
    core_->currencyList_.LoadBaseCurrencySettings();
    tBalanceStr = CurrencyFormatter::float2Money(tBalance);

    hb.startTable("100%");
    hb.addTotalRow(_("Grand Total:"), 2, tBalanceStr);
    hb.endTable();

    return hb.getHTMLinTableWraper();
}

void mmHomePagePanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    htmlWindow_ = new mmHtmlWindow(this, core_,
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
