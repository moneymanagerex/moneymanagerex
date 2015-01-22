/*******************************************************
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

#pragma once

#include "mmOptionBaseSettings.h"

class wxSpinCtrl;

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
    void Create();

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
