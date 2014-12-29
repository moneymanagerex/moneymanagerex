#include "mmOptionGeneralSettings.h"
#include <wx/spinctrl.h>
#include "util.h"
#include "model/Model_Currency.h"
#include "maincurrencydialog.h"

/*******************************************************/
wxBEGIN_EVENT_TABLE(mmOptionGeneralSettings, wxPanel)
    EVT_BUTTON(ID_DIALOG_OPTIONS_BUTTON_CURRENCY, mmOptionGeneralSettings::OnCurrency)
    EVT_CHOICE(ID_DIALOG_OPTIONS_WXCHOICE_DATE, mmOptionGeneralSettings::OnDateFormatChanged)
    EVT_BUTTON(ID_DIALOG_OPTIONS_BUTTON_LANGUAGE, mmOptionGeneralSettings::OnLanguageChanged)
wxEND_EVENT_TABLE()
/*******************************************************/

mmOptionGeneralSettings::mmOptionGeneralSettings()
{
}

mmOptionGeneralSettings::mmOptionGeneralSettings(wxWindow *parent, mmGUIApp* app
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
{
    wxPanel::Create(parent, id, pos, size, style, name);
    m_app = app;
    m_currency_id = Model_Infotable::instance().GetBaseCurrencyId();
    m_date_format = Model_Infotable::instance().GetStringInfo("DATEFORMAT", mmex::DEFDATEFORMAT);

    Create();
}

mmOptionGeneralSettings::~mmOptionGeneralSettings()
{
}

void mmOptionGeneralSettings::Create()
{
    wxBoxSizer* generalPanelSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(generalPanelSizer);

    // Display Header Settings
    wxStaticBox* headerStaticBox = new wxStaticBox(this, wxID_STATIC, _("Display Heading"));
    SetBoldFont(headerStaticBox);

    wxStaticBoxSizer* headerStaticBoxSizer = new wxStaticBoxSizer(headerStaticBox, wxHORIZONTAL);

    headerStaticBoxSizer->Add(new wxStaticText(this, wxID_STATIC, _("User Name")), g_flags);

    wxString userName = Model_Infotable::instance().GetStringInfo("USERNAME", "");
    wxTextCtrl* userNameTextCtr = new wxTextCtrl(this, ID_DIALOG_OPTIONS_TEXTCTRL_USERNAME
        , userName, wxDefaultPosition, wxSize(200, -1));
    userNameTextCtr->SetToolTip(_("The User Name is used as a title for the database."));
    headerStaticBoxSizer->Add(userNameTextCtr, g_flagsExpand);
    generalPanelSizer->Add(headerStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    // Language Settings
    wxStaticBox* languageStaticBox = new wxStaticBox(this, wxID_STATIC, _("Language"));
    SetBoldFont(languageStaticBox);
    wxStaticBoxSizer* languageStaticBoxSizer = new wxStaticBoxSizer(languageStaticBox, wxHORIZONTAL);
    generalPanelSizer->Add(languageStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_current_language = Model_Setting::instance().GetStringSetting(LANGUAGE_PARAMETER, "english");
    wxButton* languageButton = new wxButton(this, ID_DIALOG_OPTIONS_BUTTON_LANGUAGE
        , m_current_language.Left(1).Upper() + m_current_language.SubString(1, m_current_language.Len())
        , wxDefaultPosition, wxSize(150, -1), 0);
    languageButton->SetToolTip(_("Specify the language to use"));
    languageStaticBoxSizer->Add(languageButton, g_flags);

    // Currency Settings
    wxStaticBox* currencyStaticBox = new wxStaticBox(this, wxID_STATIC, _("Currency"));
    SetBoldFont(currencyStaticBox);
    wxStaticBoxSizer* currencyStaticBoxSizer = new wxStaticBoxSizer(currencyStaticBox, wxHORIZONTAL);
    SetBoldFont(currencyStaticBox);
    generalPanelSizer->Add(currencyStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    currencyStaticBoxSizer->Add(new wxStaticText(this, wxID_STATIC, _("Base Currency")), g_flags);

    wxString currName = _("Set Currency");
    Model_Currency::Data* currency = Model_Currency::instance().get(m_currency_id);
    if (currency)
        currName = currency->CURRENCYNAME;
    wxButton* baseCurrencyButton = new wxButton(this, ID_DIALOG_OPTIONS_BUTTON_CURRENCY
        , currName, wxDefaultPosition, wxDefaultSize);
    baseCurrencyButton->SetToolTip(_("Sets the default currency for the database."));
    currencyStaticBoxSizer->Add(baseCurrencyButton, g_flags);

    // Date Format Settings
    wxStaticBox* dateFormatStaticBox = new wxStaticBox(this, wxID_STATIC, _("Date Format"));
    SetBoldFont(dateFormatStaticBox);
    wxStaticBoxSizer* dateFormatStaticBoxSizer = new wxStaticBoxSizer(dateFormatStaticBox, wxHORIZONTAL);
    generalPanelSizer->Add(dateFormatStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_date_format_choice = new wxChoice(this, ID_DIALOG_OPTIONS_WXCHOICE_DATE);
    for (const auto& i : date_formats_map())
    {
        m_date_format_choice->Append(i.second, new wxStringClientData(i.first));
        if (m_date_format == i.first) m_date_format_choice->SetStringSelection(i.second);
    }
    dateFormatStaticBoxSizer->Add(m_date_format_choice, g_flags);
    m_date_format_choice->SetToolTip(_("Specify the date format for display"));

    m_sample_date_text = new wxStaticText(this, wxID_STATIC,
        "redefined elsewhere");
    dateFormatStaticBoxSizer->Add(new wxStaticText(this, wxID_STATIC,
        _("New date format sample:")), wxSizerFlags(g_flags).Border(wxLEFT, 15));
    dateFormatStaticBoxSizer->Add(m_sample_date_text, wxSizerFlags(g_flags).Border(wxLEFT, 5));
    m_sample_date_text->SetLabelText(wxDateTime::Now().Format(m_date_format));

    // Financial Year Settings
    wxStaticBox* financialYearStaticBox = new wxStaticBox(this, wxID_ANY, _("Financial Year"));
    SetBoldFont(financialYearStaticBox);
    wxStaticBoxSizer* financialYearStaticBoxSizer = new wxStaticBoxSizer(financialYearStaticBox, wxVERTICAL);
    wxFlexGridSizer* financialYearStaticBoxSizerGrid = new wxFlexGridSizer(0, 2, 0, 0);
    generalPanelSizer->Add(financialYearStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));
    financialYearStaticBoxSizer->Add(financialYearStaticBoxSizerGrid);

    financialYearStaticBoxSizerGrid->Add(new wxStaticText(this, wxID_STATIC, _("Start Day")), g_flags);
    int day = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_DAY", 1);

    wxSpinCtrl *textFPSDay = new wxSpinCtrl(this, ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_DAY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 31, day);
    textFPSDay->SetValue(day);
    textFPSDay->SetToolTip(_("Specify Day for start of financial year"));

    financialYearStaticBoxSizerGrid->Add(textFPSDay, g_flags);

    financialYearStaticBoxSizerGrid->Add(new wxStaticText(this, wxID_STATIC, _("Start Month")), g_flags);

    wxArrayString financialMonthsSelection;
    for (wxDateTime::Month m = wxDateTime::Jan; m <= wxDateTime::Dec; m = wxDateTime::Month(m + 1))
        financialMonthsSelection.Add(wxGetTranslation(wxDateTime::GetMonthName(m, wxDateTime::Name_Abbr)));

    m_month_selection = new wxChoice(this, ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_MONTH
        , wxDefaultPosition, wxSize(100, -1), financialMonthsSelection);
    financialYearStaticBoxSizerGrid->Add(m_month_selection, g_flags);

    int monthItem = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_MONTH", 7);
    m_month_selection->SetSelection(monthItem - 1);
    m_month_selection->SetToolTip(_("Specify month for start of financial year"));

    //a bit more space visual appearance
    generalPanelSizer->AddSpacer(15);

    m_use_org_date_copy_paste = new wxCheckBox(this, wxID_STATIC, _("Use Original Date when Pasting Transactions"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_use_org_date_copy_paste->SetValue(GetIniDatabaseCheckboxValue(INIDB_USE_ORG_DATE_COPYPASTE, false));
    m_use_org_date_copy_paste->SetToolTip(_("Select whether to use the original transaction date or current date when copying/pasting transactions"));
    generalPanelSizer->Add(m_use_org_date_copy_paste, g_flags);

    m_use_sound = new wxCheckBox(this, wxID_STATIC, _("Use Transaction Sound"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_use_sound->SetValue(GetIniDatabaseCheckboxValue(INIDB_USE_TRANSACTION_SOUND, true));
    m_use_sound->SetToolTip(_("Select whether to use sounds when entering transactions"));
    generalPanelSizer->Add(m_use_sound, g_flags);
}

void mmOptionGeneralSettings::OnCurrency(wxCommandEvent& /*event*/)
{
    int currencyID = Model_Infotable::instance().GetBaseCurrencyId();

    if (mmMainCurrencyDialog::Execute(this, currencyID) && currencyID != -1)
    {
        Model_Currency::Data* currency = Model_Currency::instance().get(currencyID);
        wxButton* bn = (wxButton*) FindWindow(ID_DIALOG_OPTIONS_BUTTON_CURRENCY);
        bn->SetLabelText(currency->CURRENCYNAME);
        m_currency_id = currencyID;

        wxMessageDialog msgDlg(this, _("Remember to update currency rate"), _("Important note"));
        msgDlg.ShowModal();
    }
}

void mmOptionGeneralSettings::OnDateFormatChanged(wxCommandEvent& /*event*/)
{
    wxStringClientData* data = (wxStringClientData*) (m_date_format_choice->GetClientObject(m_date_format_choice->GetSelection()));
    if (data)
    {
        m_date_format = data->GetData();
        mmOptions::instance().dateFormat_ = m_date_format;
        m_sample_date_text->SetLabelText(wxDateTime::Now().Format(m_date_format));
    }
    else
    {
        return;
    }
}

void mmOptionGeneralSettings::OnLanguageChanged(wxCommandEvent& /*event*/)
{
    wxString lang = mmSelectLanguage(this->m_app, this, true, false);
    if (lang.empty()) return;

    wxButton *btn = (wxButton*) FindWindow(ID_DIALOG_OPTIONS_BUTTON_LANGUAGE);
    wxASSERT(btn);
    btn->SetLabelText(lang.Left(1).Upper() + lang.SubString(1, lang.Len()));
}

void mmOptionGeneralSettings::SaveFinancialYearStart()
{
    //Save Financial Year Start Day
    wxSpinCtrl* fysDay = (wxSpinCtrl*) FindWindow(ID_DIALOG_OPTIONS_FINANCIAL_YEAR_START_DAY);
    wxString fysDayVal = wxString::Format("%d", fysDay->GetValue());
    mmOptions::instance().financialYearStartDayString_ = fysDayVal;
    Model_Infotable::instance().Set("FINANCIAL_YEAR_START_DAY", fysDayVal);

    //Save Financial Year Start Month
    wxString fysMonthVal = wxString() << m_month_selection->GetSelection() + 1;
    mmOptions::instance().financialYearStartMonthString_ = fysMonthVal;
    Model_Infotable::instance().Set("FINANCIAL_YEAR_START_MONTH", fysMonthVal);
}

void mmOptionGeneralSettings::SaveSettings()
{
    wxTextCtrl* stun = (wxTextCtrl*) FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_USERNAME);
    mmOptions::instance().userNameString_ = stun->GetValue();
    Model_Infotable::instance().Set("USERNAME", mmOptions::instance().userNameString_);

    wxButton *languageButton = (wxButton*) FindWindow(ID_DIALOG_OPTIONS_BUTTON_LANGUAGE);
    Model_Setting::instance().Set(LANGUAGE_PARAMETER, languageButton->GetLabel().Lower());
    mmSelectLanguage(this->m_app, this, false);

    Model_Infotable::instance().SetBaseCurrencyID(m_currency_id);
    Model_Infotable::instance().Set("DATEFORMAT", m_date_format);
    SaveFinancialYearStart();

    Model_Setting::instance().Set(INIDB_USE_ORG_DATE_COPYPASTE, m_use_org_date_copy_paste->GetValue());
    Model_Setting::instance().Set(INIDB_USE_TRANSACTION_SOUND, m_use_sound->GetValue());
}
