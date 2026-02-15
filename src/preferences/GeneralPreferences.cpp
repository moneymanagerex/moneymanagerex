/*******************************************************
Copyright (C) 2014 Stefano Giorgio
Copyright (C) 2016, 2017, 2020 - 2022 Nikolay Akimov
Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)
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

#include "base/defs.h"
#include <wx/spinctrl.h>
#include <fmt/core.h>
#include <fmt/format.h>

#include "util/_util.h"
#include "model/CurrencyModel.h"

#include "dialog/CurrencyChoiceDialog.h"

#include "GeneralPreferences.h"

/*******************************************************/
wxBEGIN_EVENT_TABLE(GeneralPreferences, wxPanel)
    EVT_CHOICE(ID_DIALOG_OPTIONS_WXCHOICE_DATE, GeneralPreferences::OnDateFormatChanged)
    EVT_COMBOBOX(ID_DIALOG_OPTIONS_LOCALE, GeneralPreferences::OnLocaleChanged)
    EVT_MENU(wxID_ANY, GeneralPreferences::OnChangeGUILanguage)
    EVT_BUTTON(ID_DIALOG_OPTIONS_BUTTON_LANG, GeneralPreferences::OnMouseLeftDown)
wxEND_EVENT_TABLE()
/*******************************************************/

GeneralPreferences::GeneralPreferences()
{
}

