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

#include "optionsettingsbase.h"

class wxSpinCtrl;
class OptionSettingsMisc : public OptionSettingsBase
{
    wxDECLARE_EVENT_TABLE();

public:
    OptionSettingsMisc();

    OptionSettingsMisc(wxWindow *parent
        , wxWindowID id = wxID_ANY
        , const wxPoint &pos = wxDefaultPosition
        , const wxSize &size = wxDefaultSize
        , long style = wxTAB_TRAVERSAL
        , const wxString &name = wxPanelNameStr);

    ~OptionSettingsMisc();

public:
    virtual bool SaveSettings();

private:
    void Create();

    void OnBackupChanged(wxCommandEvent& event);
    void SaveStocksUrl();

private:
    wxSpinCtrl* m_max_files;
    wxSpinCtrl* m_share_precision;

    enum
    {
        ID_DIALOG_OPTIONS_TEXTCTRL_DELIMITER4 = wxID_HIGHEST + 10,
        ID_DIALOG_OPTIONS_CHK_BACKUP,
        ID_DIALOG_OPTIONS_CHK_BACKUP_UPDATE,
        ID_DIALOG_OPTIONS_TEXTCTRL_STOCKURL,
        ID_DIALOG_OPTIONS_BULK_ENTER,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_PAYEE,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY_NONTRANSFER,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY_TRANSFER,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_STATUS,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_DATE,
    };
};
