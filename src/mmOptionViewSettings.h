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
    void Create();

    wxArrayString viewAccountStrings(bool translated, const wxString& input_string, int& row_id);
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

    wxCheckBox* m_display_internet_news;

    enum
    {
        ID_DIALOG_OPTIONS_VIEW_ACCOUNTS = wxID_HIGHEST + 10
    };
};
