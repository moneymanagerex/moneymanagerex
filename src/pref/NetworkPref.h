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

#pragma once

#include "_PrefBase.h"
#include <vector>

class wxSpinCtrl;
class PocketBaseSyncService;

class NetworkPref : public PrefBase
{
    friend class PocketBaseSyncService;
    wxDECLARE_EVENT_TABLE();

public:
    NetworkPref();

    NetworkPref(wxWindow *parent
        , wxWindowID id = wxID_ANY
        , const wxPoint &pos = wxDefaultPosition
        , const wxSize &size = wxDefaultSize
        , long style = wxTAB_TRAVERSAL
        , const wxString &name = wxPanelNameStr);

    ~NetworkPref();

public:
    virtual bool SaveSettings();

private:
    void Create();

    void OnProxyChanged(wxCommandEvent& event);
    void OnUpdateCheckChanged(wxCommandEvent& event);
    void OnEnableWebserverChanged(wxCommandEvent& event);

private:
    wxSpinCtrl* m_network_timeout = nullptr;
    wxCheckBox* m_send_data = nullptr;
    wxCheckBox* m_webserver_checkbox = nullptr;
    wxSpinCtrl* m_webserver_port = nullptr;
    wxTextCtrl* m_proxy_address = nullptr;
    wxSpinCtrl* m_proxy_port = nullptr;
    wxCheckBox* m_check_update = nullptr;
    wxCheckBox* m_check_news = nullptr;
    wxChoice* m_update_source = nullptr;
    wxTextCtrl* m_pocketbase_url = nullptr;
    wxTextCtrl* m_pocketbase_email = nullptr;
    wxTextCtrl* m_pocketbase_password = nullptr;
    wxStaticText* m_pocketbase_status = nullptr;
    wxButton* m_pocketbase_login = nullptr;
    wxButton* m_pocketbase_logout = nullptr;
    wxButton* m_pocketbase_sync_currencies = nullptr;
    wxButton* m_pocketbase_sync_accounts = nullptr;
    wxButton* m_pocketbase_sync_categories = nullptr;
    wxButton* m_pocketbase_sync_payees = nullptr;
    wxButton* m_pocketbase_sync_transactions = nullptr;
    bool m_pocketbase_auth_check = false;

    void OnPocketBaseLogin(wxCommandEvent& event);
    void OnPocketBaseLogout(wxCommandEvent& event);
    void OnPocketBaseSyncCurrencies(wxCommandEvent& event);
    void OnPocketBaseSyncAccounts(wxCommandEvent& event);
    void OnPocketBaseSyncCategories(wxCommandEvent& event);
    void OnPocketBaseSyncPayees(wxCommandEvent& event);
    void OnPocketBaseSyncTransactions(wxCommandEvent& event);

    enum
    {
        ID_DIALOG_OPTIONS_TEXTCTRL_PROXY = wxID_HIGHEST + 10,
        ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPURL,
        ID_DIALOG_OPTIONS_TEXTCTRL_WEBAPPGUID,
        ID_DIALOG_OPTIONS_ENABLE_WEBSERVER,
        ID_DIALOG_OPTIONS_UPDATES_CHECK
    };
};
