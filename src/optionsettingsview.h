/*******************************************************
Copyright (C) 2014 Stefano Giorgio
Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)


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
    void OnThemeManagerSelected(wxCommandEvent& event);
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

    
    wxButton* m_theme_manager;
    wxChoice* m_theme_mode;
    wxChoice* m_font_size;
    wxChoice* m_choice_visible;
    wxSpinCtrl* m_scale_factor;
    int htmlScaleMin, htmlScaleMax;
    wxChoice* m_toolbar_icon_size;
    wxChoice* m_navigation_icon_size;
    wxChoice* m_others_icon_size;
    wxComboBox* m_categ_delimiter_list;

    wxCheckBox* m_budget_financial_years;
    wxCheckBox* m_budget_include_transfers;
    wxCheckBox* m_budget_summary_without_category;
    wxCheckBox* m_ignore_future_transactions;
    wxSpinCtrl* m_budget_days_offset;
    wxSpinCtrl* m_reporting_firstday;

    wxCheckBox* m_showToolTips;
    wxCheckBox* m_showMoneyTips;

    enum
    {
        ID_DIALOG_THEMEMANAGER = wxID_HIGHEST + 100,
        ID_DIALOG_HTML_SCALE
    };
};
