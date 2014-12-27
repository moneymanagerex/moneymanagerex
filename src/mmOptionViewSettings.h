#pragma once

#include "mmOptionBaseSettings.h"

class mmOptionViewSettings : public mmOptionSettingsBase
{
    wxDECLARE_EVENT_TABLE();

public:
    mmOptionViewSettings();

    mmOptionViewSettings(wxWindow *parent
        , wxWindowID id = wxID_ANY
        , const wxPoint &pos = wxDefaultPosition
        , const wxSize &size = wxDefaultSize
        , long style = wxTAB_TRAVERSAL
        , const wxString &name = wxPanelNameStr);

    ~mmOptionViewSettings();

public:
    virtual void SaveSettings();

private:
    void Create(wxWindow *parent
        , wxWindowID id
        , const wxPoint& pos
        , const wxSize& size
        , long style
        , const wxString &name);

    wxArrayString viewAccountStrings(bool translated, const wxString& input_string, int& row_id);
    bool GetIniDatabaseCheckboxValue(wxString dbField, bool defaultState);
    void OnNavTreeColorChanged(wxCommandEvent& event);

private:
    /// Colour Buttons.
    wxButton* m_UDFCB1;
    wxButton* m_UDFCB2;
    wxButton* m_UDFCB3;
    wxButton* m_UDFCB4;
    wxButton* m_UDFCB5;
    wxButton* m_UDFCB6;
    wxButton* m_UDFCB7;

    wxChoice* m_choice_visible;
    wxChoice* m_choice_trans_visible;
    wxChoice* m_choice_font_size;


    wxCheckBox* m_budget_financial_years;
    wxCheckBox* m_budget_include_transfers;
    wxCheckBox* m_budget_setup_without_summary;
    wxCheckBox* m_budget_summary_without_category;
    wxCheckBox* m_ignore_future_transactions;

    enum
    {
        ID_DIALOG_OPTIONS_VIEW_ACCOUNTS = wxID_HIGHEST + 10
    };
};
