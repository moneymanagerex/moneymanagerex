#include "mmOptionNetSettings.h"
#include <wx/spinctrl.h>
#include "constants.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"

/*******************************************************/
wxBEGIN_EVENT_TABLE(mmOptionNetSettings, wxPanel)
wxEND_EVENT_TABLE()
/*******************************************************/

mmOptionNetSettings::mmOptionNetSettings()
{

}

mmOptionNetSettings::mmOptionNetSettings(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
{
    wxPanel::Create(parent, id, pos, size, style, name);
    Create(parent, id, pos, size, style, name);
}

mmOptionNetSettings::~mmOptionNetSettings()
{

}

void mmOptionNetSettings::Create(wxWindow *parent
    , wxWindowID id
    , const wxPoint& pos
    , const wxSize& size
    , long style
    , const wxString &name)
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

    WebAppStaticBoxSizerGrid->Add(new wxStaticText(this, wxID_STATIC, _("Url")), g_flags);
    wxString WebAppURL = Model_Infotable::instance().GetStringInfo("WEBAPPURL", "");
    wxTextCtrl* WebAppURLTextCtr = new wxTextCtrl(this, ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPURL,
        WebAppURL, wxDefaultPosition, wxSize(300, -1));
    WebAppURLTextCtr->SetToolTip(_("Specify the Web App URL without final slash"));
    WebAppStaticBoxSizerGrid->Add(WebAppURLTextCtr, 1, wxEXPAND | wxALL, 5);

    WebAppStaticBoxSizerGrid->Add(new wxStaticText(this, wxID_STATIC, _("GUID")), g_flags);
    wxString WebAppGUID = Model_Infotable::instance().GetStringInfo("WEBAPPGUID", "");
    wxTextCtrl* WebAppGUIDTextCtr = new wxTextCtrl(this, ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPGUID,
        WebAppGUID, wxDefaultPosition, wxSize(300, -1));
    WebAppGUIDTextCtr->SetToolTip(_("Specify the Web App GUID"));
    WebAppStaticBoxSizerGrid->Add(WebAppGUIDTextCtr, 1, wxEXPAND | wxALL, 5);

    // Proxy Settings
    wxStaticBox* proxyStaticBox = new wxStaticBox(this, wxID_STATIC, _("Proxy Settings"));
    SetBoldFont(proxyStaticBox);
    wxStaticBoxSizer* proxyStaticBoxSizer = new wxStaticBoxSizer(proxyStaticBox, wxVERTICAL);
    networkPanelSizer->Add(proxyStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxString proxyName = Model_Setting::instance().GetStringSetting("PROXYIP", "");
    wxTextCtrl* proxyNameTextCtr = new wxTextCtrl(this, ID_DIALOG_OPTIONS_TEXTCTRL_PROXY
        , proxyName, wxDefaultPosition, wxSize(150, -1));
    proxyNameTextCtr->SetToolTip(_("Specify the proxy IP address"));

    int proxyPort = Model_Setting::instance().GetIntSetting("PROXYPORT", 0);
    m_proxy_port = new wxSpinCtrl(this, wxID_ANY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535, proxyPort);
    m_proxy_port->SetValue(proxyPort);
    m_proxy_port->SetToolTip(_("Specify proxy port number"));

    wxFlexGridSizer* flex_sizer3 = new wxFlexGridSizer(0, 4, 0, 0);
    flex_sizer3->Add(new wxStaticText(this, wxID_STATIC, _("Proxy")), g_flags);
    flex_sizer3->Add(proxyNameTextCtr, g_flags);
    flex_sizer3->Add(new wxStaticText(this, wxID_STATIC, _("Port")), g_flags);
    flex_sizer3->Add(m_proxy_port, g_flags);

    proxyStaticBoxSizer->Add(flex_sizer3, g_flags);

    // Web Server Settings
    wxStaticBox* webserverStaticBox = new wxStaticBox(this, wxID_STATIC, _("Web Server"));
    SetBoldFont(webserverStaticBox);
    wxStaticBoxSizer* webserverStaticBoxSizer = new wxStaticBoxSizer(webserverStaticBox, wxVERTICAL);
    networkPanelSizer->Add(webserverStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_webserver_checkbox = new wxCheckBox(this, wxID_ANY
        , _("Enable"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_webserver_checkbox->SetValue(GetIniDatabaseCheckboxValue("ENABLEWEBSERVER", true));
    m_webserver_checkbox->SetToolTip(_("Enable internal web server when MMEX Starts."));

    int webserverPort = Model_Setting::instance().GetIntSetting("WEBSERVERPORT", 8080);
    m_webserver_port = new wxSpinCtrl(this, wxID_ANY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535, webserverPort);
    m_webserver_port->SetValue(webserverPort);
    m_webserver_port->SetToolTip(_("Specify web server port number"));

    wxFlexGridSizer* flex_sizer4 = new wxFlexGridSizer(0, 4, 0, 0);
    flex_sizer4->Add(m_webserver_checkbox, g_flags);
    flex_sizer4->Add(new wxStaticText(this, wxID_STATIC, _("Port")), g_flags);
    flex_sizer4->Add(m_webserver_port, g_flags);

    webserverStaticBoxSizer->Add(flex_sizer4, g_flags);

    //Usage data send
    wxStaticBox* usageStaticBox = new wxStaticBox(this, wxID_STATIC, _("Usage statistics"));
    SetBoldFont(usageStaticBox);
    wxStaticBoxSizer* usageStaticBoxSizer = new wxStaticBoxSizer(usageStaticBox, wxVERTICAL);
    networkPanelSizer->Add(usageStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_send_data = new wxCheckBox(this, wxID_ANY
        , _("Send anonymous statistics usage data"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_send_data->SetValue(GetIniDatabaseCheckboxValue(INIDB_SEND_USAGE_STATS, true));
    m_send_data->SetToolTip(_("Enable to help us sending anonymous data about MMEX usage."));

    usageStaticBoxSizer->Add(m_send_data, g_flags);

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
    flex_sizer5->Add(new wxStaticText(this, wxID_STATIC, _("Seconds")), g_flags);
    flex_sizer5->Add(m_network_timeout, g_flags);

    timeoutStaticBoxSizer->Add(flex_sizer5, g_flags);

    //Updates check
    wxStaticBox* updateStaticBox = new wxStaticBox(this, wxID_STATIC, _("Updates"));
    SetBoldFont(updateStaticBox);
    wxStaticBoxSizer* updateStaticBoxSizer = new wxStaticBoxSizer(updateStaticBox, wxVERTICAL);
    networkPanelSizer->Add(updateStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_check_update = new wxCheckBox(this, wxID_ANY
        , _("Check for updates at StartUp"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_check_update->SetValue(GetIniDatabaseCheckboxValue("UPDATECHECK", true));
    m_check_update->SetToolTip(_("Enable to automatically check if new MMEX version is available at StartUp"));
    updateStaticBoxSizer->Add(m_check_update, g_flags);

    wxFlexGridSizer* UpdateSourceStaticBoxSizerGrid = new wxFlexGridSizer(0, 2, 0, 0);
    updateStaticBoxSizer->Add(UpdateSourceStaticBoxSizerGrid, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxArrayString UpdatesType_;
    UpdatesType_.Add(_("Stable"));
    UpdatesType_.Add(_("Unstable"));
    wxChoice* updatesTypeChoice = new wxChoice(this, ID_DIALOG_OPTIONS_UPDATES_SOURCE_TYPE
        , wxDefaultPosition, wxSize(150, -1), UpdatesType_);
    updatesTypeChoice->SetSelection(Model_Setting::instance().GetIntSetting("UPDATESOURCE", 0));
    UpdateSourceStaticBoxSizerGrid->Add(new wxStaticText(this, wxID_STATIC, _("Updates source:")), g_flags);
    UpdateSourceStaticBoxSizerGrid->Add(updatesTypeChoice, g_flags);

    SetSizer(networkPanelSizer);
}

void mmOptionNetSettings::SaveSettings()
{
    wxTextCtrl* proxy = (wxTextCtrl*) FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_PROXY);
    wxString proxyName = proxy->GetValue();
    Model_Setting::instance().Set("PROXYIP", proxyName);
    Model_Setting::instance().Set("PROXYPORT", m_proxy_port->GetValue());

    wxTextCtrl* WebAppURL = (wxTextCtrl*) FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPURL);
    Model_Infotable::instance().Set("WEBAPPURL", WebAppURL->GetValue());

    wxTextCtrl* WebAppGUID = (wxTextCtrl*) FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPGUID);
    Model_Infotable::instance().Set("WEBAPPGUID", WebAppGUID->GetValue());

    Model_Setting::instance().Set("ENABLEWEBSERVER", m_webserver_checkbox->GetValue());
    Model_Setting::instance().Set("WEBSERVERPORT", m_webserver_port->GetValue());

    Model_Setting::instance().Set(INIDB_SEND_USAGE_STATS, m_send_data->GetValue());

    Model_Setting::instance().Set("NETWORKTIMEOUT", m_network_timeout->GetValue());

    Model_Setting::instance().Set("UPDATECHECK", m_check_update->GetValue());
    wxChoice* itemChoice = (wxChoice*) FindWindow(ID_DIALOG_OPTIONS_UPDATES_SOURCE_TYPE);
    Model_Setting::instance().Set("UPDATESOURCE", itemChoice->GetSelection());
}
