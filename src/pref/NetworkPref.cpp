/*******************************************************
Copyright (C) 2014 Stefano Giorgio
Copyright (C) 2020 - 2022 Nikolay Akimov
Copyright (C) 2025 Klaus Wich

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

#include "base/_defs.h"
#include <wx/hyperlink.h>
#include <wx/spinctrl.h>
#include <wx/sstream.h>
#include <rapidjson/document.h>
#include <curl/curl.h>

#include "base/_constants.h"
#include "util/_util.h"

#include "model/PrefModel.h"

#include "NetworkPref.h"
#include "pocketbase/PocketBaseSync.h"

namespace {
size_t PocketBaseWrite(char* data, size_t size, size_t count, void* context)
{
    static_cast<std::string*>(context)->append(data, size * count);
    return size * count;
}

}

/*******************************************************/
wxBEGIN_EVENT_TABLE(NetworkPref, wxPanel)
    EVT_TEXT(ID_DIALOG_OPTIONS_TEXTCTRL_PROXY, NetworkPref::OnProxyChanged)
    EVT_CHECKBOX(ID_DIALOG_OPTIONS_UPDATES_CHECK, NetworkPref::OnUpdateCheckChanged)
    EVT_BUTTON(wxID_HIGHEST + 80, NetworkPref::OnPocketBaseLogin)
    EVT_BUTTON(wxID_HIGHEST + 82, NetworkPref::OnPocketBaseLogout)
    EVT_BUTTON(wxID_HIGHEST + 83, NetworkPref::OnPocketBaseSyncCurrencies)
    EVT_BUTTON(wxID_HIGHEST + 84, NetworkPref::OnPocketBaseSyncAccounts)
    EVT_BUTTON(wxID_HIGHEST + 85, NetworkPref::OnPocketBaseSyncCategories)
    EVT_BUTTON(wxID_HIGHEST + 86, NetworkPref::OnPocketBaseSyncPayees)
    EVT_BUTTON(wxID_HIGHEST + 87, NetworkPref::OnPocketBaseSyncTransactions)
    EVT_BUTTON(wxID_HIGHEST + 88, NetworkPref::OnPocketBaseSyncInvestments)
wxEND_EVENT_TABLE()
/*******************************************************/

NetworkPref::NetworkPref()
{
}

