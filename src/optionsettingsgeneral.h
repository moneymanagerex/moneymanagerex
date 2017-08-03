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

class mmGUIApp;

class OptionSettingsGeneral : public OptionSettingsBase
{
    wxDECLARE_EVENT_TABLE();

public:
    OptionSettingsGeneral();

    OptionSettingsGeneral(wxWindow *parent, mmGUIApp* app
        , wxWindowID id = wxID_ANY
        , const wxPoint &pos = wxDefaultPosition
        , const wxSize &size = wxDefaultSize
        , long style = wxTAB_TRAVERSAL
        , const wxString &name = "OptionSettingsGeneral");

    ~OptionSettingsGeneral();

public:
    virtual void SaveSettings();

private:
    void Create();

    void OnCurrency(wxCommandEvent& event);
    void OnDateFormatChanged(wxCommandEvent& event);

    bool SaveFinancialYearStart();

private:
    mmGUIApp* m_app;
    wxChoice* m_date_format_choice;
    wxChoice* m_month_selection;
    wxStaticText* m_sample_date_text;

    int m_currency_id;
    wxString m_date_format;

    wxCheckBox* m_use_org_date_copy_paste;
    wxCheckBox* m_use_sound;

    enum
    {
        ID_DIALOG_OPTIONS_BUTTON_CURRENCY = wxID_HIGHEST + 10,
        ID_DIALOG_OPTIONS_WXCHOICE_DATE,
        ID_DIALOG_OPTIONS_TEXTCTRL_USERNAME,
        ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_DAY,
        ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_MONTH
    };
};