GeneralPreferences::GeneralPreferences(wxWindow *parent, mmGUIApp* app
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
{
    wxPanel::Create(parent, id, pos, size, style, name);
    m_app = app;
    m_date_format = PreferencesModel::instance().getDateFormat();

    Create();
}

GeneralPreferences::~GeneralPreferences()
{
}

void GeneralPreferences::Create()
{
    wxBoxSizer* generalPanelSizer0 = new wxBoxSizer(wxVERTICAL);
    SetSizer(generalPanelSizer0);

    wxScrolledWindow* general_panel = new wxScrolledWindow(this, wxID_ANY);

    wxBoxSizer* generalPanelSizer = new wxBoxSizer(wxVERTICAL);

    general_panel->SetSizer(generalPanelSizer);
    generalPanelSizer0->Add(general_panel, wxSizerFlags(g_flagsExpand).Proportion(0));

    // Display Header Settings
    wxStaticBox* headerStaticBox = new wxStaticBox(general_panel, wxID_STATIC, _t("Display Heading"));


    wxStaticBoxSizer* headerStaticBoxSizer = new wxStaticBoxSizer(headerStaticBox, wxHORIZONTAL);

    headerStaticBoxSizer->Add(new wxStaticText(headerStaticBox, wxID_STATIC, _t("User Name")), g_flagsH);

    wxString userName = InfoModel::instance().getString("USERNAME", "");
    wxTextCtrl* userNameTextCtr = new wxTextCtrl(headerStaticBox, ID_DIALOG_OPTIONS_TEXTCTRL_USERNAME, userName);
    userNameTextCtr->SetMinSize(wxSize(200, -1));
    mmToolTip(userNameTextCtr, _t("The User Name is used as a title for the database."));
    headerStaticBoxSizer->Add(userNameTextCtr, g_flagsExpand);
    generalPanelSizer->Add(headerStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    // User Interface Language
    auto language = PreferencesModel::instance().getLanguageID(true);
    const auto langName = language == wxLANGUAGE_DEFAULT ? _t("System default") : wxLocale::GetLanguageName(language);

    wxStaticBox* langStaticBox = new wxStaticBox(general_panel, wxID_STATIC, _t("User Interface Language"));

    wxStaticBoxSizer* langFormatStaticBoxSizer = new wxStaticBoxSizer(langStaticBox, wxHORIZONTAL);
    generalPanelSizer->Add(langFormatStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxButton* langButton = new wxButton(langStaticBox, ID_DIALOG_OPTIONS_BUTTON_LANG, wxGetTranslation(langName));
    langButton->SetMinSize(wxSize(200, -1));
    langFormatStaticBoxSizer->Add(langButton, g_flagsH);
    mmToolTip(langButton, _t("Change user interface language"));

    // Date Format Settings
    wxStaticBox* dateFormatStaticBox = new wxStaticBox(general_panel, wxID_STATIC, _t("Date Format"));
    wxStaticBoxSizer* dateFormatStaticBoxSizer = new wxStaticBoxSizer(dateFormatStaticBox, wxHORIZONTAL);
    generalPanelSizer->Add(dateFormatStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_date_format_choice = new wxChoice(dateFormatStaticBox, ID_DIALOG_OPTIONS_WXCHOICE_DATE);
    for (const auto& i : g_date_formats_map())
    {
        m_date_format_choice->Append(i.second, new wxStringClientData(i.first));
        if (m_date_format == i.first) m_date_format_choice->SetStringSelection(i.second);
    }
    dateFormatStaticBoxSizer->Add(m_date_format_choice, g_flagsH);
    mmToolTip(m_date_format_choice, _t("Specify the date format for display"));

    m_sample_date_text = new wxStaticText(dateFormatStaticBox, wxID_STATIC, "redefined elsewhere");
    dateFormatStaticBoxSizer->Add(new wxStaticText(dateFormatStaticBox, wxID_STATIC, _t("Date format sample:")), wxSizerFlags(g_flagsH).Border(wxLEFT, 15));
    dateFormatStaticBoxSizer->Add(m_sample_date_text, wxSizerFlags(g_flagsH).Border(wxLEFT, 5));
    m_sample_date_text->SetLabelText(mmGetDateTimeForDisplay(wxDateTime::Now().FormatISODate()));


    // Currency Settings
    wxStaticBox* currencyStaticBox = new wxStaticBox(general_panel, wxID_STATIC, _t("Currency"));

    m_currencyStaticBoxSizer = new wxStaticBoxSizer(currencyStaticBox, wxVERTICAL);
    generalPanelSizer->Add(m_currencyStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxBoxSizer* currencyBaseSizer = new wxBoxSizer(wxHORIZONTAL);
    m_currencyStaticBoxSizer->Add(currencyBaseSizer, wxSizerFlags(g_flagsV).Border(wxLEFT, 0));
    currencyBaseSizer->Add(new wxStaticText(currencyStaticBox, wxID_STATIC, _t("Base Currency")), g_flagsH);

    CurrencyModel::Data* currency = CurrencyModel::instance().cache_id(PreferencesModel::instance().getBaseCurrencyID());
    wxString currName = currency ? currency->CURRENCYNAME : _t("Set Currency");
    m_currency_id = currency ? currency->CURRENCYID : -1;
    baseCurrencyComboBox_ = new mmComboBoxCurrency(currencyStaticBox, ID_DIALOG_OPTIONS_BUTTON_CURRENCY);
    baseCurrencyComboBox_->SetMinSize(wxSize(200, -1));
    baseCurrencyComboBox_->ChangeValue(currName);
    mmToolTip(baseCurrencyComboBox_, _t("Set default database currency using 'Currency Manager'"));
    currencyBaseSizer->Add(baseCurrencyComboBox_, g_flagsH);

    m_currencyStaticBoxSizer->AddSpacer(10);

    { // Locale
        const wxString locale = InfoModel::instance().getString("LOCALE", "");

        wxBoxSizer* localeBaseSizer = new wxBoxSizer(wxHORIZONTAL);
        m_currencyStaticBoxSizer->Add(localeBaseSizer, wxSizerFlags(g_flagsV).Border(wxLEFT, 0));

        m_itemListOfLocales = new wxComboBox(currencyStaticBox, ID_DIALOG_OPTIONS_LOCALE, ""
            , wxDefaultPosition, wxDefaultSize, g_locales());
        m_itemListOfLocales->SetValue(locale);
        m_itemListOfLocales->AutoComplete(g_locales());
        m_itemListOfLocales->SetMinSize(wxSize(100, -1));
        localeBaseSizer->Add(m_itemListOfLocales, g_flagsH);

        m_sample_value_text = new wxStaticText(currencyStaticBox, wxID_STATIC, "redefined elsewhere");
        localeBaseSizer->Add(m_sample_value_text, wxSizerFlags(g_flagsH).Border(wxLEFT, 15));
        wxString result;
        doFormatDoubleValue(locale, result);
        m_sample_value_text->SetLabelText(wxGetTranslation(result));

        m_currencyStaticBoxSizer->Add(new wxStaticText(currencyStaticBox, wxID_STATIC
            , _t("Format derived from locale.\n"
                "Leave blank to manually set format via 'Currency Manager | Edit'")),
            wxSizerFlags(g_flagsV).Border(wxTOP, 0).Border(wxLEFT, 5));

        m_itemListOfLocales->Connect(ID_DIALOG_OPTIONS_LOCALE, wxEVT_COMMAND_TEXT_UPDATED
            , wxCommandEventHandler(GeneralPreferences::OnLocaleChanged), nullptr, this);
        m_itemListOfLocales->SetMinSize(wxSize(100, -1));
    }

    m_currencyStaticBoxSizer->AddSpacer(15);

    m_currency_history = new wxCheckBox(currencyStaticBox, wxID_ANY, _t("Use historical currency"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_currency_history->SetValue(PreferencesModel::instance().getUseCurrencyHistory());
    mmToolTip(m_currency_history, _t("Select to use historical currency (one rate for each day), deselect to use a fixed rate"));
    m_currencyStaticBoxSizer->Add(m_currency_history, g_flagsV);

    // Financial Year Settings
    wxStaticBox* financialYearStaticBox = new wxStaticBox(general_panel, wxID_ANY, _t("Financial Year"));

    wxStaticBoxSizer* financialYearStaticBoxSizer = new wxStaticBoxSizer(financialYearStaticBox, wxVERTICAL);
    wxFlexGridSizer* financialYearStaticBoxSizerGrid = new wxFlexGridSizer(0, 2, 0, 0);
    generalPanelSizer->Add(financialYearStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));
    financialYearStaticBoxSizer->Add(financialYearStaticBoxSizerGrid);

    financialYearStaticBoxSizerGrid->Add(new wxStaticText(financialYearStaticBox, wxID_STATIC, _t("First Day")), g_flagsH);
    int day = InfoModel::instance().getInt("FINANCIAL_YEAR_START_DAY", 1);

    wxSpinCtrl *textFPSDay = new wxSpinCtrl(financialYearStaticBox, ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_DAY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 31, day);
    textFPSDay->SetValue(day);
    mmToolTip(textFPSDay, _t("Specify Day for start of financial year"));

    financialYearStaticBoxSizerGrid->Add(textFPSDay, g_flagsH);

    financialYearStaticBoxSizerGrid->Add(new wxStaticText(financialYearStaticBox, wxID_STATIC, _t("First Month")), g_flagsH);

    wxArrayString financialMonthsSelection;
    for (wxDateTime::Month m = wxDateTime::Jan; m <= wxDateTime::Dec; m = wxDateTime::Month(m + 1))
        financialMonthsSelection.Add(wxGetTranslation(wxDateTime::GetEnglishMonthName(m, wxDateTime::Name_Abbr)));

    m_month_selection = new wxChoice(financialYearStaticBox, ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_MONTH
        , wxDefaultPosition, wxSize(100, -1), financialMonthsSelection);
    financialYearStaticBoxSizerGrid->Add(m_month_selection, g_flagsH);

    int monthItem = InfoModel::instance().getInt("FINANCIAL_YEAR_START_MONTH", 7);
    m_month_selection->SetSelection(monthItem - 1);
    mmToolTip(m_month_selection, _t("Specify month for start of financial year"));

    SetBoldFontToStaticBoxHeader(dateFormatStaticBox);
    SetBoldFontToStaticBoxHeader(langStaticBox);
    SetBoldFontToStaticBoxHeader(headerStaticBox);
    SetBoldFontToStaticBoxHeader(currencyStaticBox);
    SetBoldFontToStaticBoxHeader(financialYearStaticBox);

    Fit();
    general_panel->SetMinSize(general_panel->GetBestVirtualSize());
    general_panel->SetScrollRate(6, 6);
}

void GeneralPreferences::OnDateFormatChanged(wxCommandEvent& /*event*/)
{
    wxStringClientData* data = static_cast<wxStringClientData*>(m_date_format_choice->GetClientObject(m_date_format_choice->GetSelection()));
    if (data)
    {
        m_date_format = data->GetData();
        m_sample_date_text->SetLabelText(mmGetDateTimeForDisplay(wxDateTime::Now().FormatISODate(), m_date_format));
    }

}

void GeneralPreferences::OnLocaleChanged(wxCommandEvent& /*event*/)
{
    wxComboBox* cbln = static_cast<wxComboBox*>(FindWindow(ID_DIALOG_OPTIONS_LOCALE));
    const wxString locale = cbln->GetValue();
    wxString result;

    doFormatDoubleValue(locale, result);
    m_sample_value_text->SetLabelText(wxGetTranslation(result));
}

bool GeneralPreferences::SaveFinancialYearStart()
{
    //Save Financial Year Start Month
    wxDateTime::Month month = wxDateTime::Month(m_month_selection->GetSelection());
    PreferencesModel::instance().setFinancialFirstMonth(month);
    int last_month_day = wxDateTime(1, month, 2015).GetLastMonthDay().GetDay();

    //Save Financial Year Start Day
    wxSpinCtrl* fysDay = static_cast<wxSpinCtrl*>(FindWindow(ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_DAY));
    int day = fysDay->GetValue();
    if (last_month_day < day)
        day = last_month_day;

    PreferencesModel::instance().setFinancialFirstDay(day);
    return last_month_day < day;
}

bool GeneralPreferences::SaveSettings()
{
    int64 baseCurrencyOLD = PreferencesModel::instance().getBaseCurrencyID();
    int64 currency_id = baseCurrencyComboBox_->mmGetId();
    if (currency_id != baseCurrencyOLD)
    {
        if (!baseCurrencyComboBox_->mmIsValid())
        {
            mmErrorDialogs::ToolTip4Object(baseCurrencyComboBox_, _t("Invalid value"), _t("Currency"), wxICON_ERROR);
            return false;
        }
        m_currency_id = currency_id;

        if (PreferencesModel::instance().getUseCurrencyHistory())
        {
            if (wxMessageBox(_t("Changing base currency will delete all historical rates, proceed?")
                , _t("Currency Manager")
                , wxYES_NO | wxYES_DEFAULT | wxICON_WARNING) != wxYES)
                return false;
        }

        PreferencesModel::instance().setBaseCurrencyID(m_currency_id);
    }

    wxTextCtrl* stun = static_cast<wxTextCtrl*>(FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_USERNAME));
    PreferencesModel::instance().setUserName(stun->GetValue());

    wxComboBox* cbln = static_cast<wxComboBox*>(FindWindow(ID_DIALOG_OPTIONS_LOCALE));
    wxString value;
    if (doFormatDoubleValue(cbln->GetValue(), value)) {
        PreferencesModel::instance().setLocaleName(cbln->GetValue());
    }
    else {
        mmErrorDialogs::ToolTip4Object(m_itemListOfLocales, _t("Invalid value"), _t("Locale"), wxICON_ERROR);
        return false;
    }

    PreferencesModel::instance().setUseCurrencyHistory(m_currency_history->GetValue());

    PreferencesModel::instance().setDateFormat(m_date_format);
    SaveFinancialYearStart();

    return true;
}

bool GeneralPreferences::doFormatDoubleValue(const wxString& locale, wxString& result)
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
        result = wxString::Format(_t("Currency value sample: %s"), test);
    }
    catch (std::exception & ex) {
        result = wxString(ex.what());
        if (result.Contains("locale name not valid")) {
            result = wxTRANSLATE("Bad locale name");
        }
        return false;
    }

    return true;
}

void GeneralPreferences::OnChangeGUILanguage(wxCommandEvent& event)
{
    wxLanguage lang = static_cast<wxLanguage>(event.GetId() - wxID_LAST - 1);
    if (lang != m_app->getGUILanguage() && m_app->setGUILanguage(lang))
        mmErrorDialogs::MessageWarning(this
            , _t("The language for this application has been changed. "
                "The change will take effect the next time the application is started.")
            , _t("Language change"));
}

void GeneralPreferences::OnMouseLeftDown(wxCommandEvent& event)
{
    wxMenu menuLang;
    wxArrayString lang_files = wxTranslations::Get()->GetAvailableTranslations("mmex");
    std::map<wxString, std::pair<int, wxString>> langs;
    menuLang.AppendRadioItem(wxID_LAST + 1 + wxLANGUAGE_DEFAULT, _t("System default"))
        ->Check(m_app->getGUILanguage() == wxLANGUAGE_DEFAULT);
    for (auto & file : lang_files)
    {
        const wxLanguageInfo* info = wxLocale::FindLanguageInfo(file);
        if (info)
            langs[wxGetTranslation(info->Description)] = std::make_pair(info->Language, info->CanonicalName);
    }
    langs[wxGetTranslation(wxLocale::GetLanguageName(wxLANGUAGE_ENGLISH_US))] = std::make_pair(wxLANGUAGE_ENGLISH_US, "en_US");
    for (auto const& lang : langs)
    {
        menuLang.AppendRadioItem(wxID_LAST + 1 + lang.second.first, lang.first, lang.second.second)
            ->Check(lang.second.first == m_app->getGUILanguage());
    }
    PopupMenu(&menuLang);

    event.Skip();
}
