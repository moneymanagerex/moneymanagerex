/*******************************************************
Copyright (C) 2006 Madhan Kanagavel
Copyright (C) 2014 - 2020 Nikolay Akimov
Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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

#include "images_list.h"
#include "mmhomepagepanel.h"
#include "mmhomepage.h"
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
    , browser_(nullptr)
{
    Create(parent, winid, pos, size, style, name);
    m_frame->menuPrintingEnable(true);
}

mmHomePagePanel::~mmHomePagePanel()
{
    m_frame->menuPrintingEnable(false);
    clearVFprintedFiles("hp");
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

    createControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    createHtml();

    Model_Usage::instance().pageview(this);

    return TRUE;
}

void  mmHomePagePanel::createHtml()
{
    // Read template from file
    m_templateText.clear();
    const wxString template_path = mmex::getPathResource(mmex::HOME_PAGE_TEMPLATE);
    wxFileInputStream input(template_path);
    wxTextInputStream text(input, "\x09", wxConvUTF8);
    while (input.IsOk() && !input.Eof())
    {
        m_templateText += text.ReadLine() + "\n";
    }

    insertDataIntoTemplate();
    fillData();
}

void mmHomePagePanel::createControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);
    browser_ = wxWebView::New();
#ifdef __WXMAC__
    // With WKWebView handlers need to be registered before creation
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
    browser_->Create(this, mmID_BROWSER);
#else
    browser_->Create(this, mmID_BROWSER);
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
#endif
#ifndef _DEBUG
    browser_->EnableContextMenu(false);
#endif

    Bind(wxEVT_WEBVIEW_NEWWINDOW, &mmHomePagePanel::OnNewWindow, this, mmID_BROWSER);

    itemBoxSizer2->Add(browser_, 1, wxGROW | wxALL, 0);
}

void mmHomePagePanel::PrintPage()
{
    browser_->Print();
}

void mmHomePagePanel::insertDataIntoTemplate()
{
    m_frames["HTMLSCALE"] = wxString::Format("%d", Option::instance().getHtmlFontSize());

    double tBalance = 0.0, tReconciled = 0.0;
    double cardBalance = 0.0, cardReconciled = 0.0;
    double termBalance = 0.0, termReconciled = 0.0;
    double cashBalance = 0.0, cashReconciled = 0.0;
    double loanBalance = 0.0, loanReconciled = 0.0;
    //double shareBalance = 0.0, assetBalance = 0.0;

    htmlWidgetAccounts account_stats;
    m_frames["ACCOUNTS_INFO"] = account_stats.displayAccounts(tBalance, tReconciled, Model_Account::CHECKING);
    m_frames["CARD_ACCOUNTS_INFO"] = account_stats.displayAccounts(cardBalance, cardReconciled, Model_Account::CREDIT_CARD);
    tBalance += cardBalance;
    tReconciled += cardReconciled;

    // Accounts
    m_frames["CASH_ACCOUNTS_INFO"] = account_stats.displayAccounts(cashBalance, cashReconciled, Model_Account::CASH);
    tBalance += cashBalance;
    tReconciled += cashReconciled;

    m_frames["LOAN_ACCOUNTS_INFO"] = account_stats.displayAccounts(loanBalance, loanReconciled, Model_Account::LOAN);
    tBalance += loanBalance;
    tReconciled += loanReconciled;

    m_frames["TERM_ACCOUNTS_INFO"] = account_stats.displayAccounts(termBalance, termReconciled, Model_Account::TERM);
    tBalance += termBalance;
    tReconciled += termReconciled;

    //m_frames["ASSET_ACCOUNTS_INFO"] = account_stats.displayAccounts(assetBalance, Model_Account::ASSET);
    //tBalance += assetBalance;

    //m_frames["SHARE_ACCOUNTS_INFO"] = account_stats.displayAccounts(shareBalance, Model_Account::SHARES);
    //tBalance += shareBalance;

    //Stocks
    htmlWidgetStocks stocks_widget;
    m_frames["STOCKS_INFO"] = stocks_widget.getHTMLText();
    tBalance += stocks_widget.get_total();

    htmlWidgetAssets assets;
    m_frames["ASSETS_INFO"] = assets.getHTMLText();
    tBalance += Model_Asset::instance().balance();

    htmlWidgetGrandTotals grand_totals;
    m_frames["GRAND_TOTAL"] = grand_totals.getHTMLText(tBalance, tReconciled
                                                , Model_Asset::instance().balance(), stocks_widget.get_total());

    //
    htmlWidgetIncomeVsExpenses income_vs_expenses;
    m_frames["INCOME_VS_EXPENSES"] = income_vs_expenses.getHTMLText();
    m_frames["INCOME_VS_EXPENSES_FORECOLOR"] = mmThemeMetaString(meta::COLOR_REPORT_FORECOLOR);
    m_frames["INCOME_VS_EXPENSES_COLORS"] = wxString::Format("'%s', '%s'", mmThemeMetaString(meta::COLOR_REPORT_CREDIT)
                                                , mmThemeMetaString(meta::COLOR_REPORT_DEBIT));

    htmlWidgetBillsAndDeposits bills_and_deposits(_("Upcoming Transactions"));
    m_frames["BILLS_AND_DEPOSITS"] = bills_and_deposits.getHTMLText();

    htmlWidgetTop7Categories top_trx;
    m_frames["TOP_CATEGORIES"] = top_trx.getHTMLText();

    htmlWidgetStatistics stat_widget;
    m_frames["STATISTICS"] = stat_widget.getHTMLText();
    m_frames["TOGGLES"] = getToggles();

    htmlWidgetCurrency currency_rates;
    m_frames["CURRENCY_RATES"] = currency_rates.getHtmlText();
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

    const auto name = getVFname4print("hp", m_templateText);
    browser_->LoadURL(name);

}

void mmHomePagePanel::OnNewWindow(wxWebViewEvent& evt)
{
    const wxString uri = evt.GetURL();
    wxString sData;

    wxRegEx pattern(R"(^(https?:)|(file:)\/\/)");
    if (pattern.Matches(uri))
    {
        wxLaunchDefaultBrowser(uri);
    }
    else if (uri.StartsWith("memory:", &sData))
    {
        wxLaunchDefaultBrowser(sData);
    }
    else if (uri.StartsWith("assets:", &sData))
    {
        m_frame->setNavTreeSection(_("Assets"));
        wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, MENU_ASSETS);
        m_frame->GetEventHandler()->AddPendingEvent(event);
    }
    else if (uri.StartsWith("billsdeposits:", &sData))
    {
        m_frame->setNavTreeSection(_("Recurring Transactions"));
        wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, MENU_BILLSDEPOSITS);
        m_frame->GetEventHandler()->AddPendingEvent(event);
    }
    else if (uri.StartsWith("acct:", &sData))
    {
        long id = -1;
        sData.ToLong(&id);
        const Model_Account::Data* account = Model_Account::instance().get(id);
        if (account) {
            m_frame->setGotoAccountID(id);
            m_frame->setAccountNavTreeSection(account->ACCOUNTNAME);
            wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, MENU_GOTOACCOUNT);
            m_frame->GetEventHandler()->AddPendingEvent(event);
        }
    }
    else if (uri.StartsWith("stock:", &sData))
    {
        long id = -1;
        sData.ToLong(&id);
        const Model_Account::Data* account = Model_Account::instance().get(id);
        if (account) {
            m_frame->setGotoAccountID(id);
            m_frame->setAccountNavTreeSection(account->ACCOUNTNAME);
            wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, MENU_STOCKS);
            m_frame->GetEventHandler()->AddPendingEvent(event);
        }
    }

    evt.Skip();
}

void mmHomePagePanel::OnLinkClicked(wxWebViewEvent& event)
{
    const wxString& url = wxURI::Unescape(event.GetURL());

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

        const wxString type[] = { "TOP_CATEGORIES", "INVEST", "ACCOUNTS_INFO"
            ,"CARD_ACCOUNTS_INFO" ,"CASH_ACCOUNTS_INFO", "LOAN_ACCOUNTS_INFO"
            , "TERM_ACCOUNTS_INFO", "ASSET_ACCOUNTS_INFO", "SHARE_ACCOUNTS_INFO"
            , "CURRENCY_RATES", "BILLS_AND_DEPOSITS" };

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
