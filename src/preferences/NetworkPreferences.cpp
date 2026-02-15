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

#include "base/defs.h"
#include <wx/hyperlink.h>
#include <wx/spinctrl.h>

#include "base/constants.h"
#include "util/_util.h"

#include "model/PreferencesModel.h"

#include "NetworkPreferences.h"

/*******************************************************/
wxBEGIN_EVENT_TABLE(NetworkPreferences, wxPanel)
    EVT_TEXT(ID_DIALOG_OPTIONS_TEXTCTRL_PROXY, NetworkPreferences::OnProxyChanged)
    EVT_CHECKBOX(ID_DIALOG_OPTIONS_UPDATES_CHECK, NetworkPreferences::OnUpdateCheckChanged)
wxEND_EVENT_TABLE()
/*******************************************************/

NetworkPreferences::NetworkPreferences()
{
}

NetworkPreferences::NetworkPreferences(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
{
    wxPanel::Create(parent, id, pos, size, style, name);
    Create();
}

NetworkPreferences::~NetworkPreferences()
{
}

void NetworkPreferences::Create()
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
    m_send_data->SetValue(PreferencesModel::instance().getSendUsageStats());
    mmToolTip(m_send_data, _t("Enable to help us sending anonymous data about MMEX usage."));

    usageStaticBoxSizer->Add(m_send_data, g_flagsV);

     //  News updates
    wxStaticBox* newsStaticBox = new wxStaticBox(usageStaticBox, wxID_STATIC, _t("News"));
    wxStaticBoxSizer* newsStaticBoxSizer = new wxStaticBoxSizer(newsStaticBox, wxVERTICAL);
    networkPanelSizer->Add(newsStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_check_news = new wxCheckBox(newsStaticBox, wxID_ANY
        , _t("Check for latest news on startup"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_check_news->SetValue(PreferencesModel::instance().getCheckNews());
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
    NetworkPreferences::OnProxyChanged(evt);
    NetworkPreferences::OnUpdateCheckChanged(evt);

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

void NetworkPreferences::OnProxyChanged(wxCommandEvent& WXUNUSED(event))
{
    m_proxy_port->Enable(m_proxy_address->GetValue() != "");
}

void NetworkPreferences::OnUpdateCheckChanged(wxCommandEvent& WXUNUSED(event))
{
    m_update_source->Enable(m_check_update->GetValue());
}

bool NetworkPreferences::SaveSettings()
{
    SettingModel::instance().setString("PROXYIP", m_proxy_address->GetValue().Trim(false).Trim());
    SettingModel::instance().setInt("PROXYPORT", m_proxy_port->GetValue());

    wxTextCtrl* WebAppURL = static_cast<wxTextCtrl*>(FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPURL));
    InfoModel::instance().setString("WEBAPPURL", WebAppURL->GetValue().Trim(false).Trim());

    wxTextCtrl* WebAppGUID = static_cast<wxTextCtrl*>(FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPGUID));
    InfoModel::instance().setString("WEBAPPGUID", WebAppGUID->GetValue().Trim(false).Trim());

    PreferencesModel::instance().setSendUsageStats(m_send_data->GetValue());
    PreferencesModel::instance().setCheckNews(m_check_news->GetValue());

    SettingModel::instance().setInt("NETWORKTIMEOUT", m_network_timeout->GetValue());

    SettingModel::instance().setBool("UPDATECHECK", m_check_update->GetValue());
    SettingModel::instance().setInt("UPDATESOURCE", m_update_source->GetSelection());

    return true;
}
