/*******************************************************
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

#include "optionsettingstransaction.h"
#include "constants.h"
#include "option.h"
#include "util.h"

#include <wx/colordlg.h>

/*******************************************************/

OptionSettingsTransaction::OptionSettingsTransaction()
{
}

OptionSettingsTransaction::OptionSettingsTransaction(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name) : OptionSettingsTransaction()
{
    wxPanel::Create(parent, id, pos, size, style, name);
    Create();
}

OptionSettingsTransaction::~OptionSettingsTransaction()
{
}

void OptionSettingsTransaction::Create()
{
    wxBoxSizer* homePanelSizer0 = new wxBoxSizer(wxVERTICAL);
    SetSizer(homePanelSizer0);

    wxScrolledWindow* home_panel = new wxScrolledWindow(this, wxID_ANY);
    wxBoxSizer* homePanelSizer = new wxBoxSizer(wxVERTICAL);
    home_panel->SetSizer(homePanelSizer);
    homePanelSizer0->Add(home_panel, wxSizerFlags(g_flagsExpand).Proportion(0));

    // New transaction dialog settings
    wxStaticBox* transSettingsStaticBox = new wxStaticBox(home_panel, wxID_STATIC, _t("New Transaction"));
    SetBoldFont(transSettingsStaticBox);

    wxStaticBoxSizer* transSettingsStaticBoxSizer = new wxStaticBoxSizer(transSettingsStaticBox, wxVERTICAL);
    homePanelSizer->Add(transSettingsStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxArrayString default_values;
    default_values.Add(_t("None"));
    default_values.Add(_t("Last Used"));

    wxChoice* defaultCategoryTransferChoice = new wxChoice(home_panel
        , ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY_TRANSFER
        , wxDefaultPosition, wxDefaultSize, default_values);
    defaultCategoryTransferChoice->SetSelection(Option::instance().getTransCategoryTransferNone());

    wxChoice* defaultDateChoice = new wxChoice(home_panel
        , ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_DATE
        , wxDefaultPosition, wxDefaultSize, default_values);
    defaultDateChoice->SetSelection(Option::instance().getTransDateDefault());

    default_values.Add(_t("Unused"));
    wxChoice* defaultPayeeChoice = new wxChoice(home_panel
        , ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_PAYEE
        , wxDefaultPosition, wxDefaultSize, default_values);
    defaultPayeeChoice->SetSelection(Option::instance().getTransPayeeNone());

    default_values[1] = (_t("Last used for payee"));
    default_values.Add(_t("Use default for payee"));
    wxChoice* defaultCategoryNonTransferChoice = new wxChoice(home_panel
        , ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY_NONTRANSFER
        , wxDefaultPosition, wxDefaultSize, default_values);
    defaultCategoryNonTransferChoice->SetSelection(Option::instance().getTransCategoryNone());

    wxChoice* default_status = new wxChoice(home_panel
        , ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_STATUS);
    for (int i = 0; i < Model_Checking::STATUS_ID_size; ++i) {
        wxString status = Model_Checking::status_name(i);
        default_status->Append(wxGetTranslation(status), new wxStringClientData(status));
    }
    default_status->SetSelection(Option::instance().getTransStatusReconciled());

    wxChoice* bulk_enter = new wxChoice(home_panel, ID_DIALOG_OPTIONS_BULK_ENTER, wxDefaultPosition, wxDefaultSize, {_("Yes"), _("No")});
    bulk_enter->SetSelection(Option::instance().getBulkTransactions() ? 0 : 1);

    wxFlexGridSizer* newTransflexGridSizer = new wxFlexGridSizer(0, 2, 0, 0);
    newTransflexGridSizer->AddGrowableCol(1, 0);
    transSettingsStaticBoxSizer->Add(newTransflexGridSizer);
    newTransflexGridSizer->Add(new wxStaticText(home_panel, wxID_STATIC, _t("Default Date")), g_flagsH);
    newTransflexGridSizer->Add(defaultDateChoice, g_flagsExpand);
    newTransflexGridSizer->Add(new wxStaticText(home_panel, wxID_STATIC, _t("Default Payee")), g_flagsH);
    newTransflexGridSizer->Add(defaultPayeeChoice, g_flagsExpand);
    newTransflexGridSizer->Add(new wxStaticText(home_panel, wxID_STATIC, _t("Default Deposit/Withdrawal Category")), g_flagsH);
    newTransflexGridSizer->Add(defaultCategoryNonTransferChoice, g_flagsExpand);
    newTransflexGridSizer->Add(new wxStaticText(home_panel, wxID_STATIC, _t("Default Transfer Category")), g_flagsH);
    newTransflexGridSizer->Add(defaultCategoryTransferChoice, g_flagsExpand);
    newTransflexGridSizer->Add(new wxStaticText(home_panel, wxID_STATIC, _t("Default Status")), g_flagsH);
    newTransflexGridSizer->Add(default_status, g_flagsExpand);
    newTransflexGridSizer->Add(new wxStaticText(home_panel, wxID_STATIC, _t("Bulk Transactions")), g_flagsH);
    newTransflexGridSizer->Add(bulk_enter, g_flagsExpand);


    // transaction
    wxStaticBox* transactioCopyStaticBox = new wxStaticBox(home_panel, wxID_ANY, _t("Transaction"));
    SetBoldFont(transactioCopyStaticBox);
    wxStaticBoxSizer* transactioCopyStaticBoxSizer = new wxStaticBoxSizer(transactioCopyStaticBox, wxVERTICAL);
    homePanelSizer->Add(transactioCopyStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_use_org_date_copy_paste = new wxCheckBox(home_panel, wxID_STATIC, _t("Use Original Date when Pasting Transactions"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_use_org_date_copy_paste->SetValue(GetIniDatabaseCheckboxValue(INIDB_USE_ORG_DATE_COPYPASTE, false));
    mmToolTip(m_use_org_date_copy_paste, _t("Select whether to use the original transaction date or current date when copying/pasting transactions"));
    transactioCopyStaticBoxSizer->Add(m_use_org_date_copy_paste, g_flagsV);

    m_use_org_date_duplicate = new wxCheckBox(home_panel, wxID_STATIC, _t("Use Original Date when Duplicating Transactions"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_use_org_date_duplicate->SetValue(GetIniDatabaseCheckboxValue(INIDB_USE_ORG_DATE_DUPLICATE, false));
    mmToolTip(m_use_org_date_duplicate, _t("Select whether to use the original transaction date or current date when duplicating transactions"));
    transactioCopyStaticBoxSizer->Add(m_use_org_date_duplicate, g_flagsV);

    m_use_org_state_duplicate_paste = new wxCheckBox(home_panel, wxID_STATIC, _t("Use Original State when Duplicating or Pasting Transactions"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_use_org_state_duplicate_paste->SetValue(GetIniDatabaseCheckboxValue(INIDB_USE_ORG_STATE_DUPLICATE_PASTE, true));
    mmToolTip(m_use_org_state_duplicate_paste, _t("Select whether to use the original state or default state when duplicating or copy and paste transactions"));
    transactioCopyStaticBoxSizer->Add(m_use_org_state_duplicate_paste, g_flagsV);

    wxArrayString sounds;
    sounds.Add(_t("None"));
    sounds.Add("drop.wav");
    sounds.Add("cash.wav");

    wxBoxSizer* soundBaseSizer = new wxBoxSizer(wxHORIZONTAL);
    transactioCopyStaticBoxSizer->Add(soundBaseSizer, wxSizerFlags(g_flagsV).Border(wxLEFT, 0));
    soundBaseSizer->Add(new wxStaticText(home_panel, wxID_STATIC, _t("Transaction Sound")), g_flagsH);
    m_use_sound = new wxChoice(home_panel, wxID_STATIC
        , wxDefaultPosition, wxSize(100, -1)
        , sounds);
    m_use_sound->SetSelection(Model_Setting::instance().getInt(INIDB_USE_TRANSACTION_SOUND, 0));
    mmToolTip(m_use_sound, _t("Select whether to use sounds when entering transactions"));
    soundBaseSizer->Add(m_use_sound, g_flagsV);

    // Transaction/Budget options
    wxStaticBox* trxBox = new wxStaticBox(home_panel, wxID_STATIC, _t("Transaction/Budget"));
    SetBoldFont(trxBox);
    wxStaticBoxSizer* trxSizer = new wxStaticBoxSizer(trxBox, wxVERTICAL);
    homePanelSizer->Add(trxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_budget_financial_years = new wxCheckBox(
        home_panel, wxID_STATIC,
        _t("View Budgets as Financial Years"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    m_budget_financial_years->SetValue(Option::instance().getBudgetFinancialYears());
    trxSizer->Add(m_budget_financial_years, g_flagsV);

    m_budget_include_transfers = new wxCheckBox(
        home_panel, wxID_STATIC,
        _t("View Budgets with 'transfer' transactions"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    m_budget_include_transfers->SetValue(Option::instance().getBudgetIncludeTransfers());
    trxSizer->Add(m_budget_include_transfers, g_flagsV);

    m_budget_summary_without_category = new wxCheckBox(
        home_panel, wxID_STATIC,
        _t("View Budget Category Report with Summaries"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    m_budget_summary_without_category->SetValue(Option::instance().getBudgetSummaryWithoutCategories());
    trxSizer->Add(m_budget_summary_without_category, g_flagsV);

    // Budget Yearly/Monthly relationship if both exist
    m_budget_override = new wxCheckBox(
        home_panel, wxID_STATIC,
        _t("Override yearly budget with monthly budget"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    mmToolTip(m_budget_override, _t("If monthly budget exists then use this to override the yearly budget; otherwise combine them"));
    m_budget_override->SetValue(Option::instance().getBudgetOverride());
    trxSizer->Add(m_budget_override, g_flagsV);

    // Option to deduct monthly budget from yearly budget for reporting
    m_budget_deduct_monthly = new wxCheckBox(
        home_panel, wxID_STATIC,
        _t("Subtract monthly budgets from yearly budget in reporting"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    mmToolTip(m_budget_deduct_monthly, _t("Yearly budget will be reduced by the amount budgeted monthly.\nTotal estimate for the year will be reported as either the yearly budget OR the sum of the monthly budgets, whichever is greater."));
    m_budget_deduct_monthly->SetValue(Option::instance().getBudgetDeductMonthly());
    trxSizer->Add(m_budget_deduct_monthly, g_flagsV);

    wxFlexGridSizer* trxChoiceSizer = new wxFlexGridSizer(0, 2, 0, 5);
    trxSizer->Add(trxChoiceSizer);

    trxChoiceSizer->Add(new wxStaticText(home_panel, wxID_STATIC, _t("Budget Offset (days)")), g_flagsH);
    m_budget_days_offset = new wxSpinCtrl(
        home_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxSP_ARROW_KEYS, -30, +30
    );
    mmToolTip(m_budget_days_offset, _t("Adjusts the first day of month (normally 1st) for budget calculations"));
    m_budget_days_offset->SetValue(Option::instance().getBudgetDaysOffset());
    trxChoiceSizer->Add(m_budget_days_offset, g_flagsH);

    trxChoiceSizer->Add(new wxStaticText(home_panel, wxID_STATIC, _t("First Day of Month")), g_flagsH);
    m_reporting_firstday = new wxSpinCtrl(
        home_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxSP_ARROW_KEYS, 1, 28
    );
    mmToolTip(m_reporting_firstday, _t("Adjusts the first day of month for reporting"));
    m_reporting_firstday->SetValue(Option::instance().getReportingFirstDay());
    trxChoiceSizer->Add(m_reporting_firstday, g_flagsH);

    trxChoiceSizer->Add(new wxStaticText(home_panel, wxID_STATIC, _t("First Weekday")), g_flagsH);
    m_reporting_first_weekday = new wxChoice(home_panel, wxID_ANY);
    m_reporting_first_weekday->Append(wxGetTranslation(g_days_of_week[0]));
    m_reporting_first_weekday->Append(wxGetTranslation(g_days_of_week[1]));
    m_reporting_first_weekday->SetSelection(Option::instance().getReportingFirstWeekday());
    mmToolTip(m_reporting_first_weekday, _t("Adjusts the first day of week for filtering and reporting"));
    trxChoiceSizer->Add(m_reporting_first_weekday, g_flagsH);

    m_ignore_future_transactions = new wxCheckBox(
        home_panel, wxID_STATIC,
        _t("Ignore Future Transactions"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    m_ignore_future_transactions->SetValue(Option::instance().getIgnoreFutureTransactions());
    trxSizer->Add(m_ignore_future_transactions, g_flagsV);

    m_use_trans_date_time = new wxCheckBox(home_panel, wxID_ANY, _t("Use 'Time' in transaction recording/reporting"));
    m_use_trans_date_time->SetValue(Option::instance().UseTransDateTime());
    trxSizer->Add(m_use_trans_date_time, g_flagsV);

    // Colors settings
    wxStaticBox* colorsBox = new wxStaticBox(home_panel, wxID_ANY, _t("Transaction Colors"));
    SetBoldFont(colorsBox);
    wxStaticBoxSizer* colorsSizer = new wxStaticBoxSizer(colorsBox, wxHORIZONTAL);
    homePanelSizer->Add(colorsSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    int size_x = 30;
    m_UDFCB1 = new wxButton(home_panel, wxID_HIGHEST + 11, "1", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB1->SetBackgroundColour(mmColors::userDefColor1);
    colorsSizer->Add(m_UDFCB1, g_flagsH);

    m_UDFCB2 = new wxButton(home_panel, wxID_HIGHEST + 22, "2", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB2->SetBackgroundColour(mmColors::userDefColor2);
    colorsSizer->Add(m_UDFCB2, g_flagsH);

    m_UDFCB3 = new wxButton(home_panel, wxID_HIGHEST + 33, "3", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB3->SetBackgroundColour(mmColors::userDefColor3);
    colorsSizer->Add(m_UDFCB3, g_flagsH);

    m_UDFCB4 = new wxButton(home_panel, wxID_HIGHEST + 44, "4", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB4->SetBackgroundColour(mmColors::userDefColor4);
    colorsSizer->Add(m_UDFCB4, g_flagsH);

    m_UDFCB5 = new wxButton(home_panel, wxID_HIGHEST + 55, "5", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB5->SetBackgroundColour(mmColors::userDefColor5);
    colorsSizer->Add(m_UDFCB5, g_flagsH);

    m_UDFCB6 = new wxButton(home_panel, wxID_HIGHEST + 66, "6", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB6->SetBackgroundColour(mmColors::userDefColor6);
    colorsSizer->Add(m_UDFCB6, g_flagsH);

    m_UDFCB7 = new wxButton(home_panel, wxID_HIGHEST + 77, "7", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB7->SetBackgroundColour(mmColors::userDefColor7);
    colorsSizer->Add(m_UDFCB7, g_flagsH);

    wxButton* reset = new wxButton(home_panel, wxID_REDO, _t("Default"), wxDefaultPosition, wxDefaultSize, 0);
    m_UDFCB7->SetBackgroundColour(mmColors::userDefColor7);
    colorsSizer->Add(reset, g_flagsH);

    // -------------
    Fit();
    home_panel->SetMinSize(home_panel->GetBestVirtualSize());
    home_panel->SetScrollRate(6, 6);

    this->Connect(
        wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(OptionSettingsTransaction::OnNavTreeColorChanged),
        nullptr, this
    );
}

bool OptionSettingsTransaction::SaveSettings()
{
    Model_Setting::instance().setBool(INIDB_USE_ORG_DATE_COPYPASTE, m_use_org_date_copy_paste->GetValue());
    Model_Setting::instance().setBool(INIDB_USE_ORG_DATE_DUPLICATE, m_use_org_date_duplicate->GetValue());
    Model_Setting::instance().setBool(INIDB_USE_ORG_STATE_DUPLICATE_PASTE, m_use_org_state_duplicate_paste->GetValue());
    Model_Setting::instance().setInt(INIDB_USE_TRANSACTION_SOUND, m_use_sound->GetSelection());

    Option::instance().setBudgetFinancialYears(m_budget_financial_years->GetValue());
    Option::instance().setBudgetIncludeTransfers(m_budget_include_transfers->GetValue());
    Option::instance().setBudgetSummaryWithoutCategories(m_budget_summary_without_category->GetValue());
    Option::instance().setBudgetOverride(m_budget_override->GetValue());
    Option::instance().setBudgetDeductMonthly(m_budget_deduct_monthly->GetValue());
    Option::instance().setBudgetDaysOffset(m_budget_days_offset->GetValue());
    Option::instance().setReportingFirstDay(m_reporting_firstday->GetValue());
    Option::instance().setReportingFirstWeekday(
        static_cast<wxDateTime::WeekDay>(m_reporting_first_weekday->GetSelection())
    );
    Option::instance().setIgnoreFutureTransactions(m_ignore_future_transactions->GetValue());
    Option::instance().UseTransDateTime(m_use_trans_date_time->GetValue());

    mmColors::userDefColor1 = m_UDFCB1->GetBackgroundColour();
    mmColors::userDefColor2 = m_UDFCB2->GetBackgroundColour();
    mmColors::userDefColor3 = m_UDFCB3->GetBackgroundColour();
    mmColors::userDefColor4 = m_UDFCB4->GetBackgroundColour();
    mmColors::userDefColor5 = m_UDFCB5->GetBackgroundColour();
    mmColors::userDefColor6 = m_UDFCB6->GetBackgroundColour();
    mmColors::userDefColor7 = m_UDFCB7->GetBackgroundColour();

    Model_Infotable::instance().setColour("USER_COLOR1", mmColors::userDefColor1);
    Model_Infotable::instance().setColour("USER_COLOR2", mmColors::userDefColor2);
    Model_Infotable::instance().setColour("USER_COLOR3", mmColors::userDefColor3);
    Model_Infotable::instance().setColour("USER_COLOR4", mmColors::userDefColor4);
    Model_Infotable::instance().setColour("USER_COLOR5", mmColors::userDefColor5);
    Model_Infotable::instance().setColour("USER_COLOR6", mmColors::userDefColor6);
    Model_Infotable::instance().setColour("USER_COLOR7", mmColors::userDefColor7);

    wxChoice* itemChoice = static_cast<wxChoice*>(FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_PAYEE));
    Option::instance().setTransPayeeNone(itemChoice->GetSelection());

    itemChoice = static_cast<wxChoice*>(FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY_NONTRANSFER));
    Option::instance().setTransCategoryNone(itemChoice->GetSelection());

    itemChoice = static_cast<wxChoice*>(FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY_TRANSFER));
    Option::instance().setTransCategoryTransferNone(itemChoice->GetSelection());

    itemChoice = static_cast<wxChoice*>(FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_STATUS));
    Option::instance().setTransStatusReconciled(itemChoice->GetSelection());

    itemChoice = static_cast<wxChoice*>(FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_DATE));
    Option::instance().setTransDateDefault(itemChoice->GetSelection());

    itemChoice = static_cast<wxChoice*>(FindWindow(ID_DIALOG_OPTIONS_BULK_ENTER));
    Option::instance().setBulkTransactions(itemChoice->GetSelection() == 0);

    return true;
}

void OptionSettingsTransaction::OnNavTreeColorChanged(wxCommandEvent& event)
{
    if (event.GetId() == wxID_REDO)
    {
        mmLoadColorsFromDatabase(true);
        m_UDFCB1->SetBackgroundColour(mmColors::userDefColor1);
        m_UDFCB2->SetBackgroundColour(mmColors::userDefColor2);
        m_UDFCB3->SetBackgroundColour(mmColors::userDefColor3);
        m_UDFCB4->SetBackgroundColour(mmColors::userDefColor4);
        m_UDFCB5->SetBackgroundColour(mmColors::userDefColor5);
        m_UDFCB6->SetBackgroundColour(mmColors::userDefColor6);
        m_UDFCB7->SetBackgroundColour(mmColors::userDefColor7);
        return;
    }


    wxButton* button = wxDynamicCast(FindWindow(event.GetId()), wxButton);
    if (button)
    {
        wxColour color = button->GetBackgroundColour();
        wxColourData data;
        data.SetChooseFull(true);
        data.SetColour(color);

        wxColourDialog dialog(this, &data);
        if (dialog.ShowModal() == wxID_OK)
        {
            color = dialog.GetColourData().GetColour();
            button->SetBackgroundColour(color);
        }
    }
}
