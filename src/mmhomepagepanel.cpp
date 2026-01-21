/*******************************************************
Copyright (C) 2006 Madhan Kanagavel
Copyright (C) 2014 - 2020 Nikolay Akimov
Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)
Copyright (C) 2026 Klaus Wich

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
    mmPanelBase::Create(parent, winid, pos, size, style, name);

    createControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    createHtml();

    Model_Usage::instance().pageview(this);

    return true;
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

    formatHTML(m_templateText);
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
    mmThemeAutoColour(browser_, false);
}

void mmHomePagePanel::PrintPage()
{
    browser_->Print();
}

void mmHomePagePanel::insertDataIntoTemplate()
{
    m_frames["HTMLSCALE"] = wxString::Format("%d", Option::instance().getHtmlScale());

    // Get curreny details to pass to report for Apexcharts
    int64 baseCurrencyID = Option::instance().getBaseCurrencyID();
    Model_Currency::Data* baseCurrency = Model_Currency::instance().get(baseCurrencyID);

    // Get locale to pass to reports for Apexcharts
    wxString locale = Model_Infotable::instance().getString("LOCALE", "en-US"); // Stay blank of not set, currency override handled in Apexcharts call.
    if (locale == "") {
        locale = "en-US";
    }
    locale.Replace("_", "-");
    m_frames["LOCALE"] = locale;

    double tBalance = 0.0, tReconciled = 0.0;

    // Accounts
    htmlWidgetStocks stocks_widget;
    htmlWidgetAccounts account_stats;

    int accountCount = 0;
    wxString AccountsInfo;
    bool isAccount = false;

    NavigatorTypesInfo* navinfo = NavigatorTypes::instance().getFirstActiveEntry();
    while (navinfo) {
        if (navinfo->navTyp == NavigatorTypes::NAV_TYP_ACCOUNT) {
            if (!isAccount) {
                isAccount = true;
                accountCount++;
                AccountsInfo = wxString::Format("ACCOUNTS_%d", accountCount);
                m_frames[AccountsInfo] = R"(<div class="shadow">)";
            }
            m_frames[AccountsInfo] += account_stats.displayAccounts(tBalance, tReconciled, navinfo->type);
        }
        else if (navinfo->type == NavigatorTypes::TYPE_ID_INVESTMENT) {
            if (isAccount) {
               m_frames[AccountsInfo] += "</div>";
            }
            isAccount = false;
            accountCount++;
            AccountsInfo = wxString::Format("ACCOUNTS_%d", accountCount);

            m_frames[AccountsInfo]= stocks_widget.getHTMLText();
            tBalance += stocks_widget.get_total();

            account_stats.displayAccounts(tBalance, tReconciled, NavigatorTypes::TYPE_ID_SHARES);

        }
        else if (navinfo->type == NavigatorTypes::TYPE_ID_ASSET) {
            if (isAccount) {
               m_frames[AccountsInfo] += "</div>";
            }
            isAccount = false;
            accountCount++;
            AccountsInfo = wxString::Format("ACCOUNTS_%d", accountCount);

            htmlWidgetAssets assets;
            m_frames[AccountsInfo] = assets.getHTMLText();
            tBalance += Model_Asset::instance().balance();
            account_stats.displayAccounts(tBalance, tReconciled, NavigatorTypes::TYPE_ID_ASSET);
        }
        navinfo = NavigatorTypes::instance().getNextActiveEntry(navinfo);
    }
    if (isAccount) {
        m_frames[AccountsInfo] +="</div>";
    }

    htmlWidgetGrandTotals grand_totals;
    m_frames["GRAND_TOTAL"] = grand_totals.getHTMLText(tBalance, tReconciled
                                                , Model_Asset::instance().balance(), stocks_widget.get_total());

    //
    htmlWidgetIncomeVsExpenses income_vs_expenses;
    m_frames["INCOME_VS_EXPENSES"] = income_vs_expenses.getHTMLText();
    m_frames["INCOME_VS_EXPENSES_FORECOLOR"] = mmThemeMetaString(meta::COLOR_REPORT_FORECOLOR);
    m_frames["INCOME_VS_EXPENSES_COLORS"] = wxString::Format("'%s', '%s'", mmThemeMetaString(meta::COLOR_REPORT_CREDIT)
                                                , mmThemeMetaString(meta::COLOR_REPORT_DEBIT));
    m_frames["INCOME_VS_EXPENSES_CURR_PFX_SYMBOL"] = baseCurrency ? baseCurrency->PFX_SYMBOL : "$";
    m_frames["INCOME_VS_EXPENSES_CURR_SFX_SYMBOL"] = baseCurrency ? baseCurrency->SFX_SYMBOL : "";
    m_frames["INCOME_VS_EXPENSES_CURR_GROUP_SEPARATOR"] = baseCurrency ? baseCurrency->GROUP_SEPARATOR : ",";
    m_frames["INCOME_VS_EXPENSES_CURR_DECIMAL_POINT"] = baseCurrency ? baseCurrency->DECIMAL_POINT : ".";
    m_frames["INCOME_VS_EXPENSES_CURR_SCALE"] = baseCurrency ? wxString::Format("%d", static_cast<int>(log10(baseCurrency->SCALE.GetValue()))) : "";


    htmlWidgetBillsAndDeposits bills_and_deposits(_t("Upcoming Transactions"));
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
    const wxString json = Model_Infotable::instance().getString("HOME_PAGE_STATUS", "{}");
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
    wxString sData = "";
    int cmdInt = -1;

    wxRegEx pattern(R"(^(https?:)|(file:)\/\/)");
    if (pattern.Matches(uri))
    {
        wxLaunchDefaultBrowser(uri);
        evt.Veto();
    }
    else if (uri.StartsWith("memory:", &sData))
    {
        wxLaunchDefaultBrowser(sData);
        evt.Veto();
    }
    else if (uri.StartsWith("assets:", &sData))
    {
        cmdInt = NavigatorTypes::TYPE_ID_ASSET;
    }
    else if (uri.StartsWith("billsdeposits:", &sData))
    {
        cmdInt = NavigatorTypes::NAV_ENTRY_SCHEDULED_TRANSACTIONS;
    }
    else if (uri.StartsWith("acct:", &sData))
    {
        cmdInt = NavigatorTypes::TYPE_ID_CHECKING;
    }
    else if (uri.StartsWith("stock:", &sData))
    {
        cmdInt = NavigatorTypes::TYPE_ID_INVESTMENT;
    }

    if (cmdInt > -1) {
        wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, MENU_GOTOACCOUNT);
        event.SetInt(cmdInt);
        event.SetString(sData);
        wxPostEvent(m_frame, event);
        evt.Veto();  // Inhibit a wxEVT_WEBVIEW_NEWWINDOW_FEATURES event, which will crash!
    }
}

void mmHomePagePanel::OnLinkClicked(wxWebViewEvent& event)
{
    const wxString& url = wxURI::Unescape(event.GetURL());
    if (!url.Contains("#"))
        return;

    wxLogDebug("{{{ mmHomePagePanel::OnLinkClicked()");
    wxString name = url.AfterLast('#');
    wxLogDebug("Name = %s", name);

    //Convert the JSON string from database to a json object
    const wxString key = "HOME_PAGE_STATUS";
    wxString j_str = Model_Infotable::instance().getString(key, "{}");
    Document j_doc;
    if (j_doc.Parse(j_str.c_str()).HasParseError())
        return;

    Document::AllocatorType& json_allocator = j_doc.GetAllocator();
    wxLogDebug("Old %s:\n%s", key, JSON_PrettyFormated(j_doc));

    const wxString type[] = { "TOP_CATEGORIES", "INVEST", "ACCOUNTS_INFO"
        ,"CARD_ACCOUNTS_INFO" ,"CASH_ACCOUNTS_INFO", "LOAN_ACCOUNTS_INFO"
        , "TERM_ACCOUNTS_INFO", "ASSETS", "SHARE_ACCOUNTS_INFO"
        , "CURRENCY_RATES", "BILLS_AND_DEPOSITS" };

    for (const auto& entry : type) {
        if (name != entry) continue;
        Value v_type(entry.c_str(), json_allocator);
        if (j_doc.HasMember(v_type) && j_doc[v_type].IsBool()) {
            j_doc[v_type] = !j_doc[v_type].GetBool();
        }
        else {
            j_doc.AddMember(v_type, true, json_allocator);
        }
    }

    wxLogDebug("New %s:\n%s", key, JSON_PrettyFormated(j_doc));
    Model_Infotable::instance().setJdoc(key, j_doc);
    wxLogDebug("}}}");
}
