/*******************************************************
Copyright (C) 2014 Stefano Giorgio
Copyright (C) 2020 - 2022 Nikolay Akimov

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

#include "optionsettingsnet.h"
#include "constants.h"
#include "option.h"
#include "util.h"

#include <wx/hyperlink.h>
#include <wx/spinctrl.h>

/*******************************************************/
wxBEGIN_EVENT_TABLE(OptionSettingsNet, wxPanel)
    EVT_TEXT(ID_DIALOG_OPTIONS_TEXTCTRL_PROXY, OptionSettingsNet::OnProxyChanged)
    EVT_CHECKBOX(ID_DIALOG_OPTIONS_UPDATES_CHECK, OptionSettingsNet::OnUpdateCheckChanged)
wxEND_EVENT_TABLE()
/*******************************************************/

OptionSettingsNet::OptionSettingsNet()
{
}

OptionSettingsNet::OptionSettingsNet(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
{
    wxPanel::Create(parent, id, pos, size, style, name);
    Create();
}

OptionSettingsNet::~OptionSettingsNet()
{
}

void OptionSettingsNet::Create()
{
    wxBoxSizer* networkPanelSizer0 = new wxBoxSizer(wxVERTICAL);
    SetSizer(networkPanelSizer0);

    wxScrolledWindow* network_panel = new wxScrolledWindow(this, wxID_ANY);

    wxBoxSizer* networkPanelSizer = new wxBoxSizer(wxVERTICAL);
    network_panel->SetSizer(networkPanelSizer);
    networkPanelSizer0->Add(network_panel, wxSizerFlags(g_flagsExpand).Proportion(0));

    //WebApp settings
    wxStaticBox* WebAppStaticBox = new wxStaticBox(network_panel, wxID_STATIC, _t("WebApp"));
    SetBoldFont(WebAppStaticBox);
    wxStaticBoxSizer* WebAppStaticBoxSizer = new wxStaticBoxSizer(WebAppStaticBox, wxVERTICAL);
    wxFlexGridSizer* WebAppStaticBoxSizerGrid = new wxFlexGridSizer(0, 2, 0, 10);
    WebAppStaticBoxSizerGrid->AddGrowableCol(1);

    networkPanelSizer->Add(WebAppStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));
    WebAppStaticBoxSizer->Add(WebAppStaticBoxSizerGrid, wxSizerFlags(g_flagsExpand).Proportion(0));

    WebAppStaticBoxSizerGrid->Add(new wxStaticText(network_panel, wxID_STATIC, _t("URL")), g_flagsH);
    wxString WebAppURL = Model_Infotable::instance().getString("WEBAPPURL", "");
    wxTextCtrl* WebAppURLTextCtr = new wxTextCtrl(network_panel, ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPURL,
        WebAppURL, wxDefaultPosition, wxSize(300, -1));
    mmToolTip(WebAppURLTextCtr, _t("Specify the Web App URL without final slash"));
    WebAppStaticBoxSizerGrid->Add(WebAppURLTextCtr, 1, wxEXPAND | wxALL, 5);

    WebAppStaticBoxSizerGrid->Add(new wxStaticText(network_panel, wxID_STATIC, _t("GUID")), g_flagsH);
    wxString WebAppGUID = Model_Infotable::instance().getString("WEBAPPGUID", "");
    wxTextCtrl* WebAppGUIDTextCtr = new wxTextCtrl(network_panel, ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPGUID,
        WebAppGUID, wxDefaultPosition, wxSize(300, -1));
    mmToolTip(WebAppGUIDTextCtr, _t("Specify the Web App GUID"));
    WebAppGUIDTextCtr->SetHint("{XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}");
    WebAppStaticBoxSizerGrid->Add(WebAppGUIDTextCtr, 1, wxEXPAND | wxALL, 5);

    wxHyperlinkCtrl* WebAppLink = new wxHyperlinkCtrl(network_panel, wxID_STATIC, _t("More information about WebApp"), mmex::weblink::WebApp);
    WebAppStaticBoxSizer->Add(WebAppLink, wxSizerFlags(g_flagsV).Border(wxLEFT, 10));

    // Proxy Settings
    wxStaticBox* proxyStaticBox = new wxStaticBox(network_panel, wxID_STATIC, _t("Proxy"));
    SetBoldFont(proxyStaticBox);
    wxStaticBoxSizer* proxyStaticBoxSizer = new wxStaticBoxSizer(proxyStaticBox, wxVERTICAL);
    networkPanelSizer->Add(proxyStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxString proxyName = Model_Setting::instance().getString("PROXYIP", "");
    m_proxy_address = new wxTextCtrl(network_panel, ID_DIALOG_OPTIONS_TEXTCTRL_PROXY
        , proxyName, wxDefaultPosition, wxSize(150, -1));
    mmToolTip(m_proxy_address, _t("Specify the proxy IP address"));

    int proxyPort = Model_Setting::instance().getInt("PROXYPORT", 0);
    m_proxy_port = new wxSpinCtrl(network_panel, wxID_ANY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535, proxyPort);
    m_proxy_port->SetValue(proxyPort);
    mmToolTip(m_proxy_port, _t("Specify proxy port number"));

    wxFlexGridSizer* flex_sizer3 = new wxFlexGridSizer(0, 4, 0, 0);
    flex_sizer3->AddGrowableCol(1);
    flex_sizer3->Add(new wxStaticText(network_panel, wxID_STATIC, _t("Proxy")), g_flagsH);
    flex_sizer3->Add(m_proxy_address, 1, wxEXPAND | wxALL, 5);
    flex_sizer3->Add(new wxStaticText(network_panel, wxID_STATIC, _t("Port")), g_flagsH);
    flex_sizer3->Add(m_proxy_port, 1, wxEXPAND | wxALL, 5);

    proxyStaticBoxSizer->Add(flex_sizer3, wxSizerFlags(g_flagsExpand).Proportion(0));

    //Usage data send
    wxStaticBox* usageStaticBox = new wxStaticBox(network_panel, wxID_STATIC, _t("Usage Statistics"));
    SetBoldFont(usageStaticBox);
    wxStaticBoxSizer* usageStaticBoxSizer = new wxStaticBoxSizer(usageStaticBox, wxVERTICAL);
    networkPanelSizer->Add(usageStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_send_data = new wxCheckBox(network_panel, wxID_ANY
        , _t("Send anonymous statistics usage data"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_send_data->SetValue(Option::instance().getSendUsageStats());
    mmToolTip(m_send_data, _t("Enable to help us sending anonymous data about MMEX usage."));

    usageStaticBoxSizer->Add(m_send_data, g_flagsV);

     //  News updates
    wxStaticBox* newsStaticBox = new wxStaticBox(network_panel, wxID_STATIC, _t("News"));
    SetBoldFont(newsStaticBox);
    wxStaticBoxSizer* newsStaticBoxSizer = new wxStaticBoxSizer(newsStaticBox, wxVERTICAL);
    networkPanelSizer->Add(newsStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_check_news = new wxCheckBox(network_panel, wxID_ANY
        , _t("Check for latest news on startup"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_check_news->SetValue(Option::instance().getCheckNews());
    mmToolTip(m_check_news, _t("Enable to allow news to be checked on application start"));
    
    newsStaticBoxSizer->Add(m_check_news, g_flagsV);

    // Communication timeout
    wxStaticBox* timeoutStaticBox = new wxStaticBox(network_panel, wxID_STATIC, _t("Timeout"));
    SetBoldFont(timeoutStaticBox);
    wxStaticBoxSizer* timeoutStaticBoxSizer = new wxStaticBoxSizer(timeoutStaticBox, wxVERTICAL);
    networkPanelSizer->Add(timeoutStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    int nTimeout = Model_Setting::instance().getInt("NETWORKTIMEOUT", 10);
    m_network_timeout = new wxSpinCtrl(network_panel, wxID_ANY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 150, nTimeout);
    m_network_timeout->SetValue(nTimeout);
    mmToolTip(m_network_timeout, _t("Specify a network communication timeout value to use."));

    wxFlexGridSizer* flex_sizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    flex_sizer5->Add(new wxStaticText(network_panel, wxID_STATIC, _t("Seconds")), g_flagsH);
    flex_sizer5->Add(m_network_timeout, g_flagsH);

    timeoutStaticBoxSizer->Add(flex_sizer5, g_flagsV);

    //Updates check
    wxStaticBox* updateStaticBox = new wxStaticBox(network_panel, wxID_STATIC, _t("Updates"));
    SetBoldFont(updateStaticBox);
    wxStaticBoxSizer* updateStaticBoxSizer = new wxStaticBoxSizer(updateStaticBox, wxVERTICAL);
    networkPanelSizer->Add(updateStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_check_update = new wxCheckBox(network_panel, ID_DIALOG_OPTIONS_UPDATES_CHECK
        , _t("Check for updates at StartUp"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_check_update->SetValue(GetIniDatabaseCheckboxValue("UPDATECHECK", true));
    mmToolTip(m_check_update, _t("Enable to automatically check if new MMEX version is available at StartUp"));

    wxArrayString UpdatesType_;
    UpdatesType_.Add(_t("Stable"));
    UpdatesType_.Add(_t("Unstable"));
    m_update_source = new wxChoice(network_panel, wxID_ANY
        , wxDefaultPosition,wxDefaultSize, UpdatesType_);
    m_update_source->SetMinSize(wxSize(150, -1));
    m_update_source->SetSelection(Model_Setting::instance().getInt("UPDATESOURCE", 0));
    mmToolTip(m_update_source, _t("Updates source"));

    wxFlexGridSizer* UpdateSourceStaticBoxSizerGrid = new wxFlexGridSizer(0, 2, 0, 0);
    UpdateSourceStaticBoxSizerGrid->Add(m_check_update, g_flagsH);
    UpdateSourceStaticBoxSizerGrid->Add(m_update_source, g_flagsH);
    updateStaticBoxSizer->Add(UpdateSourceStaticBoxSizerGrid, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxCommandEvent evt;
    OptionSettingsNet::OnProxyChanged(evt);
    OptionSettingsNet::OnUpdateCheckChanged(evt);

    Fit();
    network_panel->SetMinSize(network_panel->GetBestVirtualSize());
    network_panel->SetScrollRate(6, 6);
}

void OptionSettingsNet::OnProxyChanged(wxCommandEvent& WXUNUSED(event))
{
    m_proxy_port->Enable(m_proxy_address->GetValue() != "");
}

void OptionSettingsNet::OnUpdateCheckChanged(wxCommandEvent& WXUNUSED(event))
{
    m_update_source->Enable(m_check_update->GetValue());
}

bool OptionSettingsNet::SaveSettings()
{
    Model_Setting::instance().setString("PROXYIP", m_proxy_address->GetValue().Trim(false).Trim());
    Model_Setting::instance().setInt("PROXYPORT", m_proxy_port->GetValue());

    wxTextCtrl* WebAppURL = static_cast<wxTextCtrl*>(FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPURL));
    Model_Infotable::instance().setString("WEBAPPURL", WebAppURL->GetValue().Trim(false).Trim());

    wxTextCtrl* WebAppGUID = static_cast<wxTextCtrl*>(FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPGUID));
    Model_Infotable::instance().setString("WEBAPPGUID", WebAppGUID->GetValue().Trim(false).Trim());

    Option::instance().setSendUsageStats(m_send_data->GetValue());
    Option::instance().setCheckNews(m_check_news->GetValue());

    Model_Setting::instance().setInt("NETWORKTIMEOUT", m_network_timeout->GetValue());

    Model_Setting::instance().setBool("UPDATECHECK", m_check_update->GetValue());
    Model_Setting::instance().setInt("UPDATESOURCE", m_update_source->GetSelection());

    return true;
}