NetworkPref::NetworkPref(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
{
    wxPanel::Create(parent, id, pos, size, style, name);
    Create();
}

NetworkPref::~NetworkPref()
{
}

void NetworkPref::Create()
{
    wxBoxSizer* networkPanelSizer0 = new wxBoxSizer(wxVERTICAL);
    SetSizer(networkPanelSizer0);

    wxScrolledWindow* network_panel = new wxScrolledWindow(this, wxID_ANY);

    wxBoxSizer* networkPanelSizer = new wxBoxSizer(wxVERTICAL);
    network_panel->SetSizer(networkPanelSizer);
    networkPanelSizer0->Add(network_panel, wxSizerFlags(g_flagsExpand).Proportion(0));

    //WebApp settings
    wxStaticBox* WebAppStaticBox = new wxStaticBox(network_panel, wxID_STATIC, _t("WebApp"));
    wxStaticBoxSizer* WebAppStaticBoxSizer = new wxStaticBoxSizer(WebAppStaticBox, wxVERTICAL);
    wxFlexGridSizer* WebAppStaticBoxSizerGrid = new wxFlexGridSizer(0, 2, 0, 10);
    WebAppStaticBoxSizerGrid->AddGrowableCol(1);

    networkPanelSizer->Add(WebAppStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));
    WebAppStaticBoxSizer->Add(WebAppStaticBoxSizerGrid, wxSizerFlags(g_flagsExpand).Proportion(0));

    WebAppStaticBoxSizerGrid->Add(new wxStaticText(WebAppStaticBox, wxID_STATIC, _t("URL")), g_flagsH);
    wxString WebAppURL = InfoModel::instance().getString("WEBAPPURL", "");
    wxTextCtrl* WebAppURLTextCtr = new wxTextCtrl(WebAppStaticBox, ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPURL,
        WebAppURL, wxDefaultPosition, wxSize(300, -1));
    mmToolTip(WebAppURLTextCtr, _t("Specify the Web App URL without final slash"));
    WebAppStaticBoxSizerGrid->Add(WebAppURLTextCtr, 1, wxEXPAND | wxALL, 5);

    WebAppStaticBoxSizerGrid->Add(new wxStaticText(WebAppStaticBox, wxID_STATIC, _t("GUID")), g_flagsH);
    wxString WebAppGUID = InfoModel::instance().getString("WEBAPPGUID", "");
    wxTextCtrl* WebAppGUIDTextCtr = new wxTextCtrl(WebAppStaticBox, ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPGUID,
        WebAppGUID, wxDefaultPosition, wxSize(300, -1));
    mmToolTip(WebAppGUIDTextCtr, _t("Specify the Web App GUID"));
    WebAppGUIDTextCtr->SetHint("{XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}");
    WebAppStaticBoxSizerGrid->Add(WebAppGUIDTextCtr, 1, wxEXPAND | wxALL, 5);

    wxHyperlinkCtrl* WebAppLink = new wxHyperlinkCtrl(WebAppStaticBox, wxID_STATIC, _t("More information about WebApp"), mmex::weblink::WebApp);
    WebAppStaticBoxSizer->Add(WebAppLink, wxSizerFlags(g_flagsV).Border(wxLEFT, 10));

    // PocketBase connection settings. Credentials and auth tokens are deliberately
    // not stored here; the URL is only the endpoint used by the sync client.
    wxStaticBox* pocketBaseBox = new wxStaticBox(network_panel, wxID_STATIC, "PocketBase");
    wxStaticBoxSizer* pocketBaseSizer = new wxStaticBoxSizer(pocketBaseBox, wxVERTICAL);
    wxFlexGridSizer* pocketBaseGrid = new wxFlexGridSizer(0, 2, 0, 10);
    pocketBaseGrid->AddGrowableCol(1);
    networkPanelSizer->Add(pocketBaseSizer, wxSizerFlags(g_flagsExpand).Proportion(0));
    pocketBaseSizer->Add(pocketBaseGrid, wxSizerFlags(g_flagsExpand).Proportion(0));
    pocketBaseGrid->Add(new wxStaticText(pocketBaseBox, wxID_STATIC, "Instance URL"), g_flagsH);
    m_pocketbase_url = new wxTextCtrl(pocketBaseBox, wxID_ANY,
        SettingModel::instance().getString("POCKETBASE_URL", ""), wxDefaultPosition, wxSize(300, -1));
    pocketBaseGrid->Add(m_pocketbase_url, 1, wxEXPAND | wxALL, 5);
    pocketBaseGrid->Add(new wxStaticText(pocketBaseBox, wxID_STATIC, "Account email"), g_flagsH);
    m_pocketbase_email = new wxTextCtrl(pocketBaseBox, wxID_ANY,
        SettingModel::instance().getString("POCKETBASE_EMAIL", ""), wxDefaultPosition, wxSize(300, -1));
    pocketBaseGrid->Add(m_pocketbase_email, 1, wxEXPAND | wxALL, 5);
    pocketBaseGrid->Add(new wxStaticText(pocketBaseBox, wxID_STATIC, "Password"), g_flagsH);
    m_pocketbase_password = new wxTextCtrl(pocketBaseBox, wxID_ANY, "", wxDefaultPosition,
        wxSize(300, -1), wxTE_PASSWORD);
    pocketBaseGrid->Add(m_pocketbase_password, 1, wxEXPAND | wxALL, 5);
    m_pocketbase_login = new wxButton(pocketBaseBox, wxID_HIGHEST + 80,
        SettingModel::instance().getString("POCKETBASE_TOKEN", "").empty() ? "Sign in" : "Check session");
    m_pocketbase_logout = new wxButton(pocketBaseBox, wxID_HIGHEST + 82, "Sign out");
    m_pocketbase_logout->Enable(!SettingModel::instance().getString("POCKETBASE_TOKEN", "").empty());
    pocketBaseSizer->Add(new wxStaticText(pocketBaseBox, wxID_STATIC, "Session"),
        wxSizerFlags(g_flagsV).Border(wxLEFT | wxRIGHT | wxTOP, 8));
    wxBoxSizer* pocketBaseSessionActions = new wxBoxSizer(wxHORIZONTAL);
    pocketBaseSessionActions->Add(m_pocketbase_login, wxSizerFlags(1).Expand().Border(wxRIGHT, 8));
    pocketBaseSessionActions->Add(m_pocketbase_logout, wxSizerFlags(1).Expand());
    pocketBaseSizer->Add(pocketBaseSessionActions,
        wxSizerFlags(g_flagsExpand).Border(wxLEFT | wxRIGHT | wxBOTTOM, 8));
    m_pocketbase_status = new wxStaticText(pocketBaseBox, wxID_ANY,
        SettingModel::instance().getString("POCKETBASE_TOKEN", "").empty() ? "Not signed in" : "Saved session available");
    m_pocketbase_status->SetForegroundColour(wxSystemSettings::GetColour(
        SettingModel::instance().getString("POCKETBASE_TOKEN", "").empty() ? wxSYS_COLOUR_GRAYTEXT : wxSYS_COLOUR_WINDOWTEXT));
    pocketBaseSizer->Add(m_pocketbase_status, wxSizerFlags(g_flagsV).Border(wxLEFT | wxRIGHT | wxBOTTOM, 8));

    pocketBaseSizer->Add(new wxStaticText(pocketBaseBox, wxID_STATIC, "Basic data"),
        wxSizerFlags(g_flagsV).Border(wxLEFT | wxRIGHT | wxTOP, 8));
    m_pocketbase_sync_currencies = new wxButton(pocketBaseBox, wxID_HIGHEST + 83, "Sync currencies");
    m_pocketbase_sync_currencies->Enable(!SettingModel::instance().getString("POCKETBASE_TOKEN", "").empty());
    m_pocketbase_sync_accounts = new wxButton(pocketBaseBox, wxID_HIGHEST + 84, "Sync accounts");
    m_pocketbase_sync_accounts->Enable(!SettingModel::instance().getString("POCKETBASE_TOKEN", "").empty());
    m_pocketbase_sync_categories = new wxButton(pocketBaseBox, wxID_HIGHEST + 85, "Sync categories");
    m_pocketbase_sync_categories->Enable(!SettingModel::instance().getString("POCKETBASE_TOKEN", "").empty());
    m_pocketbase_sync_payees = new wxButton(pocketBaseBox, wxID_HIGHEST + 86, "Sync payees");
    m_pocketbase_sync_payees->Enable(!SettingModel::instance().getString("POCKETBASE_TOKEN", "").empty());
    m_pocketbase_sync_transactions = new wxButton(pocketBaseBox, wxID_HIGHEST + 87, "Sync transactions");
    m_pocketbase_sync_transactions->Enable(!SettingModel::instance().getString("POCKETBASE_TOKEN", "").empty());
    m_pocketbase_sync_investments = new wxButton(pocketBaseBox, wxID_HIGHEST + 88, "Sync investments");
    m_pocketbase_sync_investments->Enable(!SettingModel::instance().getString("POCKETBASE_TOKEN", "").empty());
    wxGridSizer* pocketBaseSyncActions = new wxGridSizer(2, 8, 8);
    pocketBaseSyncActions->Add(m_pocketbase_sync_currencies, wxSizerFlags(1).Expand());
    pocketBaseSyncActions->Add(m_pocketbase_sync_accounts, wxSizerFlags(1).Expand());
    pocketBaseSyncActions->Add(m_pocketbase_sync_categories, wxSizerFlags(1).Expand());
    pocketBaseSyncActions->Add(m_pocketbase_sync_payees, wxSizerFlags(1).Expand());
    pocketBaseSizer->Add(pocketBaseSyncActions,
        wxSizerFlags(g_flagsExpand).Border(wxLEFT | wxRIGHT | wxBOTTOM, 8));
    pocketBaseSizer->Add(new wxStaticText(pocketBaseBox, wxID_STATIC, "Transactions"),
        wxSizerFlags(g_flagsV).Border(wxLEFT | wxRIGHT | wxTOP, 8));
    pocketBaseSizer->Add(m_pocketbase_sync_transactions,
        wxSizerFlags(g_flagsExpand).Border(wxLEFT | wxRIGHT | wxBOTTOM, 8));
    pocketBaseSizer->Add(new wxStaticText(pocketBaseBox, wxID_STATIC, "Investments"),
        wxSizerFlags(g_flagsV).Border(wxLEFT | wxRIGHT | wxTOP, 8));
    pocketBaseSizer->Add(m_pocketbase_sync_investments,
        wxSizerFlags(g_flagsExpand).Border(wxLEFT | wxRIGHT | wxBOTTOM, 8));
    pocketBaseSizer->Add(new wxStaticText(pocketBaseBox, wxID_ANY,
        "Syncs assets, stocks, price history, and their linked transactions and share lot details."),
        wxSizerFlags(g_flagsV).Border(wxLEFT | wxRIGHT | wxBOTTOM, 8));
    pocketBaseSizer->Add(new wxStaticText(pocketBaseBox, wxID_ANY,
        "Use HTTPS for remote instances. PocketBase sync is configured separately from the local database."),
        wxSizerFlags(g_flagsV).Border(wxLEFT | wxRIGHT | wxBOTTOM, 8));

    // Proxy Settings
    wxStaticBox* proxyStaticBox = new wxStaticBox(network_panel, wxID_STATIC, _t("Proxy"));
    wxStaticBoxSizer* proxyStaticBoxSizer = new wxStaticBoxSizer(proxyStaticBox, wxVERTICAL);
    networkPanelSizer->Add(proxyStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxString proxyName = SettingModel::instance().getString("PROXYIP", "");
    m_proxy_address = new wxTextCtrl(proxyStaticBox, ID_DIALOG_OPTIONS_TEXTCTRL_PROXY
        , proxyName, wxDefaultPosition, wxSize(150, -1));
    mmToolTip(m_proxy_address, _t("Specify the proxy IP address"));

    int proxyPort = SettingModel::instance().getInt("PROXYPORT", 0);
    m_proxy_port = new wxSpinCtrl(proxyStaticBox, wxID_ANY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535, proxyPort);
    m_proxy_port->SetValue(proxyPort);
    mmToolTip(m_proxy_port, _t("Specify proxy port number"));

    wxFlexGridSizer* flex_sizer3 = new wxFlexGridSizer(0, 4, 0, 0);
    flex_sizer3->AddGrowableCol(1);
    flex_sizer3->Add(new wxStaticText(proxyStaticBox, wxID_STATIC, _t("Proxy")), g_flagsH);
    flex_sizer3->Add(m_proxy_address, 1, wxEXPAND | wxALL, 5);
    flex_sizer3->Add(new wxStaticText(proxyStaticBox, wxID_STATIC, _t("Port")), g_flagsH);
    flex_sizer3->Add(m_proxy_port, 1, wxEXPAND | wxALL, 5);

    proxyStaticBoxSizer->Add(flex_sizer3, wxSizerFlags(g_flagsExpand).Proportion(0));

    //Usage data send
    wxStaticBox* usageStaticBox = new wxStaticBox(network_panel, wxID_STATIC, _t("Usage Statistics"));
    wxStaticBoxSizer* usageStaticBoxSizer = new wxStaticBoxSizer(usageStaticBox, wxVERTICAL);
    networkPanelSizer->Add(usageStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_send_data = new wxCheckBox(usageStaticBox, wxID_ANY
        , _t("Send anonymous statistics usage data"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_send_data->SetValue(PrefModel::instance().getSendUsageStats());
    mmToolTip(m_send_data, _t("Enable to help us sending anonymous data about MMEX usage."));

    usageStaticBoxSizer->Add(m_send_data, g_flagsV);

     //  News updates
    wxStaticBox* newsStaticBox = new wxStaticBox(usageStaticBox, wxID_STATIC, _t("News"));
    wxStaticBoxSizer* newsStaticBoxSizer = new wxStaticBoxSizer(newsStaticBox, wxVERTICAL);
    networkPanelSizer->Add(newsStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_check_news = new wxCheckBox(newsStaticBox, wxID_ANY
        , _t("Check for latest news on startup"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_check_news->SetValue(PrefModel::instance().getCheckNews());
    mmToolTip(m_check_news, _t("Enable to allow news to be checked on application start"));

    newsStaticBoxSizer->Add(m_check_news, g_flagsV);

    // Communication timeout
    wxStaticBox* timeoutStaticBox = new wxStaticBox(network_panel, wxID_STATIC, _t("Timeout"));
    wxStaticBoxSizer* timeoutStaticBoxSizer = new wxStaticBoxSizer(timeoutStaticBox, wxVERTICAL);
    networkPanelSizer->Add(timeoutStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    int nTimeout = SettingModel::instance().getInt("NETWORKTIMEOUT", 10);
    m_network_timeout = new wxSpinCtrl(timeoutStaticBox, wxID_ANY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 150, nTimeout);
    m_network_timeout->SetValue(nTimeout);
    mmToolTip(m_network_timeout, _t("Specify a network communication timeout value to use."));

    wxFlexGridSizer* flex_sizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    flex_sizer5->Add(new wxStaticText(timeoutStaticBox, wxID_STATIC, _t("Seconds")), g_flagsH);
    flex_sizer5->Add(m_network_timeout, g_flagsH);

    timeoutStaticBoxSizer->Add(flex_sizer5, g_flagsV);

    //Updates check
    wxStaticBox* updateStaticBox = new wxStaticBox(network_panel, wxID_STATIC, _t("Updates"));
    wxStaticBoxSizer* updateStaticBoxSizer = new wxStaticBoxSizer(updateStaticBox, wxVERTICAL);
    networkPanelSizer->Add(updateStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_check_update = new wxCheckBox(updateStaticBox, ID_DIALOG_OPTIONS_UPDATES_CHECK
        , _t("Check for updates at StartUp"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_check_update->SetValue(GetIniDatabaseCheckboxValue("UPDATECHECK", true));
    mmToolTip(m_check_update, _t("Enable to automatically check if new MMEX version is available at StartUp"));

    wxArrayString UpdatesType_;
    UpdatesType_.Add(_t("Stable"));
    UpdatesType_.Add(_t("Unstable"));
    m_update_source = new wxChoice(updateStaticBox, wxID_ANY
        , wxDefaultPosition,wxDefaultSize, UpdatesType_);
    m_update_source->SetMinSize(wxSize(150, -1));
    m_update_source->SetSelection(SettingModel::instance().getInt("UPDATESOURCE", 0));
    mmToolTip(m_update_source, _t("Updates source"));

    wxFlexGridSizer* UpdateSourceStaticBoxSizerGrid = new wxFlexGridSizer(0, 2, 0, 0);
    UpdateSourceStaticBoxSizerGrid->Add(m_check_update, g_flagsH);
    UpdateSourceStaticBoxSizerGrid->Add(m_update_source, g_flagsH);
    updateStaticBoxSizer->Add(UpdateSourceStaticBoxSizerGrid, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxCommandEvent evt;
    NetworkPref::OnProxyChanged(evt);
    NetworkPref::OnUpdateCheckChanged(evt);

    SetBoldFontToStaticBoxHeader(WebAppStaticBox);
    SetBoldFontToStaticBoxHeader(proxyStaticBox);
    SetBoldFontToStaticBoxHeader(usageStaticBox);
    SetBoldFontToStaticBoxHeader(newsStaticBox);
    SetBoldFontToStaticBoxHeader(timeoutStaticBox);
    SetBoldFontToStaticBoxHeader(updateStaticBox);

    Fit();
    network_panel->SetMinSize(network_panel->GetBestVirtualSize());
    network_panel->SetScrollRate(6, 6);
}

void NetworkPref::OnProxyChanged(wxCommandEvent& WXUNUSED(event))
{
    m_proxy_port->Enable(m_proxy_address->GetValue() != "");
}

void NetworkPref::OnUpdateCheckChanged(wxCommandEvent& WXUNUSED(event))
{
    m_update_source->Enable(m_check_update->GetValue());
}

bool NetworkPref::SaveSettings()
{
    SettingModel::instance().saveString("POCKETBASE_URL", m_pocketbase_url->GetValue().Trim(false).Trim());
    SettingModel::instance().saveString("POCKETBASE_EMAIL", m_pocketbase_email->GetValue().Trim(false).Trim());
    SettingModel::instance().saveString("PROXYIP", m_proxy_address->GetValue().Trim(false).Trim());
    SettingModel::instance().saveInt("PROXYPORT", m_proxy_port->GetValue());

    wxTextCtrl* WebAppURL = static_cast<wxTextCtrl*>(FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPURL));
    InfoModel::instance().saveString("WEBAPPURL", WebAppURL->GetValue().Trim(false).Trim());

    wxTextCtrl* WebAppGUID = static_cast<wxTextCtrl*>(FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPGUID));
    InfoModel::instance().saveString("WEBAPPGUID", WebAppGUID->GetValue().Trim(false).Trim());

    PrefModel::instance().saveSendUsageStats(m_send_data->GetValue());
    PrefModel::instance().saveCheckNews(m_check_news->GetValue());

    SettingModel::instance().saveInt("NETWORKTIMEOUT", m_network_timeout->GetValue());

    SettingModel::instance().saveBool("UPDATECHECK", m_check_update->GetValue());
    SettingModel::instance().saveInt("UPDATESOURCE", m_update_source->GetSelection());

    return true;
}

void NetworkPref::OnPocketBaseLogin(wxCommandEvent&)
{
    wxString base = m_pocketbase_url->GetValue().Trim(false).Trim();
    while (base.EndsWith("/")) base.RemoveLast();
    SettingModel::instance().saveString("POCKETBASE_URL", base);
    SettingModel::instance().saveString("POCKETBASE_EMAIL", m_pocketbase_email->GetValue().Trim(false).Trim());
    if (!base.StartsWith("https://") && !base.StartsWith("http://localhost") && !base.StartsWith("http://127.0.0.1")) {
        m_pocketbase_status->SetLabel("Use HTTPS for remote PocketBase instances.");
        return;
    }
    const wxString token = SettingModel::instance().getString("POCKETBASE_TOKEN", "");
    m_pocketbase_auth_check = !token.empty() && m_pocketbase_password->GetValue().empty();
    const wxString endpoint = base + (m_pocketbase_auth_check
        ? "/api/collections/users/auth-refresh"
        : "/api/collections/users/auth-with-password");
    std::string requestBody;
    if (m_pocketbase_auth_check) {
        requestBody = "{}";
    } else {
        rapidjson::Document body;
        body.SetObject();
        auto& alloc = body.GetAllocator();
        const auto email = m_pocketbase_email->GetValue().Trim(false).Trim().utf8_string();
        const auto password = m_pocketbase_password->GetValue().utf8_string();
        body.AddMember("identity", rapidjson::Value(email.c_str(), alloc), alloc);
        body.AddMember("password", rapidjson::Value(password.c_str(), alloc), alloc);
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        body.Accept(writer);
        requestBody = buffer.GetString();
    }
    m_pocketbase_password->Clear();
    m_pocketbase_status->SetLabel("Connecting…");
    m_pocketbase_status->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
    m_pocketbase_login->Disable();
    CURL* curl = curl_easy_init();
    std::string responseBody;
    long status = 0;
    if (curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (m_pocketbase_auth_check) {
            const auto auth = std::string("Authorization: ") + token.utf8_string().data();
            headers = curl_slist_append(headers, auth.c_str());
        }
        const auto url = endpoint.utf8_string();
        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(requestBody.size()));
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, PocketBaseWrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        const CURLcode result = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        if (result != CURLE_OK) {
            m_pocketbase_login->Enable();
            m_pocketbase_status->SetLabel("PocketBase connection failed: " + wxString::FromUTF8(curl_easy_strerror(result)));
            return;
        }
    } else {
        m_pocketbase_login->Enable();
        m_pocketbase_status->SetLabel("Could not initialize the HTTP client.");
        return;
    }
    if (status < 200 || status >= 300) {
        m_pocketbase_login->Enable();
        rapidjson::Document error;
        error.Parse(responseBody.c_str());
        wxString message = wxString::Format("PocketBase request failed (%ld).", status);
        if (!error.HasParseError() && error.IsObject() && error.HasMember("message") && error["message"].IsString())
            message += " " + wxString::FromUTF8(error["message"].GetString());
        else if (!responseBody.empty()) message += " " + wxString::FromUTF8(responseBody.c_str());
        m_pocketbase_status->SetLabel(message);
        return;
    }
    rapidjson::Document result;
    result.Parse(responseBody.c_str());
    if (result.HasParseError() || !result.IsObject() || !result.HasMember("token") || !result["token"].IsString()) {
        m_pocketbase_status->SetLabel("PocketBase returned an invalid authentication response.");
        return;
    }
    SettingModel::instance().saveString("POCKETBASE_TOKEN", wxString::FromUTF8(result["token"].GetString()));
    m_pocketbase_status->SetLabel("Signed in. Session saved in this MMEX database.");
    m_pocketbase_status->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
    m_pocketbase_login->SetLabel("Check session");
    m_pocketbase_login->Enable();
    m_pocketbase_logout->Enable();
    m_pocketbase_sync_currencies->Enable();
    m_pocketbase_sync_accounts->Enable();
    m_pocketbase_sync_categories->Enable();
    m_pocketbase_sync_payees->Enable();
    m_pocketbase_sync_transactions->Enable();
    m_pocketbase_sync_investments->Enable();
    m_pocketbase_auth_check = false;
}

void NetworkPref::OnPocketBaseLogout(wxCommandEvent&)
{
    SettingModel::instance().saveString("POCKETBASE_TOKEN", "");
    m_pocketbase_login->SetLabel("Sign in");
    m_pocketbase_logout->Disable();
    m_pocketbase_sync_currencies->Disable();
    m_pocketbase_sync_accounts->Disable();
    m_pocketbase_sync_categories->Disable();
    m_pocketbase_sync_payees->Disable();
    m_pocketbase_sync_transactions->Disable();
    m_pocketbase_sync_investments->Disable();
    m_pocketbase_status->SetLabel("Signed out.");
    m_pocketbase_status->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
}

void NetworkPref::OnPocketBaseSyncCurrencies(wxCommandEvent&)
{
    m_pocketbase_sync_currencies->Disable();
    auto result = PocketBaseSyncService::syncCurrencies(m_pocketbase_url->GetValue().Trim(false).Trim(), SettingModel::instance().getString("POCKETBASE_TOKEN", ""));
    m_pocketbase_status->SetLabel(result.message);
    m_pocketbase_sync_currencies->Enable(!SettingModel::instance().getString("POCKETBASE_TOKEN", "").empty());
}

void NetworkPref::OnPocketBaseSyncAccounts(wxCommandEvent&)
{
    m_pocketbase_sync_accounts->Disable();
    auto result = PocketBaseSyncService::syncAccounts(m_pocketbase_url->GetValue().Trim(false).Trim(), SettingModel::instance().getString("POCKETBASE_TOKEN", ""));
    m_pocketbase_status->SetLabel(result.message);
    m_pocketbase_sync_accounts->Enable(!SettingModel::instance().getString("POCKETBASE_TOKEN", "").empty());
}

void NetworkPref::OnPocketBaseSyncCategories(wxCommandEvent&)
{
    m_pocketbase_sync_categories->Disable();
    auto result = PocketBaseSyncService::syncCategories(m_pocketbase_url->GetValue().Trim(false).Trim(), SettingModel::instance().getString("POCKETBASE_TOKEN", ""));
    m_pocketbase_status->SetLabel(result.message);
    m_pocketbase_sync_categories->Enable(!SettingModel::instance().getString("POCKETBASE_TOKEN", "").empty());
}

void NetworkPref::OnPocketBaseSyncPayees(wxCommandEvent&)
{
    m_pocketbase_sync_payees->Disable();
    auto result = PocketBaseSyncService::syncPayees(m_pocketbase_url->GetValue().Trim(false).Trim(), SettingModel::instance().getString("POCKETBASE_TOKEN", ""));
    m_pocketbase_status->SetLabel(result.message);
    m_pocketbase_sync_payees->Enable(!SettingModel::instance().getString("POCKETBASE_TOKEN", "").empty());
}

void NetworkPref::OnPocketBaseSyncTransactions(wxCommandEvent&)
{
    m_pocketbase_sync_transactions->Disable();
    auto result = PocketBaseSyncService::syncTransactions(m_pocketbase_url->GetValue().Trim(false).Trim(), SettingModel::instance().getString("POCKETBASE_TOKEN", ""));
    m_pocketbase_status->SetLabel(result.message);
    m_pocketbase_sync_transactions->Enable(!SettingModel::instance().getString("POCKETBASE_TOKEN", "").empty());
}

void NetworkPref::OnPocketBaseSyncInvestments(wxCommandEvent&)
{
    m_pocketbase_sync_investments->Disable();
    auto result = PocketBaseSyncService::syncInvestments(m_pocketbase_url->GetValue().Trim(false).Trim(), SettingModel::instance().getString("POCKETBASE_TOKEN", ""));
    m_pocketbase_status->SetLabel(result.message);
    m_pocketbase_sync_investments->Enable(!SettingModel::instance().getString("POCKETBASE_TOKEN", "").empty());
}

