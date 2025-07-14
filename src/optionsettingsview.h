/*******************************************************
Copyright (C) 2014 Stefano Giorgio
Copyright (C) 2021-2022 Mark Whalley (mark@ipx.co.uk)


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
#include <wx/spinctrl.h>
#include <wx/slider.h>

class OptionSettingsView : public OptionSettingsBase
{
    wxDECLARE_EVENT_TABLE();

public:
    OptionSettingsView();

    OptionSettingsView(wxWindow *parent
        , wxWindowID id = wxID_ANY
        , const wxPoint &pos = wxDefaultPosition
        , const wxSize &size = wxDefaultSize
        , long style = wxTAB_TRAVERSAL
        , const wxString &name = wxPanelNameStr);

    ~OptionSettingsView();

public:
    virtual bool SaveSettings();

private:
    void Create();
    void OnHTMLScaleSpin(wxSpinEvent& event);
    void OnThemeManagerSelected(wxCommandEvent&);
    void OnNavTreeColorChanged(wxCommandEvent& event);

private:
    /// Colour Buttons.
    wxButton* m_UDFCB1 = nullptr;
    wxButton* m_UDFCB2 = nullptr;
    wxButton* m_UDFCB3 = nullptr;
    wxButton* m_UDFCB4 = nullptr;
    wxButton* m_UDFCB5 = nullptr;
    wxButton* m_UDFCB6 = nullptr;
    wxButton* m_UDFCB7 = nullptr;


    wxButton* m_theme_manager = nullptr;
    wxChoice* m_theme_mode = nullptr;
    wxChoice* m_font_size_chooser = nullptr;
    wxChoice* m_choice_visible = nullptr;
    wxSpinCtrl* m_scale_factor = nullptr;
    int htmlScaleMin, htmlScaleMax;
    wxChoice* m_toolbar_icon_size = nullptr;
    wxChoice* m_navigation_icon_size = nullptr;
    wxChoice* m_others_icon_size = nullptr;
    wxComboBox* m_categ_delimiter_list = nullptr;

    wxCheckBox* m_budget_financial_years = nullptr;
    wxCheckBox* m_budget_include_transfers = nullptr;
    wxCheckBox* m_budget_summary_without_category = nullptr;
    wxCheckBox* m_budget_override = nullptr;
    wxCheckBox* m_budget_deduct_monthly = nullptr;
    wxCheckBox* m_ignore_future_transactions = nullptr;
    wxCheckBox* m_use_trans_date_time = nullptr;
    wxSpinCtrl* m_budget_days_offset = nullptr;
    wxSpinCtrl* m_reporting_firstday = nullptr;
    wxChoice* m_reporting_first_weekday = nullptr;

    wxCheckBox* m_doNotColorFuture = nullptr;
    wxCheckBox* m_doSpecialColorReconciled = nullptr;

    wxCheckBox* m_showToolTips = nullptr;
    wxCheckBox* m_showMoneyTips = nullptr;

    enum
    {
        ID_DIALOG_THEMEMANAGER = wxID_HIGHEST + 100,
        ID_DIALOG_HTML_SCALE
    };
};
