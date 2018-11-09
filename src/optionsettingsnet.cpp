/*******************************************************
Copyright (C) 2014 Stefano Giorgio

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
#include "webapp.h"


#include <wx/hyperlink.h>
#include <wx/spinctrl.h>

/*******************************************************/
wxBEGIN_EVENT_TABLE(OptionSettingsNet, wxPanel)
    EVT_TEXT(ID_DIALOG_OPTIONS_TEXTCTRL_PROXY, OptionSettingsNet::OnProxyChanged)
#ifdef MMEX_WEBSERVER
    EVT_CHECKBOX(ID_DIALOG_OPTIONS_ENABLE_WEBSERVER, OptionSettingsNet::OnEnableWebserverChanged)
#endif
    EVT_BUTTON(ID_DIALOG_OPTIONS_BUTTON_WEBAPP_TEST, OptionSettingsNet::OnWebAppTest)
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
    wxBoxSizer* networkPanelSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(networkPanelSizer);

    //WebApp settings
    wxStaticBox* WebAppStaticBox = new wxStaticBox(this, wxID_STATIC, _("WebApp Settings"));
    SetBoldFont(WebAppStaticBox);
    wxStaticBoxSizer* WebAppStaticBoxSizer = new wxStaticBoxSizer(WebAppStaticBox, wxVERTICAL);
    wxFlexGridSizer* WebAppStaticBoxSizerGrid = new wxFlexGridSizer(0, 2, 0, 10);
    networkPanelSizer->Add(WebAppStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));
    WebAppStaticBoxSizer->Add(WebAppStaticBoxSizerGrid, wxSizerFlags(g_flagsExpand).Proportion(0));

    WebAppStaticBoxSizerGrid->Add(new wxStaticText(this, wxID_STATIC, _("Url")), g_flagsH);
    wxString WebAppURL = Model_Infotable::instance().GetStringInfo("WEBAPPURL", "");
    wxTextCtrl* WebAppURLTextCtr = new wxTextCtrl(this, ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPURL,
        WebAppURL, wxDefaultPosition, wxSize(300, -1));
    WebAppURLTextCtr->SetToolTip(_("Specify the Web App URL without final slash"));
    WebAppStaticBoxSizerGrid->Add(WebAppURLTextCtr, 1, wxEXPAND | wxALL, 5);

    WebAppStaticBoxSizerGrid->Add(new wxStaticText(this, wxID_STATIC, _("GUID")), g_flagsH);
    wxString WebAppGUID = Model_Infotable::instance().GetStringInfo("WEBAPPGUID", "");
    wxTextCtrl* WebAppGUIDTextCtr = new wxTextCtrl(this, ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPGUID,
        WebAppGUID, wxDefaultPosition, wxSize(300, -1));
    WebAppGUIDTextCtr->SetToolTip(_("Specify the Web App GUID"));
    WebAppStaticBoxSizerGrid->Add(WebAppGUIDTextCtr, 1, wxEXPAND | wxALL, 5);

    wxFlexGridSizer* WebAppStaticBoxSizerGridBottom = new wxFlexGridSizer(0, 2, 0, 10);
    WebAppStaticBoxSizer->Add(WebAppStaticBoxSizerGridBottom, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxButton* WebAppTestButton = new wxButton(this, ID_DIALOG_OPTIONS_BUTTON_WEBAPP_TEST, _("Test connection"));
    WebAppStaticBoxSizerGridBottom->Add(WebAppTestButton, 1, wxEXPAND | wxALL);

    wxHyperlinkCtrl* WebAppLink = new wxHyperlinkCtrl(this, wxID_STATIC, _("More information about WebApp"), mmex::weblink::WebApp);
    WebAppStaticBoxSizerGridBottom->Add(WebAppLink, 1, wxEXPAND | wxALL);

    // Proxy Settings
    wxStaticBox* proxyStaticBox = new wxStaticBox(this, wxID_STATIC, _("Proxy Settings"));
    SetBoldFont(proxyStaticBox);
    wxStaticBoxSizer* proxyStaticBoxSizer = new wxStaticBoxSizer(proxyStaticBox, wxVERTICAL);
    networkPanelSizer->Add(proxyStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxString proxyName = Model_Setting::instance().GetStringSetting("PROXYIP", "");
    m_proxy_address = new wxTextCtrl(this, ID_DIALOG_OPTIONS_TEXTCTRL_PROXY
        , proxyName, wxDefaultPosition, wxSize(150, -1));
    m_proxy_address->SetToolTip(_("Specify the proxy IP address"));

    int proxyPort = Model_Setting::instance().GetIntSetting("PROXYPORT", 0);
    m_proxy_port = new wxSpinCtrl(this, wxID_ANY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535, proxyPort);
    m_proxy_port->SetValue(proxyPort);
    m_proxy_port->SetToolTip(_("Specify proxy port number"));

    wxFlexGridSizer* flex_sizer3 = new wxFlexGridSizer(0, 4, 0, 0);
    flex_sizer3->Add(new wxStaticText(this, wxID_STATIC, _("Proxy")), g_flagsH);
    flex_sizer3->Add(m_proxy_address, g_flagsH);
    flex_sizer3->Add(new wxStaticText(this, wxID_STATIC, _("Port")), g_flagsH);
    flex_sizer3->Add(m_proxy_port, g_flagsH);

    proxyStaticBoxSizer->Add(flex_sizer3, g_flagsV);

#ifdef MMEX_WEBSERVER
    // Web Server Settings
    wxStaticBox* webserverStaticBox = new wxStaticBox(this, wxID_STATIC, _("Web Server"));
    SetBoldFont(webserverStaticBox);
    wxStaticBoxSizer* webserverStaticBoxSizer = new wxStaticBoxSizer(webserverStaticBox, wxVERTICAL);
    networkPanelSizer->Add(webserverStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_webserver_checkbox = new wxCheckBox(this, ID_DIALOG_OPTIONS_ENABLE_WEBSERVER
        , _("Enable"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_webserver_checkbox->SetValue(GetIniDatabaseCheckboxValue("ENABLEWEBSERVER", false));
    m_webserver_checkbox->SetToolTip(_("Enable internal web server when MMEX Starts."));

    int webserverPort = Model_Setting::instance().GetIntSetting("WEBSERVERPORT", 8080);
    m_webserver_port = new wxSpinCtrl(this, wxID_ANY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535, webserverPort);
    m_webserver_port->SetValue(webserverPort);
    m_webserver_port->SetToolTip(_("Specify web server port number"));

    wxFlexGridSizer* flex_sizer4 = new wxFlexGridSizer(0, 4, 0, 0);
    flex_sizer4->Add(m_webserver_checkbox, g_flagsH);
    flex_sizer4->Add(new wxStaticText(this, wxID_STATIC, _("Port")), g_flagsH);
    flex_sizer4->Add(m_webserver_port, g_flagsH);

    webserverStaticBoxSizer->Add(flex_sizer4, g_flagsV);
#endif

    //Usage data send
    wxStaticBox* usageStaticBox = new wxStaticBox(this, wxID_STATIC, _("Usage statistics"));
    SetBoldFont(usageStaticBox);
    wxStaticBoxSizer* usageStaticBoxSizer = new wxStaticBoxSizer(usageStaticBox, wxVERTICAL);
    networkPanelSizer->Add(usageStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_send_data = new wxCheckBox(this, wxID_ANY
        , _("Send anonymous statistics usage data"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_send_data->SetValue(Option::instance().SendUsageStatistics());
    m_send_data->SetToolTip(_("Enable to help us sending anonymous data about MMEX usage."));

    usageStaticBoxSizer->Add(m_send_data, g_flagsV);

    // Communication timeout
    wxStaticBox* timeoutStaticBox = new wxStaticBox(this, wxID_STATIC, _("Timeout"));
    SetBoldFont(timeoutStaticBox);
    wxStaticBoxSizer* timeoutStaticBoxSizer = new wxStaticBoxSizer(timeoutStaticBox, wxVERTICAL);
    networkPanelSizer->Add(timeoutStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    int nTimeout = Model_Setting::instance().GetIntSetting("NETWORKTIMEOUT", 10);
    m_network_timeout = new wxSpinCtrl(this, wxID_ANY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 150, nTimeout);
    m_network_timeout->SetValue(nTimeout);
    m_network_timeout->SetToolTip(_("Specify a network communication timeout value to use."));

    wxFlexGridSizer* flex_sizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    flex_sizer5->Add(new wxStaticText(this, wxID_STATIC, _("Seconds")), g_flagsH);
    flex_sizer5->Add(m_network_timeout, g_flagsH);

    timeoutStaticBoxSizer->Add(flex_sizer5, g_flagsV);

    //Updates check
    wxStaticBox* updateStaticBox = new wxStaticBox(this, wxID_STATIC, _("Check for Updates"));
    SetBoldFont(updateStaticBox);
    wxStaticBoxSizer* updateStaticBoxSizer = new wxStaticBoxSizer(updateStaticBox, wxVERTICAL);
    networkPanelSizer->Add(updateStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_check_update = new wxCheckBox(this, ID_DIALOG_OPTIONS_UPDATES_CHECK
        , _("Check at StartUp"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_check_update->SetValue(GetIniDatabaseCheckboxValue("UPDATECHECK", true));
    m_check_update->SetToolTip(_("Enable to automatically check if new MMEX version is available at StartUp"));

    wxArrayString UpdatesType_;
    UpdatesType_.Add(_("Stable"));
    UpdatesType_.Add(_("Unstable"));
    m_update_source = new wxChoice(this, wxID_ANY
        , wxDefaultPosition, wxSize(150, -1), UpdatesType_);
    m_update_source->SetSelection(Model_Setting::instance().GetIntSetting("UPDATESOURCE", 0));
    m_update_source->SetToolTip(_("Select updates source channel"));

    wxFlexGridSizer* UpdateSourceStaticBoxSizerGrid = new wxFlexGridSizer(0, 3, 0, 0);
    UpdateSourceStaticBoxSizerGrid->Add(new wxStaticText(this, wxID_STATIC, _("Select preferred version:")), g_flagsH);
    UpdateSourceStaticBoxSizerGrid->Add(m_update_source, g_flagsH);
    UpdateSourceStaticBoxSizerGrid->Add(m_check_update, g_flagsH);
    updateStaticBoxSizer->Add(UpdateSourceStaticBoxSizerGrid, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxCommandEvent evt;
    OptionSettingsNet::OnProxyChanged(evt);
#ifdef MMEX_WEBSERVER
    OptionSettingsNet::OnEnableWebserverChanged(evt);
#endif
    SetSizer(networkPanelSizer);
}

void OptionSettingsNet::OnProxyChanged(wxCommandEvent& WXUNUSED(event))
{
    m_proxy_port->Enable(m_proxy_address->GetValue() != "");
}

#ifdef MMEX_WEBSERVER
void OptionSettingsNet::OnEnableWebserverChanged(wxCommandEvent& WXUNUSED(event))
{
    m_webserver_port->Enable(m_webserver_checkbox->GetValue());
}
#endif

void OptionSettingsNet::OnWebAppTest(wxCommandEvent& WXUNUSED(event))
{
    OptionSettingsNet::SaveSettings();
    if (mmWebApp::WebApp_CheckEnabled())
    {
        if (mmWebApp::WebApp_CheckGuid() && mmWebApp::WebApp_CheckApiVersion())
        {
            wxMessageBox(_("WebApp connection tested successfully!"), _("WebApp connection test"));
        }
    }
    else
    {
        wxMessageBox(_("Some parameters are missing."), _("WebApp connection test"));
    }
}

void OptionSettingsNet::SaveSettings()
{
    Model_Setting::instance().Set("PROXYIP", m_proxy_address->GetValue());
    Model_Setting::instance().Set("PROXYPORT", m_proxy_port->GetValue());

    wxTextCtrl* WebAppURL = (wxTextCtrl*) FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPURL);
    Model_Infotable::instance().Set("WEBAPPURL", WebAppURL->GetValue());

    wxTextCtrl* WebAppGUID = (wxTextCtrl*) FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPGUID);
    Model_Infotable::instance().Set("WEBAPPGUID", WebAppGUID->GetValue());

#ifdef MMEX_WEBSERVER
    Model_Setting::instance().Set("ENABLEWEBSERVER", m_webserver_checkbox->GetValue());
    Model_Setting::instance().Set("WEBSERVERPORT", m_webserver_port->GetValue());
#endif

    Option::instance().SendUsageStatistics(m_send_data->GetValue());

    Model_Setting::instance().Set("NETWORKTIMEOUT", m_network_timeout->GetValue());

    Model_Setting::instance().Set("UPDATECHECK", m_check_update->GetValue());
    Model_Setting::instance().Set("UPDATESOURCE", m_update_source->GetSelection());
}
