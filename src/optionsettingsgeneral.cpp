/*******************************************************
Copyright (C) 2014 Stefano Giorgio
Copyright (C) 2016, 2017, 2020 - 2022 Nikolay Akimov
Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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
#include "util.h"

#include "model/Model_Currency.h"

#include <wx/spinctrl.h>
#include <fmt/core.h>
#include <fmt/format.h>

/*******************************************************/
wxBEGIN_EVENT_TABLE(OptionSettingsGeneral, wxPanel)
    EVT_CHOICE(ID_DIALOG_OPTIONS_WXCHOICE_DATE, OptionSettingsGeneral::OnDateFormatChanged)
    EVT_COMBOBOX(ID_DIALOG_OPTIONS_LOCALE, OptionSettingsGeneral::OnLocaleChanged)
    EVT_MENU(wxID_ANY, OptionSettingsGeneral::OnChangeGUILanguage)
    EVT_BUTTON(ID_DIALOG_OPTIONS_BUTTON_LANG, OptionSettingsGeneral::OnMouseLeftDown)
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
    wxBoxSizer* generalPanelSizer0 = new wxBoxSizer(wxVERTICAL);
    SetSizer(generalPanelSizer0);

    wxScrolledWindow* general_panel = new wxScrolledWindow(this, wxID_ANY);

    wxBoxSizer* generalPanelSizer = new wxBoxSizer(wxVERTICAL);

    general_panel->SetSizer(generalPanelSizer);
    generalPanelSizer0->Add(general_panel, wxSizerFlags(g_flagsExpand).Proportion(0));

    // Display Header Settings
    wxStaticBox* headerStaticBox = new wxStaticBox(general_panel, wxID_STATIC, _("Display Heading"));
    SetBoldFont(headerStaticBox);

    wxStaticBoxSizer* headerStaticBoxSizer = new wxStaticBoxSizer(headerStaticBox, wxHORIZONTAL);

    headerStaticBoxSizer->Add(new wxStaticText(general_panel, wxID_STATIC, _("User Name")), g_flagsH);

    wxString userName = Model_Infotable::instance().GetStringInfo("USERNAME", "");
    wxTextCtrl* userNameTextCtr = new wxTextCtrl(general_panel, ID_DIALOG_OPTIONS_TEXTCTRL_USERNAME, userName);
    userNameTextCtr->SetMinSize(wxSize(200, -1));
    mmToolTip(userNameTextCtr, _("The User Name is used as a title for the database."));
    headerStaticBoxSizer->Add(userNameTextCtr, g_flagsExpand);
    generalPanelSizer->Add(headerStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    // Language
    auto language = Option::instance().getLanguageID(true);
    const auto langName = language == wxLANGUAGE_DEFAULT ? _("system default") : wxLocale::GetLanguageName(language);

    wxStaticBox* langStaticBox = new wxStaticBox(general_panel, wxID_STATIC, _("Language"));
    SetBoldFont(langStaticBox);
    wxStaticBoxSizer* langFormatStaticBoxSizer = new wxStaticBoxSizer(langStaticBox, wxHORIZONTAL);
    generalPanelSizer->Add(langFormatStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxButton* langButton = new wxButton(general_panel, ID_DIALOG_OPTIONS_BUTTON_LANG, langName);
    langButton->SetMinSize(wxSize(200, -1));
    langFormatStaticBoxSizer->Add(langButton, g_flagsH);
    mmToolTip(langButton, _("Change language used for MMEX GUI"));

    // Date Format Settings
    wxStaticBox* dateFormatStaticBox = new wxStaticBox(general_panel, wxID_STATIC, _("Date Format"));
    wxStaticBoxSizer* dateFormatStaticBoxSizer = new wxStaticBoxSizer(dateFormatStaticBox, wxHORIZONTAL);
    generalPanelSizer->Add(dateFormatStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_date_format_choice = new wxChoice(dateFormatStaticBox, ID_DIALOG_OPTIONS_WXCHOICE_DATE);
    for (const auto& i : g_date_formats_map())
    {
        m_date_format_choice->Append(i.second, new wxStringClientData(i.first));
        if (m_date_format == i.first) m_date_format_choice->SetStringSelection(i.second);
    }
    dateFormatStaticBoxSizer->Add(m_date_format_choice, g_flagsH);
    mmToolTip(m_date_format_choice, _("Specify the date format for display"));

    m_sample_date_text = new wxStaticText(dateFormatStaticBox, wxID_STATIC, "redefined elsewhere");
    dateFormatStaticBoxSizer->Add(new wxStaticText(dateFormatStaticBox, wxID_STATIC, _("Date format sample:")), wxSizerFlags(g_flagsH).Border(wxLEFT, 15));
    dateFormatStaticBoxSizer->Add(m_sample_date_text, wxSizerFlags(g_flagsH).Border(wxLEFT, 5));
    m_sample_date_text->SetLabelText(mmGetDateForDisplay(wxDateTime::Now().FormatISODate()));
    SetBoldFont(dateFormatStaticBox);

    // Currency Settings
    wxStaticBox* currencyStaticBox = new wxStaticBox(general_panel, wxID_STATIC, _("Currency"));
    SetBoldFont(currencyStaticBox);
    m_currencyStaticBoxSizer = new wxStaticBoxSizer(currencyStaticBox, wxVERTICAL);
    generalPanelSizer->Add(m_currencyStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxBoxSizer* currencyBaseSizer = new wxBoxSizer(wxHORIZONTAL);
    m_currencyStaticBoxSizer->Add(currencyBaseSizer, wxSizerFlags(g_flagsV).Border(wxLEFT, 0));
    currencyBaseSizer->Add(new wxStaticText(general_panel, wxID_STATIC, _("Base Currency")), g_flagsH);

    Model_Currency::Data* currency = Model_Currency::instance().get(Option::instance().getBaseCurrencyID());
    wxString currName = currency ? currency->CURRENCYNAME : _("Set Currency");
    m_currency_id = currency ? currency->CURRENCYID : -1;
    baseCurrencyComboBox_ = new mmComboBoxCurrency(general_panel, ID_DIALOG_OPTIONS_BUTTON_CURRENCY);
    baseCurrencyComboBox_->SetMinSize(wxSize(200, -1));
    baseCurrencyComboBox_->ChangeValue(currName);
    mmToolTip(baseCurrencyComboBox_, _("Sets the database default Currency using the 'Currency Dialog'"));
    currencyBaseSizer->Add(baseCurrencyComboBox_, g_flagsH);

    m_currencyStaticBoxSizer->AddSpacer(10);

    { // Locale
        const wxString locale = Model_Infotable::instance().GetStringInfo("LOCALE", "");

        wxBoxSizer* localeBaseSizer = new wxBoxSizer(wxHORIZONTAL);
        m_currencyStaticBoxSizer->Add(localeBaseSizer, wxSizerFlags(g_flagsV).Border(wxLEFT, 0));

        m_itemListOfLocales = new wxComboBox(general_panel, ID_DIALOG_OPTIONS_LOCALE, ""
            , wxDefaultPosition, wxDefaultSize, g_locales());
        m_itemListOfLocales->SetValue(locale);
        m_itemListOfLocales->AutoComplete(g_locales());
        m_itemListOfLocales->SetMinSize(wxSize(100, -1));
        localeBaseSizer->Add(m_itemListOfLocales, g_flagsH);

        m_sample_value_text = new wxStaticText(general_panel, wxID_STATIC, "redefined elsewhere");
        localeBaseSizer->Add(m_sample_value_text, wxSizerFlags(g_flagsH).Border(wxLEFT, 15));
        wxString result;
        doFormatDoubleValue(locale, result);
        m_sample_value_text->SetLabelText(wxGetTranslation(result));

        m_currencyStaticBoxSizer->Add(new wxStaticText(general_panel, wxID_STATIC
            , _("Format derived from locale.\n"
                "Leave blank to manually set format via 'Currency Dialog | Edit'")),
            wxSizerFlags(g_flagsV).Border(wxTOP, 0).Border(wxLEFT, 5));

        m_itemListOfLocales->Connect(ID_DIALOG_OPTIONS_LOCALE, wxEVT_COMMAND_TEXT_UPDATED
            , wxCommandEventHandler(OptionSettingsGeneral::OnLocaleChanged), nullptr, this);
        m_itemListOfLocales->SetMinSize(wxSize(100, -1));
    }

    m_currencyStaticBoxSizer->AddSpacer(15);

    m_currency_history = new wxCheckBox(general_panel, wxID_STATIC, _("Use currency history"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_currency_history->SetValue(Option::instance().getCurrencyHistoryEnabled());
    mmToolTip(m_currency_history, _("Select to use currency history (one rate for each day), deselect to use a fixed rate"));
    m_currencyStaticBoxSizer->Add(m_currency_history, g_flagsV);

    // Financial Year Settings
    wxStaticBox* financialYearStaticBox = new wxStaticBox(general_panel, wxID_ANY, _("Financial Year"));
    SetBoldFont(financialYearStaticBox);
    wxStaticBoxSizer* financialYearStaticBoxSizer = new wxStaticBoxSizer(financialYearStaticBox, wxVERTICAL);
    wxFlexGridSizer* financialYearStaticBoxSizerGrid = new wxFlexGridSizer(0, 2, 0, 0);
    generalPanelSizer->Add(financialYearStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));
    financialYearStaticBoxSizer->Add(financialYearStaticBoxSizerGrid);

    financialYearStaticBoxSizerGrid->Add(new wxStaticText(general_panel, wxID_STATIC, _("Start Day")), g_flagsH);
    int day = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_DAY", 1);

    wxSpinCtrl *textFPSDay = new wxSpinCtrl(general_panel, ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_DAY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 31, day);
    textFPSDay->SetValue(day);
    mmToolTip(textFPSDay, _("Specify Day for start of financial year"));

    financialYearStaticBoxSizerGrid->Add(textFPSDay, g_flagsH);

    financialYearStaticBoxSizerGrid->Add(new wxStaticText(general_panel, wxID_STATIC, _("Start Month")), g_flagsH);

    wxArrayString financialMonthsSelection;
    for (wxDateTime::Month m = wxDateTime::Jan; m <= wxDateTime::Dec; m = wxDateTime::Month(m + 1))
        financialMonthsSelection.Add(wxGetTranslation(wxDateTime::GetEnglishMonthName(m, wxDateTime::Name_Abbr)));

    m_month_selection = new wxChoice(general_panel, ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_MONTH
        , wxDefaultPosition, wxSize(100, -1), financialMonthsSelection);
    financialYearStaticBoxSizerGrid->Add(m_month_selection, g_flagsH);

    int monthItem = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_MONTH", 7);
    m_month_selection->SetSelection(monthItem - 1);
    mmToolTip(m_month_selection, _("Specify month for start of financial year"));

    // Misc settings
    generalPanelSizer->AddSpacer(15);

    m_use_org_date_copy_paste = new wxCheckBox(general_panel, wxID_STATIC, _("Use Original Date when Pasting Transactions"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_use_org_date_copy_paste->SetValue(GetIniDatabaseCheckboxValue(INIDB_USE_ORG_DATE_COPYPASTE, false));
    mmToolTip(m_use_org_date_copy_paste, _("Select whether to use the original transaction date or current date when copying/pasting transactions"));
    generalPanelSizer->Add(m_use_org_date_copy_paste, g_flagsV);

    m_use_org_date_duplicate = new wxCheckBox(general_panel, wxID_STATIC, _("Use Original Date when Duplicating Transactions"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_use_org_date_duplicate->SetValue(GetIniDatabaseCheckboxValue(INIDB_USE_ORG_DATE_DUPLICATE, false));
    mmToolTip(m_use_org_date_duplicate, _("Select whether to use the original transaction date or current date when duplicating transactions"));
    generalPanelSizer->Add(m_use_org_date_duplicate, g_flagsV);

    m_use_sound = new wxCheckBox(general_panel, wxID_STATIC, _("Use Transaction Sound"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_use_sound->SetValue(GetIniDatabaseCheckboxValue(INIDB_USE_TRANSACTION_SOUND, true));
    mmToolTip(m_use_sound, _("Select whether to use sounds when entering transactions"));
    generalPanelSizer->Add(m_use_sound, g_flagsV);

    Fit();
    general_panel->SetMinSize(general_panel->GetBestVirtualSize());
    general_panel->SetScrollRate(1, 1);
}

void OptionSettingsGeneral::OnDateFormatChanged(wxCommandEvent& /*event*/)
{
    wxStringClientData* data = static_cast<wxStringClientData*>(m_date_format_choice->GetClientObject(m_date_format_choice->GetSelection()));
    if (data)
    {
        m_date_format = data->GetData();
        m_sample_date_text->SetLabelText(mmGetDateForDisplay(wxDateTime::Now().FormatISODate(), m_date_format));
    }

}


void OptionSettingsGeneral::OnLocaleChanged(wxCommandEvent& /*event*/)
{
    wxComboBox* cbln = static_cast<wxComboBox*>(FindWindow(ID_DIALOG_OPTIONS_LOCALE));
    const wxString locale = cbln->GetValue();
    wxString result;

    doFormatDoubleValue(locale, result);
    m_sample_value_text->SetLabelText(wxGetTranslation(result));
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
    int baseCurrencyOLD = Option::instance().getBaseCurrencyID();
    int currency_id = baseCurrencyComboBox_->mmGetId();
    if (currency_id != baseCurrencyOLD)
    {
        if (!baseCurrencyComboBox_->mmIsValid())
        {
            mmErrorDialogs::ToolTip4Object(baseCurrencyComboBox_, _("Invalid value"), _("Currency"), wxICON_ERROR);
            return false;
        }
        m_currency_id = currency_id;

        if (Option::instance().getCurrencyHistoryEnabled())
        {
            if (wxMessageBox(_("Changing base currency will delete all history rates, proceed?")
                , _("Currency Dialog")
                , wxYES_NO | wxYES_DEFAULT | wxICON_WARNING) != wxYES)
                return false;
        }

        Option::instance().setBaseCurrency(m_currency_id);
    }

    wxTextCtrl* stun = static_cast<wxTextCtrl*>(FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_USERNAME));
    Option::instance().UserName(stun->GetValue());

    wxComboBox* cbln = static_cast<wxComboBox*>(FindWindow(ID_DIALOG_OPTIONS_LOCALE));
    wxString value;
    if (doFormatDoubleValue(cbln->GetValue(), value)) {
        Option::instance().LocaleName(cbln->GetValue());
    }
    else {
        mmErrorDialogs::ToolTip4Object(m_itemListOfLocales, _("Invalid value"), _("Locale"), wxICON_ERROR);
        return false;
    }

    Option::instance().CurrencyHistoryEnabled(m_currency_history->GetValue());

    Option::instance().setDateFormat(m_date_format);
    SaveFinancialYearStart();

    Model_Setting::instance().Set(INIDB_USE_ORG_DATE_COPYPASTE, m_use_org_date_copy_paste->GetValue());
    Model_Setting::instance().Set(INIDB_USE_ORG_DATE_DUPLICATE, m_use_org_date_duplicate->GetValue());
    Model_Setting::instance().Set(INIDB_USE_TRANSACTION_SOUND, m_use_sound->GetValue());

    return true;
}

bool OptionSettingsGeneral::doFormatDoubleValue(const wxString& locale, wxString& result)
{
    if (locale.empty()) {
        result = "";
        return true;
    }

    try {
        auto test = fmt::format(std::locale(locale.c_str()), "{:.2Lf}", 1234567.89);
        for (auto &i : test) {
            if (i < 0) i = ' ';
        }
        result = wxString::Format(_("Currency value sample: %s"), test);
    }
    catch (std::exception & ex) {
        result = wxString(ex.what());
        result.Replace("std::locale::facet::_S_create_c_locale name not valid", wxTRANSLATE("bad locale name"));
        return false;
    }

    return true;
}
void OptionSettingsGeneral::OnChangeGUILanguage(wxCommandEvent& event)
{
    wxLanguage lang = static_cast<wxLanguage>(event.GetId() - wxID_LAST - 1);
    if (lang != m_app->getGUILanguage() && m_app->setGUILanguage(lang))
        mmErrorDialogs::MessageWarning(this
            , _("The language for this application has been changed. "
                "The change will take effect the next time the application is started.")
            , _("Language change"));
}

void OptionSettingsGeneral::OnMouseLeftDown(wxCommandEvent& event)
{
    wxMenu menuLang;
    wxArrayString lang_files = wxTranslations::Get()->GetAvailableTranslations("mmex");
    std::map<wxString, std::pair<int, wxString>> langs;
    menuLang.AppendRadioItem(wxID_LAST + 1 + wxLANGUAGE_DEFAULT, _("system default"))
        ->Check(m_app->getGUILanguage() == wxLANGUAGE_DEFAULT);
    for (auto & file : lang_files)
    {
        const wxLanguageInfo* info = wxLocale::FindLanguageInfo(file);
        if (info)
            langs[info->Description] = std::make_pair(info->Language, info->CanonicalName);
    }
    langs[wxLocale::GetLanguageName(wxLANGUAGE_ENGLISH_US)] = std::make_pair(wxLANGUAGE_ENGLISH_US, "en_US");
    for (auto const& lang : langs)
    {
        menuLang.AppendRadioItem(wxID_LAST + 1 + lang.second.first, lang.first, lang.second.second)
            ->Check(lang.second.first == m_app->getGUILanguage());
    }
    PopupMenu(&menuLang);

    event.Skip();
}
