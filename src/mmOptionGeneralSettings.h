#pragma once

#include "mmOptionBaseSettings.h"

class mmGUIApp;

class mmOptionGeneralSettings : public mmOptionSettingsBase
{
    wxDECLARE_EVENT_TABLE();

public:
    mmOptionGeneralSettings();

    mmOptionGeneralSettings(wxWindow *parent, mmGUIApp* app
        , wxWindowID id = wxID_ANY
        , const wxPoint &pos = wxDefaultPosition
        , const wxSize &size = wxDefaultSize
        , long style = wxTAB_TRAVERSAL
        , const wxString &name = wxPanelNameStr);

    ~mmOptionGeneralSettings();

public:
    virtual void SaveSettings();

private:
    void Create();

    void OnCurrency(wxCommandEvent& event);
    void OnDateFormatChanged(wxCommandEvent& event);
    void OnLanguageChanged(wxCommandEvent& event);

    void SaveFinancialYearStart();

private:
    mmGUIApp* m_app;
    wxChoice* m_date_format_choice;
    wxChoice* m_month_selection;
    wxStaticText* m_sample_date_text;

    int m_currency_id;
    wxString m_date_format;
    wxString m_current_language;

    wxCheckBox* m_use_org_date_copy_paste;
    wxCheckBox* m_use_sound;

    enum
    {
        ID_DIALOG_OPTIONS_BUTTON_CURRENCY = wxID_HIGHEST + 10,
        ID_DIALOG_OPTIONS_WXCHOICE_DATE,
        ID_DIALOG_OPTIONS_BUTTON_LANGUAGE,
        ID_DIALOG_OPTIONS_TEXTCTRL_USERNAME,
        ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_DAY,
        ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_MONTH
    };
};
