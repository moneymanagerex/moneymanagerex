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

#include <algorithm>
#include <cmath>
#include <html_template.h>

#include "base/constants.h"
#include "base/images_list.h"
#include "mmex.h"
#include "base/paths.h"
#include "util/_util.h"

#include "model/_all.h"
#include "model/PrefModel.h"

#include "mmframe.h"
#include "DashboardPanel.h"
#include "DashboardWidget.h"
#include "SchedPanel.h"

wxBEGIN_EVENT_TABLE(DashboardPanel, wxPanel)
    EVT_WEBVIEW_NAVIGATING(wxID_ANY, DashboardPanel::onLinkClicked)
wxEND_EVENT_TABLE()

DashboardPanel::DashboardPanel(
    wxWindow* parent_win,
    mmGUIFrame* frame,
    wxWindowID win_id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name
) :
    w_frame(frame)
{
    create(parent_win, win_id, pos, size, style, name);
    w_frame->menuPrintingEnable(true);
}

DashboardPanel::~DashboardPanel()
{
    w_frame->menuPrintingEnable(false);
    clearVFprintedFiles("hp");
}

wxString DashboardPanel::getHomePageText() const
{
    return m_templateText;
}

bool DashboardPanel::create(
    wxWindow* parent_win,
    wxWindowID win_id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name
) {
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    PanelBase::Create(parent_win, win_id, pos, size, style, name);

    createControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    createHtml();

    UsageModel::instance().pageview(this);

    return true;
}

void DashboardPanel::createHtml()
{
    // Read template from file
    m_templateText.clear();
    const wxString template_path = mmex::getPathResource(mmex::HOME_PAGE_TEMPLATE);
    wxFileInputStream input(template_path);
    wxTextInputStream text(input, "\x09", wxConvUTF8);
    while (input.IsOk() && !input.Eof()) {
        m_templateText += text.ReadLine() + "\n";
    }

    formatHTML(m_templateText);
    insertDataIntoTemplate();
    fillData();
}

void DashboardPanel::createControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);
    w_browser = wxWebView::New();
#ifdef __WXMAC__
    // With WKWebView handlers need to be registered before creation
    w_browser->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
    w_browser->Create(this, mmID_BROWSER);
#else
    w_browser->Create(this, mmID_BROWSER);
    w_browser->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
#endif
#ifndef _DEBUG
    w_browser->EnableContextMenu(false);
#endif

    Bind(wxEVT_WEBVIEW_NEWWINDOW, &DashboardPanel::onNewWindow, this, mmID_BROWSER);

    itemBoxSizer2->Add(w_browser, 1, wxGROW | wxALL, 0);
    mmThemeAutoColour(w_browser, false);
}

