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

#include "optionsettingsgeneral.h"
#include "maincurrencydialog.h"
#include "mmSimpleDialogs.h"
#include "util.h"
#include "option.h"

#include "Model_Currency.h"

#include <wx/spinctrl.h>

/*******************************************************/
wxBEGIN_EVENT_TABLE(OptionSettingsGeneral, wxPanel)
    EVT_BUTTON(ID_DIALOG_OPTIONS_BUTTON_CURRENCY, OptionSettingsGeneral::OnCurrency)
    EVT_CHOICE(ID_DIALOG_OPTIONS_WXCHOICE_DATE, OptionSettingsGeneral::OnDateFormatChanged)
wxEND_EVENT_TABLE()
/*******************************************************/

OptionSettingsGeneral::OptionSettingsGeneral()
{
}

OptionSettingsGeneral::OptionSettingsGeneral(wxWindow *parent, mmGUIApp* app
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
{
    wxPanel::Create(parent, id, pos, size, style, name);
    m_app = app;
    m_currency_id = Option::instance().BaseCurrency();
    m_date_format = Option::instance().DateFormat();

    Create();
}

OptionSettingsGeneral::~OptionSettingsGeneral()
{
}

void OptionSettingsGeneral::Create()
{
    wxBoxSizer* generalPanelSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(generalPanelSizer);

    // Display Header Settings
    wxStaticBox* headerStaticBox = new wxStaticBox(this, wxID_STATIC, _("Display Heading"));
    SetBoldFont(headerStaticBox);

    wxStaticBoxSizer* headerStaticBoxSizer = new wxStaticBoxSizer(headerStaticBox, wxHORIZONTAL);

    headerStaticBoxSizer->Add(new wxStaticText(this, wxID_STATIC, _("User Name")), g_flagsH);

    wxString userName = Model_Infotable::instance().GetStringInfo("USERNAME", "");
    wxTextCtrl* userNameTextCtr = new wxTextCtrl(this, ID_DIALOG_OPTIONS_TEXTCTRL_USERNAME
        , userName, wxDefaultPosition, wxSize(200, -1));
    userNameTextCtr->SetToolTip(_("The User Name is used as a title for the database."));
    headerStaticBoxSizer->Add(userNameTextCtr, g_flagsExpand);
    generalPanelSizer->Add(headerStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    // Currency Settings
    wxStaticBox* currencyStaticBox = new wxStaticBox(this, wxID_STATIC, _("Currency"));
    SetBoldFont(currencyStaticBox);
    wxStaticBoxSizer* currencyStaticBoxSizer = new wxStaticBoxSizer(currencyStaticBox, wxHORIZONTAL);
    SetBoldFont(currencyStaticBox);
    generalPanelSizer->Add(currencyStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    currencyStaticBoxSizer->Add(new wxStaticText(this, wxID_STATIC, _("Base Currency")), g_flagsH);

    wxString currName = _("Set Currency");
    Model_Currency::Data* currency = Model_Currency::instance().get(m_currency_id);
    if (currency)
        currName = wxGetTranslation(currency->CURRENCYNAME);
    wxButton* baseCurrencyButton = new wxButton(this, ID_DIALOG_OPTIONS_BUTTON_CURRENCY
        , currName, wxDefaultPosition, wxDefaultSize);
    baseCurrencyButton->SetToolTip(_("Sets the database default Currency using the 'Currency Dialog'")
        + "\n" + _("Right click and select 'Set as Base Currency' in 'Currency Dialog'"));
    currencyStaticBoxSizer->Add(baseCurrencyButton, g_flagsH);

    // Date Format Settings
    wxStaticBox* dateFormatStaticBox = new wxStaticBox(this, wxID_STATIC, _("Date Format"));
    SetBoldFont(dateFormatStaticBox);
    wxStaticBoxSizer* dateFormatStaticBoxSizer = new wxStaticBoxSizer(dateFormatStaticBox, wxHORIZONTAL);
    generalPanelSizer->Add(dateFormatStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_date_format_choice = new wxChoice(this, ID_DIALOG_OPTIONS_WXCHOICE_DATE);
    for (const auto& i : g_date_formats_map)
    {
        m_date_format_choice->Append(i.second, new wxStringClientData(i.first));
        if (m_date_format == i.first) m_date_format_choice->SetStringSelection(i.second);
    }
    dateFormatStaticBoxSizer->Add(m_date_format_choice, g_flagsH);
    m_date_format_choice->SetToolTip(_("Specify the date format for display"));

    m_sample_date_text = new wxStaticText(this, wxID_STATIC,
        "redefined elsewhere");
    dateFormatStaticBoxSizer->Add(new wxStaticText(this, wxID_STATIC,
        _("New date format sample:")), wxSizerFlags(g_flagsH).Border(wxLEFT, 15));
    dateFormatStaticBoxSizer->Add(m_sample_date_text, wxSizerFlags(g_flagsH).Border(wxLEFT, 5));
    m_sample_date_text->SetLabelText(wxDateTime::Now().Format(m_date_format));

    // Financial Year Settings
    wxStaticBox* financialYearStaticBox = new wxStaticBox(this, wxID_ANY, _("Financial Year"));
    SetBoldFont(financialYearStaticBox);
    wxStaticBoxSizer* financialYearStaticBoxSizer = new wxStaticBoxSizer(financialYearStaticBox, wxVERTICAL);
    wxFlexGridSizer* financialYearStaticBoxSizerGrid = new wxFlexGridSizer(0, 2, 0, 0);
    generalPanelSizer->Add(financialYearStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));
    financialYearStaticBoxSizer->Add(financialYearStaticBoxSizerGrid);

    financialYearStaticBoxSizerGrid->Add(new wxStaticText(this, wxID_STATIC, _("Start Day")), g_flagsH);
    int day = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_DAY", 1);

    wxSpinCtrl *textFPSDay = new wxSpinCtrl(this, ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_DAY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 31, day);
    textFPSDay->SetValue(day);
    textFPSDay->SetToolTip(_("Specify Day for start of financial year"));

    financialYearStaticBoxSizerGrid->Add(textFPSDay, g_flagsH);

    financialYearStaticBoxSizerGrid->Add(new wxStaticText(this, wxID_STATIC, _("Start Month")), g_flagsH);

    wxArrayString financialMonthsSelection;
    for (wxDateTime::Month m = wxDateTime::Jan; m <= wxDateTime::Dec; m = wxDateTime::Month(m + 1))
        financialMonthsSelection.Add(wxGetTranslation(wxDateTime::GetEnglishMonthName(m, wxDateTime::Name_Abbr)));

    m_month_selection = new wxChoice(this, ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_MONTH
        , wxDefaultPosition, wxSize(100, -1), financialMonthsSelection);
    financialYearStaticBoxSizerGrid->Add(m_month_selection, g_flagsH);

    int monthItem = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_MONTH", 7);
    m_month_selection->SetSelection(monthItem - 1);
    m_month_selection->SetToolTip(_("Specify month for start of financial year"));

    //a bit more space visual appearance
    generalPanelSizer->AddSpacer(15);

    m_use_org_date_copy_paste = new wxCheckBox(this, wxID_STATIC, _("Use Original Date when Pasting Transactions"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_use_org_date_copy_paste->SetValue(GetIniDatabaseCheckboxValue(INIDB_USE_ORG_DATE_COPYPASTE, false));
    m_use_org_date_copy_paste->SetToolTip(_("Select whether to use the original transaction date or current date when copying/pasting transactions"));
    generalPanelSizer->Add(m_use_org_date_copy_paste, g_flagsV);

    m_use_sound = new wxCheckBox(this, wxID_STATIC, _("Use Transaction Sound"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_use_sound->SetValue(GetIniDatabaseCheckboxValue(INIDB_USE_TRANSACTION_SOUND, true));
    m_use_sound->SetToolTip(_("Select whether to use sounds when entering transactions"));
    generalPanelSizer->Add(m_use_sound, g_flagsV);
}

void OptionSettingsGeneral::OnCurrency(wxCommandEvent& WXUNUSED(event))
{
    int currencyID = Option::instance().BaseCurrency();
    mmMainCurrencyDialog(this, currencyID, false).ShowModal();
    currencyID = Option::instance().BaseCurrency();
    Model_Currency::Data* currency = Model_Currency::instance().get(currencyID);
    wxButton* bn = (wxButton*)FindWindow(ID_DIALOG_OPTIONS_BUTTON_CURRENCY);
    bn->SetLabelText(wxGetTranslation(currency->CURRENCYNAME));
    m_currency_id = currencyID;
}

void OptionSettingsGeneral::OnDateFormatChanged(wxCommandEvent& WXUNUSED(event))
{
    wxStringClientData* data = (wxStringClientData*) (m_date_format_choice->GetClientObject(m_date_format_choice->GetSelection()));
    if (data)
    {
        m_date_format = data->GetData();
        m_sample_date_text->SetLabelText(wxDateTime::Now().Format(m_date_format));
    }
    else
    {
        return;
    }
}

bool OptionSettingsGeneral::SaveFinancialYearStart()
{
    //Save Financial Year Start Month
    int month = 1 + m_month_selection->GetSelection();
    wxString fysMonthVal = wxString::Format("%d", month);
    Option::instance().FinancialYearStartMonth(fysMonthVal);
    int last_month_day = wxDateTime(1, wxDateTime::Month(month-1), 2015).GetLastMonthDay().GetDay();

    //Save Financial Year Start Day
    wxSpinCtrl* fysDay = (wxSpinCtrl*) FindWindow(ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_DAY);
    int day = fysDay->GetValue();
    if (last_month_day < day)
        day = last_month_day;

    Option::instance().FinancialYearStartDay(wxString::Format("%d", day));
    return last_month_day < day;
}

void OptionSettingsGeneral::SaveSettings()
{
    wxTextCtrl* stun = (wxTextCtrl*) FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_USERNAME);
    Option::instance().UserName(stun->GetValue());

    //Model_Infotable::instance().SetBaseCurrency(m_currency_id); Handled only inside MainCurrencyDialog to better manage CurrencyHistory changes

    Option::instance().DateFormat(m_date_format);
    SaveFinancialYearStart();

    Model_Setting::instance().Set(INIDB_USE_ORG_DATE_COPYPASTE, m_use_org_date_copy_paste->GetValue());
    Model_Setting::instance().Set(INIDB_USE_TRANSACTION_SOUND, m_use_sound->GetValue());
}
