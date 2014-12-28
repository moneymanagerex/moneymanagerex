#pragma once

#include "mmOptionBaseSettings.h"

class mmOptionNetSettings : public mmOptionSettingsBase
{
    wxDECLARE_EVENT_TABLE();

public:
    mmOptionNetSettings();

    mmOptionNetSettings(wxWindow *parent
        , wxWindowID id = wxID_ANY
        , const wxPoint &pos = wxDefaultPosition
        , const wxSize &size = wxDefaultSize
        , long style = wxTAB_TRAVERSAL
        , const wxString &name = wxPanelNameStr);

    ~mmOptionNetSettings();

public:
    virtual void SaveSettings();

private:
    void Create(wxWindow *parent
        , wxWindowID id
        , const wxPoint& pos
        , const wxSize& size
        , long style
        , const wxString &name);

private:
    wxCheckBox* m_check_update;
    wxSpinCtrl* m_network_timeout;
    wxCheckBox* m_send_data;
    wxCheckBox* m_webserver_checkbox;
    wxSpinCtrl* m_webserver_port;
    wxSpinCtrl* m_proxy_port;

    enum
    {
        ID_DIALOG_OPTIONS_TEXTCTRL_PROXY = wxID_HIGHEST + 10,
        ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPURL,
        ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPGUID,
        ID_DIALOG_OPTIONS_UPDATES_SOURCE_TYPE
    };
};
