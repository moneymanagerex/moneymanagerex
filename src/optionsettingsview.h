/*******************************************************
Copyright (C) 2014 Stefano Giorgio
Copyright (C) 2021-2022 Mark Whalley (mark@ipx.co.uk)
Copyright (C) 2025 Klaus Wich

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

private:
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
