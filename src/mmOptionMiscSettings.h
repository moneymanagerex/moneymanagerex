#pragma once

#include "mmOptionBaseSettings.h"

class mmOptionMiscSettings : public mmOptionSettingsBase
{
    wxDECLARE_EVENT_TABLE();

public:
    mmOptionMiscSettings();

    mmOptionMiscSettings(wxWindow *parent
        , wxWindowID id = wxID_ANY
        , const wxPoint &pos = wxDefaultPosition
        , const wxSize &size = wxDefaultSize
        , long style = wxTAB_TRAVERSAL
        , const wxString &name = wxPanelNameStr);

    ~mmOptionMiscSettings();

public:
    virtual void SaveSettings();

private:
    void Create();

    void SaveStocksUrl();

private:
    wxSpinCtrl* m_max_files;

    enum
    {
        ID_DIALOG_OPTIONS_TEXTCTRL_DELIMITER4 = wxID_HIGHEST + 10,
        ID_DIALOG_OPTIONS_CHK_BACKUP,
        ID_DIALOG_OPTIONS_CHK_BACKUP_UPDATE,
        ID_DIALOG_OPTIONS_TEXTCTRL_STOCKURL,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_PAYEE,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_STATUS,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_DATE,
    };
};