void DashboardPanel::insertDataIntoTemplate()
{
    m_htmlText_mLabel["HTMLSCALE"] = wxString::Format("%d",
        PrefModel::instance().getHtmlScale()
    );

    // Get curreny details to pass to report for Apexcharts
    int64 base_currency_id = PrefModel::instance().getBaseCurrencyID();
    const CurrencyData* base_currency_n = CurrencyModel::instance().get_id_data_n(
        base_currency_id
    );

    // Get locale to pass to reports for Apexcharts
    // Stay blank of not set, currency override handled in Apexcharts call.
    wxString locale = InfoModel::instance().getString("LOCALE", "en-US");
    if (locale == "") {
        locale = "en-US";
    }
    locale.Replace("_", "-");
    m_htmlText_mLabel["LOCALE"] = locale;

    double tBalance = 0.0, tAccountBalance = 0.0, tReconciled = 0.0;

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
                m_htmlText_mLabel[AccountsInfo] = R"(<div class="shadow">)";
            }
            m_htmlText_mLabel[AccountsInfo] +=
                account_stats.displayAccounts(tAccountBalance, tReconciled, navinfo->type);
        }
        else if (navinfo->type == NavigatorTypes::TYPE_ID_INVESTMENT) {
            if (isAccount) {
               m_htmlText_mLabel[AccountsInfo] += "</div>";
            }
            isAccount = false;
            accountCount++;
            AccountsInfo = wxString::Format("ACCOUNTS_%d", accountCount);

            m_htmlText_mLabel[AccountsInfo]= stocks_widget.getHTMLText();
            tBalance += stocks_widget.get_total();

            account_stats.displayAccounts(tBalance, tReconciled, NavigatorTypes::TYPE_ID_SHARES);

        }
        else if (navinfo->type == NavigatorTypes::TYPE_ID_ASSET) {
            if (isAccount) {
               m_htmlText_mLabel[AccountsInfo] += "</div>";
            }
            isAccount = false;
            accountCount++;
            AccountsInfo = wxString::Format("ACCOUNTS_%d", accountCount);

            htmlWidgetAssets assets;
            m_htmlText_mLabel[AccountsInfo] = assets.getHTMLText();
            tBalance += AssetModel::instance().find_all_balance();
            account_stats.displayAccounts(tBalance, tReconciled, NavigatorTypes::TYPE_ID_ASSET);
        }
        navinfo = NavigatorTypes::instance().getNextActiveEntry(navinfo);
    }
    if (isAccount) {
        m_htmlText_mLabel[AccountsInfo] +="</div>";
    }

    htmlWidgetGrandTotals grand_totals;
    m_htmlText_mLabel["GRAND_TOTAL"] = grand_totals.getHTMLText(
        tBalance + tAccountBalance,
        PrefModel::instance().getShowReconciledInHomePage() ? tReconciled : tAccountBalance,
        AssetModel::instance().find_all_balance(),
        stocks_widget.get_total()
    );

    //
    htmlWidgetIncomeVsExpenses income_vs_expenses;
    m_htmlText_mLabel["INCOME_VS_EXPENSES"] = income_vs_expenses.getHTMLText();
    m_htmlText_mLabel["INCOME_VS_EXPENSES_FORECOLOR"] =
        mmThemeMetaString(meta::COLOR_REPORT_FORECOLOR);
    m_htmlText_mLabel["INCOME_VS_EXPENSES_COLORS"] =
        wxString::Format("'%s', '%s'",
            mmThemeMetaString(meta::COLOR_REPORT_CREDIT),
            mmThemeMetaString(meta::COLOR_REPORT_DEBIT)
        );
    m_htmlText_mLabel["INCOME_VS_EXPENSES_CURR_PFX_SYMBOL"] = base_currency_n
        ? base_currency_n->m_prefix_symbol
        : "$";
    m_htmlText_mLabel["INCOME_VS_EXPENSES_CURR_SFX_SYMBOL"] = base_currency_n
        ? base_currency_n->m_suffix_symbol
        : "";
    m_htmlText_mLabel["INCOME_VS_EXPENSES_CURR_GROUP_SEPARATOR"] = base_currency_n
        ? base_currency_n->m_group_separator
        : ",";
    m_htmlText_mLabel["INCOME_VS_EXPENSES_CURR_DECIMAL_POINT"] = base_currency_n
        ? base_currency_n->m_decimal_point
        : ".";
    m_htmlText_mLabel["INCOME_VS_EXPENSES_CURR_SCALE"] = base_currency_n
        ? wxString::Format("%d", static_cast<int>(log10(base_currency_n->m_scale.GetValue())))
        : "";

    htmlWidgetBillsAndDeposits bills_and_deposits(_t("Upcoming Transactions"));
    m_htmlText_mLabel["BILLS_AND_DEPOSITS"] = bills_and_deposits.getHTMLText();

    htmlWidgetTop7Categories top_trx;
    m_htmlText_mLabel["TOP_CATEGORIES"] = top_trx.getHTMLText();

    htmlWidgetStatistics stat_widget;
    m_htmlText_mLabel["STATISTICS"] = stat_widget.getHTMLText();
    m_htmlText_mLabel["TOGGLES"] = getToggles();

    htmlWidgetCurrency currency_rates;
    m_htmlText_mLabel["CURRENCY_RATES"] = currency_rates.getHtmlText();
}

const wxString DashboardPanel::getToggles()
{
    const wxString json = InfoModel::instance().getString("HOME_PAGE_STATUS", "{}");
    return json;
}

void DashboardPanel::fillData()
{
    for (const auto& entry : m_htmlText_mLabel) {
        m_templateText.Replace(wxString::Format("<TMPL_VAR %s>", entry.first), entry.second);
    }

    const auto name = getVFname4print("hp", m_templateText);
    w_browser->LoadURL(name);
}

void DashboardPanel::onNewWindow(wxWebViewEvent& evt)
{
    const wxString uri = evt.GetURL();
    wxString sData = "";
    int cmdInt = -1;

    wxRegEx pattern(R"(^(https?:)|(file:)\/\/)");
    if (pattern.Matches(uri)) {
        wxLaunchDefaultBrowser(uri);
        evt.Veto();
    }
    else if (uri.StartsWith("memory:", &sData)) {
        wxLaunchDefaultBrowser(sData);
        evt.Veto();
    }
    else if (uri.StartsWith("assets:", &sData)) {
        cmdInt = NavigatorTypes::TYPE_ID_ASSET;
    }
    else if (uri.StartsWith("billsdeposits:", &sData)) {
        cmdInt = NavigatorTypes::NAV_ENTRY_SCHEDULED_TRANSACTIONS;
    }
    else if (uri.StartsWith("acct:", &sData)) {
        cmdInt = NavigatorTypes::TYPE_ID_CHECKING;
    }
    else if (uri.StartsWith("stock:", &sData)) {
        cmdInt = NavigatorTypes::TYPE_ID_INVESTMENT;
    }

    if (cmdInt > -1) {
        wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, MENU_GOTOACCOUNT);
        event.SetInt(cmdInt);
        event.SetString(sData);
        wxPostEvent(w_frame, event);
        evt.Veto();  // Inhibit a wxEVT_WEBVIEW_NEWWINDOW_FEATURES event, which will crash!
    }
}

void DashboardPanel::onLinkClicked(wxWebViewEvent& event)
{
    const wxString& url = wxURI::Unescape(event.GetURL());
    if (!url.Contains("#"))
        return;

    wxLogDebug("{{{ DashboardPanel::onLinkClicked()");
    wxString name = url.AfterLast('#');
    wxLogDebug("Name = %s", name);

    //Convert the JSON string from database to a json object
    const wxString key = "HOME_PAGE_STATUS";
    wxString j_str = InfoModel::instance().getString(key, "{}");
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
    InfoModel::instance().setJdoc(key, j_doc);
    wxLogDebug("}}}");
}
