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
        , const wxString &name = wxPanelNameStr);

    ~OptionSettingsGeneral();

public:
    virtual bool SaveSettings();

private:
    void Create();

    void OnDateFormatChanged(wxCommandEvent& event);
    void OnLocaleChanged(wxCommandEvent& event);
    void OnChangeGUILanguage(wxCommandEvent &event);
    void OnMouseLeftDown(wxCommandEvent& event);

    bool SaveFinancialYearStart();
    bool doFormatDoubleValue(const wxString& locale, wxString& result);

private:
    mmGUIApp* m_app = nullptr;
    wxStaticBoxSizer* m_currencyStaticBoxSizer = nullptr;
    mmComboBoxCurrency* baseCurrencyComboBox_ = nullptr;

    wxChoice* m_date_format_choice = nullptr;
    wxChoice* m_month_selection = nullptr;
    wxComboBox* m_itemListOfLocales = nullptr;
    wxStaticText* m_sample_date_text = nullptr;
    wxStaticText* m_sample_value_text = nullptr;

    int m_currency_id = -1;
    wxCheckBox* m_currency_history = nullptr;
    wxString m_date_format;

    wxCheckBox* m_use_org_date_copy_paste = nullptr;
    wxCheckBox* m_use_org_date_duplicate = nullptr;
    wxChoice* m_use_sound = nullptr;

    enum
    {
        ID_DIALOG_OPTIONS_BUTTON_CURRENCY = wxID_HIGHEST + 10,
        ID_DIALOG_OPTIONS_WXCHOICE_DATE,
        ID_DIALOG_OPTIONS_BUTTON_LANG,
        ID_DIALOG_OPTIONS_TEXTCTRL_USERNAME,
        ID_DIALOG_OPTIONS_LOCALE,
        ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_DAY,
        ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_MONTH
    };
};
