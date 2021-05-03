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

#include "model/Model_Currency.h"

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
    m_date_format = Option::instance().getDateFormat();

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
    wxTextCtrl* userNameTextCtr = new wxTextCtrl(this, ID_DIALOG_OPTIONS_TEXTCTRL_USERNAME, userName);
    userNameTextCtr->SetMinSize(wxSize(200, -1));
    userNameTextCtr->SetToolTip(_("The User Name is used as a title for the database."));
    headerStaticBoxSizer->Add(userNameTextCtr, g_flagsExpand);
    generalPanelSizer->Add(headerStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    // Locale
    wxStaticBox* localeStaticBox = new wxStaticBox(this, wxID_STATIC, _("Locale"));
    wxStaticBoxSizer* localeStaticBoxSizer = new wxStaticBoxSizer(localeStaticBox, wxHORIZONTAL);
    localeStaticBoxSizer->Add(new wxStaticText(localeStaticBox, wxID_STATIC, _("Locale")), g_flagsH);
    const wxString locale = Model_Infotable::instance().GetStringInfo("LOCALE", "en_US");

    wxArrayString list;
    list.Add("af-ZA");
    list.Add("am-ET");
    list.Add("ar-AE");
    list.Add("ar-BH");
    list.Add("ar-DZ");
    list.Add("ar-EG");
    list.Add("ar-IQ");
    list.Add("ar-JO");
    list.Add("ar-KW");
    list.Add("ar-LB");
    list.Add("ar-LY");
    list.Add("ar-MA");
    list.Add("ar-OM");
    list.Add("ar-QA");
    list.Add("ar-SA");
    list.Add("ar-SY");
    list.Add("ar-TN");
    list.Add("ar-YE");
    list.Add("as-IN");
    list.Add("ba-RU");
    list.Add("be-BY");
    list.Add("bg-BG");
    list.Add("bn-BD");
    list.Add("bn-IN");
    list.Add("bo-CN");
    list.Add("br-FR");
    list.Add("ca-ES");
    list.Add("co-FR");
    list.Add("cs-CZ");
    list.Add("cy-GB");
    list.Add("da-DK");
    list.Add("de-AT");
    list.Add("de-CH");
    list.Add("de-DE");
    list.Add("de-LI");
    list.Add("de-LU");
    list.Add("dv-MV");
    list.Add("el-GR");
    list.Add("en-AU");
    list.Add("en-BZ");
    list.Add("en-CA");
    list.Add("en-GB");
    list.Add("en-IE");
    list.Add("en-IN");
    list.Add("en-JM");
    list.Add("en-MY");
    list.Add("en-NZ");
    list.Add("en-PH");
    list.Add("en-SG");
    list.Add("en-TT");
    list.Add("en-US");
    list.Add("en-ZA");
    list.Add("en-ZW");
    list.Add("es-AR");
    list.Add("es-BO");
    list.Add("es-CL");
    list.Add("es-CO");
    list.Add("es-CR");
    list.Add("es-DO");
    list.Add("es-EC");
    list.Add("es-ES");
    list.Add("es-GT");
    list.Add("es-HN");
    list.Add("es-MX");
    list.Add("es-NI");
    list.Add("es-PA");
    list.Add("es-PE");
    list.Add("es-PR");
    list.Add("es-PY");
    list.Add("es-SV");
    list.Add("es-US");
    list.Add("es-UY");
    list.Add("es-VE");
    list.Add("et-EE");
    list.Add("eu-ES");
    list.Add("fa-IR");
    list.Add("fi-FI");
    list.Add("fo-FO");
    list.Add("fr-BE");
    list.Add("fr-CA");
    list.Add("fr-CH");
    list.Add("fr-FR");
    list.Add("fr-LU");
    list.Add("fr-MC");
    list.Add("fy-NL");
    list.Add("ga-IE");
    list.Add("gd-GB");
    list.Add("gl-ES");
    list.Add("gu-IN");
    list.Add("he-IL");
    list.Add("hi-IN");
    list.Add("hr-BA");
    list.Add("hr-HR");
    list.Add("hu-HU");
    list.Add("hy-AM");
    list.Add("id-ID");
    list.Add("ig-NG");
    list.Add("ii-CN");
    list.Add("is-IS");
    list.Add("it-CH");
    list.Add("it-IT");
    list.Add("ja-JP");
    list.Add("ka-GE");
    list.Add("kk-KZ");
    list.Add("kl-GL");
    list.Add("km-KH");
    list.Add("kn-IN");
    list.Add("ko-KR");
    list.Add("ky-KG");
    list.Add("lb-LU");
    list.Add("lo-LA");
    list.Add("lt-LT");
    list.Add("lv-LV");
    list.Add("mi-NZ");
    list.Add("mk-MK");
    list.Add("ml-IN");
    list.Add("mn-MN");
    list.Add("mr-IN");
    list.Add("ms-BN");
    list.Add("ms-MY");
    list.Add("mt-MT");
    list.Add("nb-NO");
    list.Add("ne-NP");
    list.Add("nl-BE");
    list.Add("nl-NL");
    list.Add("nn-NO");
    list.Add("oc-FR");
    list.Add("or-IN");
    list.Add("pa-IN");
    list.Add("pl-PL");
    list.Add("ps-AF");
    list.Add("pt-BR");
    list.Add("pt-PT");
    list.Add("rm-CH");
    list.Add("ro-RO");
    list.Add("ru-RU");
    list.Add("rw-RW");
    list.Add("sa-IN");
    list.Add("se-FI");
    list.Add("se-NO");
    list.Add("se-SE");
    list.Add("si-LK");
    list.Add("sk-SK");
    list.Add("sl-SI");
    list.Add("sq-AL");
    list.Add("sv-FI");
    list.Add("sv-SE");
    list.Add("sw-KE");
    list.Add("ta-IN");
    list.Add("te-IN");
    list.Add("th-TH");
    list.Add("tk-TM");
    list.Add("tn-ZA");
    list.Add("tr-TR");
    list.Add("tt-RU");
    list.Add("ug-CN");
    list.Add("uk-UA");
    list.Add("ur-PK");
    list.Add("vi-VN");
    list.Add("wo-SN");
    list.Add("xh-ZA");
    list.Add("yo-NG");
    list.Add("zh-CN");
    list.Add("zh-HK");
    list.Add("zh-MO");
    list.Add("zh-SG");
    list.Add("zh-TW");
    list.Add("zu-ZA");

    wxComboBox* itemListOfLocales = new wxComboBox(localeStaticBox, ID_DIALOG_OPTIONS_TEXTCTRL_LOCALE, ""
        , wxDefaultPosition, wxDefaultSize, list);
    itemListOfLocales->SetValue(locale);
    itemListOfLocales->SetMinSize(wxSize(200, -1));
    localeStaticBoxSizer->Add(itemListOfLocales, g_flagsExpand);
    generalPanelSizer->Add(localeStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));
    SetBoldFont(localeStaticBox);

    // Currency Settings
    wxStaticBox* currencyStaticBox = new wxStaticBox(this, wxID_STATIC, _("Currency"));
    SetBoldFont(currencyStaticBox);
    m_currencyStaticBoxSizer = new wxStaticBoxSizer(currencyStaticBox, wxVERTICAL);
    generalPanelSizer->Add(m_currencyStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxBoxSizer* currencyBaseSizer = new wxBoxSizer(wxHORIZONTAL);
    m_currencyStaticBoxSizer->Add(currencyBaseSizer, wxSizerFlags(g_flagsV).Border(wxLEFT, 0));
    currencyBaseSizer->Add(new wxStaticText(this, wxID_STATIC, _("Base Currency")), g_flagsH);

    
    Model_Currency::Data* currency = Model_Currency::instance().get(Option::instance().getBaseCurrencyID());
    wxString currName = currency ? currency->CURRENCYNAME : _("Set Currency");
    wxButton* baseCurrencyButton = new wxButton(this, ID_DIALOG_OPTIONS_BUTTON_CURRENCY, currName, wxDefaultPosition, wxDefaultSize);
    baseCurrencyButton->SetLabel(currName);
    baseCurrencyButton->SetToolTip(_("Sets the database default Currency using the 'Currency Dialog'"));
    currencyBaseSizer->Add(baseCurrencyButton, g_flagsH);
    m_currencyStaticBoxSizer->Add(new wxStaticText(this, wxID_STATIC, _("Right click and select 'Set as Base Currency' in 'Currency Dialog'")),
        wxSizerFlags(g_flagsV).Border(wxTOP, 0).Border(wxLEFT, 5));

    m_currencyStaticBoxSizer->AddSpacer(15);

    m_currency_history = new wxCheckBox(this, wxID_STATIC, _("Use currency history"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_currency_history->SetValue(Option::instance().getCurrencyHistoryEnabled());
    m_currency_history->SetToolTip(_("Select to use currency history (one rate for each day), deselect to use a fixed rate"));
    m_currencyStaticBoxSizer->Add(m_currency_history, g_flagsV);

    // Date Format Settings
    wxStaticBox* dateFormatStaticBox = new wxStaticBox(this, wxID_STATIC, _("Date Format"));
    SetBoldFont(dateFormatStaticBox);
    wxStaticBoxSizer* dateFormatStaticBoxSizer = new wxStaticBoxSizer(dateFormatStaticBox, wxHORIZONTAL);
    generalPanelSizer->Add(dateFormatStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_date_format_choice = new wxChoice(this, ID_DIALOG_OPTIONS_WXCHOICE_DATE);
    for (const auto& i : g_date_formats_map())
    {
        m_date_format_choice->Append(i.second, new wxStringClientData(i.first));
        if (m_date_format == i.first) m_date_format_choice->SetStringSelection(i.second);
    }
    dateFormatStaticBoxSizer->Add(m_date_format_choice, g_flagsH);
    m_date_format_choice->SetToolTip(_("Specify the date format for display"));

    m_sample_date_text = new wxStaticText(this, wxID_STATIC, "redefined elsewhere");
    dateFormatStaticBoxSizer->Add(new wxStaticText(this, wxID_STATIC, _("New date format sample:")), wxSizerFlags(g_flagsH).Border(wxLEFT, 15));
    dateFormatStaticBoxSizer->Add(m_sample_date_text, wxSizerFlags(g_flagsH).Border(wxLEFT, 5));
    m_sample_date_text->SetLabelText(mmGetDateForDisplay(wxDateTime::Now().FormatISODate()));

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

    // Misc settings
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

void OptionSettingsGeneral::OnCurrency(wxCommandEvent& /*event*/)
{
    int currencyID = Option::instance().getBaseCurrencyID();
    mmMainCurrencyDialog::Execute(this, currencyID);
    currencyID = Option::instance().getBaseCurrencyID();
    Model_Currency::Data* currency = Model_Currency::instance().get(currencyID);
    wxButton* bn = static_cast<wxButton*>(FindWindow(ID_DIALOG_OPTIONS_BUTTON_CURRENCY));
    bn->SetLabelText(currency->CURRENCYNAME);
    m_currencyStaticBoxSizer->Layout();
}

void OptionSettingsGeneral::OnDateFormatChanged(wxCommandEvent& /*event*/)
{
    wxStringClientData* data = static_cast<wxStringClientData*>(m_date_format_choice->GetClientObject(m_date_format_choice->GetSelection()));
    if (data)
    {
        m_date_format = data->GetData();
        mmGetDateForDisplay(wxDateTime::Now().FormatISODate(), ""); //clear the cache
        m_sample_date_text->SetLabelText(mmGetDateForDisplay(wxDateTime::Now().FormatISODate(), m_date_format));
    }

}

bool OptionSettingsGeneral::SaveFinancialYearStart()
{
    //Save Financial Year Start Month
    int month = 1 + m_month_selection->GetSelection();
    wxString fysMonthVal = wxString::Format("%d", month);
    Option::instance().FinancialYearStartMonth(fysMonthVal);
    int last_month_day = wxDateTime(1, wxDateTime::Month(month - 1), 2015).GetLastMonthDay().GetDay();

    //Save Financial Year Start Day
    wxSpinCtrl* fysDay = static_cast<wxSpinCtrl*>(FindWindow(ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_DAY));
    int day = fysDay->GetValue();
    if (last_month_day < day)
        day = last_month_day;

    Option::instance().FinancialYearStartDay(wxString::Format("%d", day));
    return last_month_day < day;
}

bool OptionSettingsGeneral::SaveSettings()
{
    wxTextCtrl* stun = static_cast<wxTextCtrl*>(FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_USERNAME));
    Option::instance().UserName(stun->GetValue());

    wxComboBox* cbln = static_cast<wxComboBox*>(FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_LOCALE));
    Option::instance().LocaleName(cbln->GetValue());

    Option::instance().CurrencyHistoryEnabled(m_currency_history->GetValue());

    Option::instance().setDateFormat(m_date_format);
    SaveFinancialYearStart();

    Model_Setting::instance().Set(INIDB_USE_ORG_DATE_COPYPASTE, m_use_org_date_copy_paste->GetValue());
    Model_Setting::instance().Set(INIDB_USE_TRANSACTION_SOUND, m_use_sound->GetValue());

    return true;
}
