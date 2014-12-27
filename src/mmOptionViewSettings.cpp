#include "mmOptionViewSettings.h"
#include "util.h"
#include "constants.h"
#include <wx/colordlg.h>
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"

/*******************************************************/
wxBEGIN_EVENT_TABLE(mmOptionViewSettings, wxPanel)
wxEND_EVENT_TABLE()
/*******************************************************/

mmOptionViewSettings::mmOptionViewSettings()
{

}

mmOptionViewSettings::mmOptionViewSettings(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
{
    wxPanel::Create(parent, id, pos, size, style, name);
    Create(parent, id, pos, size, style, name);
}

mmOptionViewSettings::~mmOptionViewSettings()
{

}

void mmOptionViewSettings::Create(wxWindow *parent
    , wxWindowID id
    , const wxPoint& pos
    , const wxSize& size
    , long style
    , const wxString &name)
{
    wxBoxSizer* viewsPanelSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(viewsPanelSizer);

    // Account View Options
    wxStaticBox* accountStaticBox = new wxStaticBox(this, wxID_STATIC, _("View Options"));

    wxFont staticBoxFontSetting = accountStaticBox->GetFont();
    staticBoxFontSetting.SetWeight(wxFONTWEIGHT_BOLD);
    accountStaticBox->SetFont(staticBoxFontSetting);

    wxStaticBoxSizer* accountStaticBoxSizer = new wxStaticBoxSizer(accountStaticBox, wxVERTICAL);
    viewsPanelSizer->Add(accountStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));
    wxFlexGridSizer* view_sizer1 = new wxFlexGridSizer(0, 2, 0, 5);
    accountStaticBoxSizer->Add(view_sizer1);

    view_sizer1->Add(new wxStaticText(this, wxID_STATIC, _("Accounts Visible")), g_flags);

    int row_id_ = 0;
    wxArrayString itemChoiceViewAccountTranslatedStrings = viewAccountStrings(true, wxEmptyString, row_id_);

    m_choice_visible = new wxChoice(this, ID_DIALOG_OPTIONS_VIEW_ACCOUNTS
        , wxDefaultPosition, wxDefaultSize, itemChoiceViewAccountTranslatedStrings);
    view_sizer1->Add(m_choice_visible, g_flags);

    wxString vAccts = Model_Setting::instance().GetStringSetting("VIEWACCOUNTS", VIEW_ACCOUNTS_ALL_STR);
    row_id_ = 0;
    wxArrayString itemChoiceViewAccountStrings = viewAccountStrings(false, vAccts, row_id_);
    m_choice_visible->SetSelection(row_id_);

    m_choice_visible->SetToolTip(_("Specify which accounts are visible"));

    view_sizer1->Add(new wxStaticText(this, wxID_STATIC, _("Transactions Visible")), g_flags);

    wxArrayString view_strings;
    view_strings.Add(VIEW_TRANS_ALL_STR);
    view_strings.Add(VIEW_TRANS_TODAY_STR);
    view_strings.Add(VIEW_TRANS_CURRENT_MONTH_STR);
    view_strings.Add(VIEW_TRANS_LAST_30_DAYS_STR);
    view_strings.Add(VIEW_TRANS_LAST_90_DAYS_STR);
    view_strings.Add(VIEW_TRANS_LAST_MONTH_STR);
    view_strings.Add(VIEW_TRANS_LAST_3MONTHS_STR);
    view_strings.Add(VIEW_TRANS_CURRENT_YEAR_STR);
    view_strings.Add(VIEW_TRANS_LAST_12MONTHS_STR);

    m_choice_trans_visible = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    for (const auto &entry : view_strings)
        m_choice_trans_visible->Append(wxGetTranslation(entry),
        new wxStringClientData(entry));

    view_sizer1->Add(m_choice_trans_visible, g_flags);

    wxString vTrans = Model_Setting::instance().GetStringSetting("VIEWTRANSACTIONS", VIEW_TRANS_ALL_STR);
    m_choice_trans_visible->SetStringSelection(wxGetTranslation(vTrans));
    m_choice_trans_visible->SetToolTip(_("Specify which transactions are visible by default"));

    view_sizer1->Add(new wxStaticText(this, wxID_STATIC, _("Report Font Size")), g_flags);

    const wxString itemChoiceFontSize [] = {
        wxTRANSLATE("XSmall"),
        wxTRANSLATE("Small"),
        wxTRANSLATE("Normal"),
        wxTRANSLATE("Large"),
        wxTRANSLATE("XLarge"),
        wxTRANSLATE("XXLarge"),
        wxTRANSLATE("Huge") };

    m_choice_font_size = new wxChoice(this, wxID_ANY);

    for (const auto &entry : itemChoiceFontSize)
        m_choice_font_size->Append(wxGetTranslation(entry));

    int vFontSize = -1 + Model_Setting::instance().GetIntSetting("HTMLFONTSIZE", 3);
    m_choice_font_size->SetSelection(vFontSize);

    m_choice_font_size->SetToolTip(_("Specify which font size is used on the report tables"));
    view_sizer1->Add(m_choice_font_size, g_flags);

    // Budget options
    m_budget_financial_years = new wxCheckBox(this, wxID_STATIC, _("View Budgets as Financial Years")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_budget_financial_years->SetValue(GetIniDatabaseCheckboxValue(INIDB_BUDGET_FINANCIAL_YEARS, false));
    viewsPanelSizer->Add(m_budget_financial_years, g_flags);

    m_budget_include_transfers = new wxCheckBox(this, wxID_STATIC
        , _("View Budgets with 'transfer' transactions")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_budget_include_transfers->SetValue(GetIniDatabaseCheckboxValue(INIDB_BUDGET_INCLUDE_TRANSFERS, false));
    viewsPanelSizer->Add(m_budget_include_transfers, g_flags);

    m_budget_setup_without_summary = new wxCheckBox(this, wxID_STATIC
        , _("View Budgets Setup Without Budget Summaries")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_budget_setup_without_summary->SetValue(GetIniDatabaseCheckboxValue(INIDB_BUDGET_SETUP_WITHOUT_SUMMARY, false));
    viewsPanelSizer->Add(m_budget_setup_without_summary, g_flags);

    m_budget_summary_without_category = new wxCheckBox(this, wxID_STATIC
        , _("View Budget Summary Report without Categories")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_budget_summary_without_category->SetValue(GetIniDatabaseCheckboxValue(INIDB_BUDGET_SUMMARY_WITHOUT_CATEG, true));
    viewsPanelSizer->Add(m_budget_summary_without_category, g_flags);

    m_ignore_future_transactions = new wxCheckBox(this, wxID_STATIC
        , _("View Reports without Future Transactions")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_ignore_future_transactions->SetValue(GetIniDatabaseCheckboxValue(INIDB_IGNORE_FUTURE_TRANSACTIONS, false));
    viewsPanelSizer->Add(m_ignore_future_transactions, g_flags);

    // Colours settings
    wxStaticBox* userColourSettingStBox = new wxStaticBox(this, wxID_ANY, _("User Colors"));
    userColourSettingStBox->SetFont(staticBoxFontSetting);
    wxStaticBoxSizer* userColourSettingStBoxSizer = new wxStaticBoxSizer(userColourSettingStBox, wxHORIZONTAL);
    viewsPanelSizer->Add(userColourSettingStBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    int size_x = 30;
    m_UDFCB1 = new wxButton(this, wxID_HIGHEST + 11
        , _("1"), wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB1->SetBackgroundColour(mmColors::userDefColor1);
    userColourSettingStBoxSizer->Add(m_UDFCB1, g_flags);

    m_UDFCB2 = new wxButton(this, wxID_HIGHEST + 22
        , _("2"), wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB2->SetBackgroundColour(mmColors::userDefColor2);
    userColourSettingStBoxSizer->Add(m_UDFCB2, g_flags);

    m_UDFCB3 = new wxButton(this, wxID_HIGHEST + 33
        , _("3"), wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB3->SetBackgroundColour(mmColors::userDefColor3);
    userColourSettingStBoxSizer->Add(m_UDFCB3, g_flags);

    m_UDFCB4 = new wxButton(this, wxID_HIGHEST + 44
        , _("4"), wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB4->SetBackgroundColour(mmColors::userDefColor4);
    userColourSettingStBoxSizer->Add(m_UDFCB4, g_flags);

    m_UDFCB5 = new wxButton(this, wxID_HIGHEST + 55
        , _("5"), wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB5->SetBackgroundColour(mmColors::userDefColor5);
    userColourSettingStBoxSizer->Add(m_UDFCB5, g_flags);

    m_UDFCB6 = new wxButton(this, wxID_HIGHEST + 66
        , _("6"), wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB6->SetBackgroundColour(mmColors::userDefColor6);
    userColourSettingStBoxSizer->Add(m_UDFCB6, g_flags);

    m_UDFCB7 = new wxButton(this, wxID_HIGHEST + 77
        , _("7"), wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB7->SetBackgroundColour(mmColors::userDefColor7);
    userColourSettingStBoxSizer->Add(m_UDFCB7, g_flags);

    this->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmOptionViewSettings::OnNavTreeColorChanged), nullptr, this);
}

wxArrayString mmOptionViewSettings::viewAccountStrings(bool translated, const wxString& input_string, int& row_id)
{
    wxArrayString itemChoiceViewAccountStrings;

    if (translated)
    {
        itemChoiceViewAccountStrings.Add(_("All"));
        itemChoiceViewAccountStrings.Add(_("Open"));
        itemChoiceViewAccountStrings.Add(_("Favorites"));
    }
    else
    {
        itemChoiceViewAccountStrings.Add(VIEW_ACCOUNTS_ALL_STR);
        itemChoiceViewAccountStrings.Add(VIEW_ACCOUNTS_OPEN_STR);
        itemChoiceViewAccountStrings.Add(VIEW_ACCOUNTS_FAVORITES_STR);
    }
    if (!input_string.IsEmpty())
    {
        for (size_t i = 0; i < itemChoiceViewAccountStrings.Count(); i++)
        {
            if (input_string == itemChoiceViewAccountStrings[i])
            {
                row_id = i;
                break;
            }
        }
    }

    return itemChoiceViewAccountStrings;
}

bool mmOptionViewSettings::GetIniDatabaseCheckboxValue(wxString dbField, bool defaultState)
{
    bool result = Model_Setting::instance().GetBoolSetting(dbField, defaultState);
    return result;
}

void mmOptionViewSettings::OnNavTreeColorChanged(wxCommandEvent& event)
{
    int buttonId = event.GetId();
    wxButton* button = (wxButton*) FindWindow(buttonId);
    if (!button) return;

    wxColour colour = button->GetBackgroundColour();
    wxColourData data;
    data.SetChooseFull(true);
    data.SetColour(colour);

    wxColourDialog dialog(this, &data);
    if (dialog.ShowModal() == wxID_OK)
    {
        colour = dialog.GetColourData().GetColour();
        button->SetBackgroundColour(colour);
    }
}

void mmOptionViewSettings::SaveSettings()
{
    int selection = m_choice_visible->GetSelection();
    int row_id_ = 0;
    wxArrayString viewAcct = viewAccountStrings(false, wxEmptyString, row_id_);
    Model_Setting::instance().Set("VIEWACCOUNTS", viewAcct[selection]);

    wxString visible = VIEW_TRANS_ALL_STR;
    wxStringClientData* visible_obj = (wxStringClientData *) m_choice_trans_visible->GetClientObject(m_choice_trans_visible->GetSelection());
    if (visible_obj)
    {
        visible = visible_obj->GetData();
    }
    Model_Setting::instance().Set("VIEWTRANSACTIONS", visible);

    int size = m_choice_font_size->GetCurrentSelection() + 1;
    mmIniOptions::instance().html_font_size_ = size;
    Model_Setting::instance().Set("HTMLFONTSIZE", size);

    mmIniOptions::instance().budgetFinancialYears_ = m_budget_financial_years->GetValue();
    Model_Setting::instance().Set(INIDB_BUDGET_FINANCIAL_YEARS, mmIniOptions::instance().budgetFinancialYears_);

    mmIniOptions::instance().budgetIncludeTransfers_ = m_budget_include_transfers->GetValue();
    Model_Setting::instance().Set(INIDB_BUDGET_INCLUDE_TRANSFERS, mmIniOptions::instance().budgetIncludeTransfers_);

    mmIniOptions::instance().budgetSetupWithoutSummaries_ = m_budget_setup_without_summary->GetValue();
    Model_Setting::instance().Set(INIDB_BUDGET_SETUP_WITHOUT_SUMMARY, mmIniOptions::instance().budgetSetupWithoutSummaries_);

    mmIniOptions::instance().budgetReportWithSummaries_ = m_budget_summary_without_category->GetValue();
    Model_Setting::instance().Set(INIDB_BUDGET_SUMMARY_WITHOUT_CATEG, mmIniOptions::instance().budgetReportWithSummaries_);

    mmIniOptions::instance().ignoreFutureTransactions_ = m_ignore_future_transactions->GetValue();
    Model_Setting::instance().Set(INIDB_IGNORE_FUTURE_TRANSACTIONS, mmIniOptions::instance().ignoreFutureTransactions_);

    mmColors::userDefColor1 = m_UDFCB1->GetBackgroundColour();
    mmColors::userDefColor2 = m_UDFCB2->GetBackgroundColour();
    mmColors::userDefColor3 = m_UDFCB3->GetBackgroundColour();
    mmColors::userDefColor4 = m_UDFCB4->GetBackgroundColour();
    mmColors::userDefColor5 = m_UDFCB5->GetBackgroundColour();
    mmColors::userDefColor6 = m_UDFCB6->GetBackgroundColour();
    mmColors::userDefColor7 = m_UDFCB7->GetBackgroundColour();

    Model_Infotable::instance().Set("USER_COLOR1", mmColors::userDefColor1);
    Model_Infotable::instance().Set("USER_COLOR2", mmColors::userDefColor2);
    Model_Infotable::instance().Set("USER_COLOR3", mmColors::userDefColor3);
    Model_Infotable::instance().Set("USER_COLOR4", mmColors::userDefColor4);
    Model_Infotable::instance().Set("USER_COLOR5", mmColors::userDefColor5);
    Model_Infotable::instance().Set("USER_COLOR6", mmColors::userDefColor6);
    Model_Infotable::instance().Set("USER_COLOR7", mmColors::userDefColor7);
}
