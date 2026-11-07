/*******************************************************
Copyright (C) 2025 Klaus Wich
Copyright (C) 2025 Mark Whalley (mark@ipx.co.uk)

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

#include "TrxPref.h"

#include "base/_defs.h"
#include <wx/colordlg.h>

#include "base/_constants.h"
#include "base/mmUserColor.h"
#include "util/_util.h"

#include "model/PrefModel.h"

TrxPref::TrxPref()
{
}

TrxPref::TrxPref(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name) : TrxPref()
{
    wxPanel::Create(parent, id, pos, size, style, name);
    Create();
}

TrxPref::~TrxPref()
{
}

void TrxPref::Create()
{
    wxBoxSizer* homePanelSizer0 = new wxBoxSizer(wxVERTICAL);
    SetSizer(homePanelSizer0);

    wxScrolledWindow* home_panel = new wxScrolledWindow(this, wxID_ANY);
    wxBoxSizer* homePanelSizer = new wxBoxSizer(wxVERTICAL);
    home_panel->SetSizer(homePanelSizer);
    homePanelSizer0->Add(home_panel, wxSizerFlags(g_flagsExpand).Proportion(0));

    // New transaction dialog settings
    wxStaticBox* transSettingsStaticBox = new wxStaticBox(home_panel, wxID_STATIC,
        _t("New Transaction")
    );

    wxStaticBoxSizer* transSettingsStaticBoxSizer = new wxStaticBoxSizer(transSettingsStaticBox, wxVERTICAL);
    homePanelSizer->Add(transSettingsStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxArrayString default_values;
    default_values.Add(_t("None"));
    default_values.Add(_t("Last Used"));

    wxChoice* defaultCategoryTransferChoice = new wxChoice(transSettingsStaticBox,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY_TRANSFER,
        wxDefaultPosition, wxDefaultSize,
        default_values
    );
    defaultCategoryTransferChoice->SetSelection(PrefModel::instance().getTransCategoryTransferNone());

    wxChoice* defaultDateChoice = new wxChoice(transSettingsStaticBox,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_DATE,
        wxDefaultPosition, wxDefaultSize,
        default_values
    );
    defaultDateChoice->SetSelection(PrefModel::instance().getTransDateDefault());

    default_values.Add(_t("Unused"));
    wxChoice* defaultPayeeChoice = new wxChoice(transSettingsStaticBox,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_PAYEE,
        wxDefaultPosition, wxDefaultSize,
        default_values);
    defaultPayeeChoice->SetSelection(PrefModel::instance().getTransPayeeNone());

    default_values[1] = (_t("Last used for payee"));
    default_values.Add(_t("Use default for payee"));
    wxChoice* defaultCategoryNonTransferChoice = new wxChoice(transSettingsStaticBox,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY_NONTRANSFER,
        wxDefaultPosition, wxDefaultSize,
        default_values
    );
    defaultCategoryNonTransferChoice->SetSelection(PrefModel::instance().getTransCategoryNone());

    wxChoice* default_status = new wxChoice(transSettingsStaticBox,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_STATUS
    );
    for (int status_id = 0; status_id < TrxStatus::size; ++status_id) {
        wxString status_name = TrxStatus(status_id).name();
        default_status->Append(
            wxGetTranslation(status_name),
            new wxStringClientData(status_name)
        );
    }
    default_status->SetSelection(PrefModel::instance().getTrxStatus().id());

    wxFlexGridSizer* newTransflexGridSizer = new wxFlexGridSizer(0, 2, 0, 0);
    newTransflexGridSizer->AddGrowableCol(1, 0);
    transSettingsStaticBoxSizer->Add(newTransflexGridSizer);
    newTransflexGridSizer->Add(new wxStaticText(transSettingsStaticBox, wxID_STATIC, _t("Default Date")), g_flagsH);
    newTransflexGridSizer->Add(defaultDateChoice, g_flagsExpand);
    newTransflexGridSizer->Add(new wxStaticText(transSettingsStaticBox, wxID_STATIC, _t("Default Payee")), g_flagsH);
    newTransflexGridSizer->Add(defaultPayeeChoice, g_flagsExpand);
    newTransflexGridSizer->Add(new wxStaticText(transSettingsStaticBox, wxID_STATIC, _t("Default Deposit/Withdrawal Category")), g_flagsH);
    newTransflexGridSizer->Add(defaultCategoryNonTransferChoice, g_flagsExpand);
    newTransflexGridSizer->Add(new wxStaticText(transSettingsStaticBox, wxID_STATIC, _t("Default Transfer Category")), g_flagsH);
    newTransflexGridSizer->Add(defaultCategoryTransferChoice, g_flagsExpand);
    newTransflexGridSizer->Add(new wxStaticText(transSettingsStaticBox, wxID_STATIC, _t("Default Status")), g_flagsH);
    newTransflexGridSizer->Add(default_status, g_flagsExpand);

    // transaction
    wxStaticBox* transactionCopyStaticBox = new wxStaticBox(home_panel, wxID_STATIC, _t("Transaction"));
    wxStaticBoxSizer* transactionCopyStaticBoxSizer = new wxStaticBoxSizer(transactionCopyStaticBox, wxVERTICAL);
    homePanelSizer->Add(transactionCopyStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_use_org_date_copy_paste = new wxCheckBox(transactionCopyStaticBox, wxID_ANY, _t("Use Original Date when Pasting Transactions"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_use_org_date_copy_paste->SetValue(GetIniDatabaseCheckboxValue(INIDB_USE_ORG_DATE_COPYPASTE, false));
    mmToolTip(m_use_org_date_copy_paste, _t("Select whether to use the original transaction date or current date when copying/pasting transactions"));
    transactionCopyStaticBoxSizer->Add(m_use_org_date_copy_paste, g_flagsV);

    m_use_org_date_duplicate = new wxCheckBox(transactionCopyStaticBox, wxID_ANY, _t("Use Original Date when Duplicating Transactions"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_use_org_date_duplicate->SetValue(GetIniDatabaseCheckboxValue(INIDB_USE_ORG_DATE_DUPLICATE, false));
    mmToolTip(m_use_org_date_duplicate, _t("Select whether to use the original transaction date or current date when duplicating transactions"));
    transactionCopyStaticBoxSizer->Add(m_use_org_date_duplicate, g_flagsV);

    m_use_org_state_duplicate_paste = new wxCheckBox(transactionCopyStaticBox, wxID_ANY, _t("Use Original State when Duplicating or Pasting Transactions"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_use_org_state_duplicate_paste->SetValue(GetIniDatabaseCheckboxValue(INIDB_USE_ORG_STATE_DUPLICATE_PASTE, true));
    mmToolTip(m_use_org_state_duplicate_paste, _t("Select whether to use the original state or default state when duplicating or copy and paste transactions"));
    transactionCopyStaticBoxSizer->Add(m_use_org_state_duplicate_paste, g_flagsV);

    wxArrayString sounds;
    sounds.Add(_t("None"));
    sounds.Add("drop.wav");
    sounds.Add("cash.wav");

    wxBoxSizer* soundBaseSizer = new wxBoxSizer(wxHORIZONTAL);
    transactionCopyStaticBoxSizer->Add(soundBaseSizer, wxSizerFlags(g_flagsV).Border(wxLEFT, 0));
    soundBaseSizer->Add(new wxStaticText(transactionCopyStaticBox, wxID_STATIC, _t("Transaction Sound")), g_flagsH);
    m_use_sound = new wxChoice(transactionCopyStaticBox, wxID_STATIC
        , wxDefaultPosition, wxDefaultSize
        , sounds);
    m_use_sound->SetSelection(SettingModel::instance().getInt(INIDB_USE_TRANSACTION_SOUND, 0));
    mmToolTip(m_use_sound, _t("Select whether to use sounds when entering transactions"));
    #ifdef __linux__
        m_use_sound->Bind(wxEVT_CHOICE, &TrxPref::onSoundChoice, this);
    #endif
    soundBaseSizer->Add(m_use_sound, g_flagsV);

    // Transaction/Budget options
    wxStaticBox* trxBox = new wxStaticBox(home_panel, wxID_STATIC,
        _t("Transaction/Budget")
    );
    wxStaticBoxSizer* trxSizer = new wxStaticBoxSizer(trxBox, wxVERTICAL);
    homePanelSizer->Add(trxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_budget_financial_years = new wxCheckBox(
        trxBox, wxID_ANY,
        _t("View Budgets as Financial Years"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    m_budget_financial_years->SetValue(PrefModel::instance().getBudgetFinancialYears());
    trxSizer->Add(m_budget_financial_years, g_flagsV);

    m_budget_include_transfers = new wxCheckBox(
        trxBox, wxID_ANY,
        _t("View Budgets with 'transfer' transactions"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    m_budget_include_transfers->SetValue(PrefModel::instance().getBudgetIncludeTransfers());
    trxSizer->Add(m_budget_include_transfers, g_flagsV);

    m_budget_summary_without_category = new wxCheckBox(
        trxBox, wxID_ANY,
        _t("View Budget Category Report with Summaries"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    m_budget_summary_without_category->SetValue(PrefModel::instance().getBudgetSummaryWithoutCategories());
    trxSizer->Add(m_budget_summary_without_category, g_flagsV);

    // Budget Yearly/Monthly relationship if both exist
    m_budget_override = new wxCheckBox(
        trxBox, wxID_ANY,
        _t("Override yearly budget with monthly budget"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    mmToolTip(m_budget_override, _t("If monthly budget exists then use this to override the yearly budget; otherwise combine them"));
    m_budget_override->SetValue(PrefModel::instance().getBudgetOverride());
    trxSizer->Add(m_budget_override, g_flagsV);

    // Option to deduct monthly budget from yearly budget for reporting
    m_budget_deduct_monthly = new wxCheckBox(
        trxBox, wxID_ANY,
        _t("Subtract monthly budgets from yearly budget in reporting"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    mmToolTip(m_budget_deduct_monthly, _t("Yearly budget will be reduced by the amount budgeted monthly.\nTotal estimate for the year will be reported as either the yearly budget OR the sum of the monthly budgets, whichever is greater."));
    m_budget_deduct_monthly->SetValue(PrefModel::instance().getBudgetDeductMonthly());
    trxSizer->Add(m_budget_deduct_monthly, g_flagsV);

    wxFlexGridSizer* trxChoiceSizer = new wxFlexGridSizer(0, 2, 0, 5);
    trxSizer->Add(trxChoiceSizer);

    trxChoiceSizer->Add(new wxStaticText(trxBox, wxID_STATIC, _t("Budget Offset (days)")), g_flagsH);
    m_budget_days_offset = new wxSpinCtrl(
        trxBox, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxSP_ARROW_KEYS, -30, +30
    );
    mmToolTip(m_budget_days_offset, _t("Adjusts the first day of month (normally 1st) for budget calculations"));
    m_budget_days_offset->SetValue(PrefModel::instance().getBudgetDaysOffset());
    trxChoiceSizer->Add(m_budget_days_offset, g_flagsH);

    trxChoiceSizer->Add(new wxStaticText(trxBox, wxID_STATIC, _t("First Day of Month")), g_flagsH);
    m_reporting_firstday = new wxSpinCtrl(
        trxBox, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxSP_ARROW_KEYS, 1, 28
    );
    mmToolTip(m_reporting_firstday, _t("Adjusts the first day of month for reporting"));
    m_reporting_firstday->SetValue(PrefModel::instance().getReportingFirstDay());
    trxChoiceSizer->Add(m_reporting_firstday, g_flagsH);

    trxChoiceSizer->Add(new wxStaticText(trxBox, wxID_STATIC, _t("First Weekday")), g_flagsH);
    m_reporting_first_weekday = new wxChoice(trxBox, wxID_ANY);
    m_reporting_first_weekday->Append(wxGetTranslation(g_days_of_week[0]));
    m_reporting_first_weekday->Append(wxGetTranslation(g_days_of_week[1]));
    m_reporting_first_weekday->SetSelection(PrefModel::instance().getReportingFirstWeekday());
    mmToolTip(m_reporting_first_weekday, _t("Adjusts the first day of week for filtering and reporting"));
    trxChoiceSizer->Add(m_reporting_first_weekday, g_flagsH);

    m_ignore_future_transactions = new wxCheckBox(
        trxBox, wxID_ANY,
        _t("Ignore Future Transactions"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    m_ignore_future_transactions->SetValue(PrefModel::instance().getIgnoreFutureTransactions());
    trxSizer->Add(m_ignore_future_transactions, g_flagsV);

    m_use_trans_date_time = new wxCheckBox(trxBox, wxID_ANY, _t("Use 'Time' in transaction recording/reporting"));
    m_use_trans_date_time->SetValue(PrefModel::instance().getUseTransDateTime());
    trxSizer->Add(m_use_trans_date_time, g_flagsV);

    m_treat_date_as_SN = new wxCheckBox(trxBox, wxID_ANY, _t("Sort Date identically to Sequence Number (SN)"));
    m_treat_date_as_SN->SetValue(PrefModel::instance().getTreatDateAsSN());
    trxSizer->Add(m_treat_date_as_SN, g_flagsV);

    // Colors settings
    wxStaticBox* colorsBox = new wxStaticBox(home_panel, wxID_ANY, _t("Transaction Colors"));
    wxStaticBoxSizer* colorsSizer = new wxStaticBoxSizer(colorsBox, wxHORIZONTAL);
    homePanelSizer->Add(colorsSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    int size_x = 30;
    m_UDFCB1 = new wxButton(colorsBox, wxID_HIGHEST + 11, "1", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB1->SetBackgroundColour(mmUserColor::s_color1);
    colorsSizer->Add(m_UDFCB1, g_flagsH);

    m_UDFCB2 = new wxButton(colorsBox, wxID_HIGHEST + 22, "2", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB2->SetBackgroundColour(mmUserColor::s_color2);
    colorsSizer->Add(m_UDFCB2, g_flagsH);

    m_UDFCB3 = new wxButton(colorsBox, wxID_HIGHEST + 33, "3", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB3->SetBackgroundColour(mmUserColor::s_color3);
    colorsSizer->Add(m_UDFCB3, g_flagsH);

    m_UDFCB4 = new wxButton(colorsBox, wxID_HIGHEST + 44, "4", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB4->SetBackgroundColour(mmUserColor::s_color4);
    colorsSizer->Add(m_UDFCB4, g_flagsH);

    m_UDFCB5 = new wxButton(colorsBox, wxID_HIGHEST + 55, "5", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB5->SetBackgroundColour(mmUserColor::s_color5);
    colorsSizer->Add(m_UDFCB5, g_flagsH);

    m_UDFCB6 = new wxButton(colorsBox, wxID_HIGHEST + 66, "6", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB6->SetBackgroundColour(mmUserColor::s_color6);
    colorsSizer->Add(m_UDFCB6, g_flagsH);

    m_UDFCB7 = new wxButton(colorsBox, wxID_HIGHEST + 77, "7", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB7->SetBackgroundColour(mmUserColor::s_color7);
    colorsSizer->Add(m_UDFCB7, g_flagsH);

    wxButton* reset = new wxButton(colorsBox, wxID_REDO, _t("Default"), wxDefaultPosition, wxDefaultSize, 0);
    m_UDFCB7->SetBackgroundColour(mmUserColor::s_color7);
    colorsSizer->Add(reset, g_flagsH);

    SetBoldFontToStaticBoxHeader(transSettingsStaticBox);
    SetBoldFontToStaticBoxHeader(transactionCopyStaticBox);
    SetBoldFontToStaticBoxHeader(trxBox);
    SetBoldFontToStaticBoxHeader(colorsBox);

    // -------------
    Fit();
    home_panel->SetMinSize(home_panel->GetBestVirtualSize());
    home_panel->SetScrollRate(6, 6);

    this->Connect(
        wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(TrxPref::OnNavTreeColorChanged),
        nullptr, this
    );
}

bool TrxPref::SaveSettings()
{
    SettingModel::instance().saveBool(INIDB_USE_ORG_DATE_COPYPASTE, m_use_org_date_copy_paste->GetValue());
    SettingModel::instance().saveBool(INIDB_USE_ORG_DATE_DUPLICATE, m_use_org_date_duplicate->GetValue());
    SettingModel::instance().saveBool(INIDB_USE_ORG_STATE_DUPLICATE_PASTE, m_use_org_state_duplicate_paste->GetValue());
    SettingModel::instance().saveInt(INIDB_USE_TRANSACTION_SOUND, m_use_sound->GetSelection());

    PrefModel::instance().saveBudgetFinancialYears(m_budget_financial_years->GetValue());
    PrefModel::instance().saveBudgetIncludeTransfers(m_budget_include_transfers->GetValue());
    PrefModel::instance().saveBudgetSummaryWithoutCategories(m_budget_summary_without_category->GetValue());
    PrefModel::instance().saveBudgetOverride(m_budget_override->GetValue());
    PrefModel::instance().saveBudgetDeductMonthly(m_budget_deduct_monthly->GetValue());
    PrefModel::instance().saveBudgetDaysOffset(m_budget_days_offset->GetValue());
    PrefModel::instance().saveReportingFirstDay(m_reporting_firstday->GetValue());
    PrefModel::instance().saveReportingFirstWeekday(
        static_cast<wxDateTime::WeekDay>(m_reporting_first_weekday->GetSelection())
    );
    PrefModel::instance().saveIgnoreFutureTransactions(m_ignore_future_transactions->GetValue());
    PrefModel::instance().saveUseTransDateTime(m_use_trans_date_time->GetValue());
    PrefModel::instance().saveTreatDateAsSN(m_treat_date_as_SN->GetValue());

    mmUserColor::s_color1 = m_UDFCB1->GetBackgroundColour();
    mmUserColor::s_color2 = m_UDFCB2->GetBackgroundColour();
    mmUserColor::s_color3 = m_UDFCB3->GetBackgroundColour();
    mmUserColor::s_color4 = m_UDFCB4->GetBackgroundColour();
    mmUserColor::s_color5 = m_UDFCB5->GetBackgroundColour();
    mmUserColor::s_color6 = m_UDFCB6->GetBackgroundColour();
    mmUserColor::s_color7 = m_UDFCB7->GetBackgroundColour();

    InfoModel::instance().saveColour("USER_COLOR1", mmUserColor::s_color1);
    InfoModel::instance().saveColour("USER_COLOR2", mmUserColor::s_color2);
    InfoModel::instance().saveColour("USER_COLOR3", mmUserColor::s_color3);
    InfoModel::instance().saveColour("USER_COLOR4", mmUserColor::s_color4);
    InfoModel::instance().saveColour("USER_COLOR5", mmUserColor::s_color5);
    InfoModel::instance().saveColour("USER_COLOR6", mmUserColor::s_color6);
    InfoModel::instance().saveColour("USER_COLOR7", mmUserColor::s_color7);

    wxChoice* itemChoice = static_cast<wxChoice*>(
        FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_PAYEE)
    );
    PrefModel::instance().saveTransPayeeNone(itemChoice->GetSelection());

    itemChoice = static_cast<wxChoice*>(
        FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY_NONTRANSFER)
    );
    PrefModel::instance().saveTransCategoryNone(itemChoice->GetSelection());

    itemChoice = static_cast<wxChoice*>(
        FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY_TRANSFER)
    );
    PrefModel::instance().saveTransCategoryTransferNone(itemChoice->GetSelection());

    itemChoice = static_cast<wxChoice*>(
        FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_STATUS)
    );
    PrefModel::instance().saveTrxStatus(TrxStatus(itemChoice->GetSelection()));

    itemChoice = static_cast<wxChoice*>(
        FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_DATE)
    );
    PrefModel::instance().saveTransDateDefault(itemChoice->GetSelection());

    return true;
}

void TrxPref::OnNavTreeColorChanged(wxCommandEvent& event)
{
    if (event.GetId() == wxID_REDO)
    {
        mmLoadColorsFromDatabase(true);
        m_UDFCB1->SetBackgroundColour(mmUserColor::s_color1);
        m_UDFCB2->SetBackgroundColour(mmUserColor::s_color2);
        m_UDFCB3->SetBackgroundColour(mmUserColor::s_color3);
        m_UDFCB4->SetBackgroundColour(mmUserColor::s_color4);
        m_UDFCB5->SetBackgroundColour(mmUserColor::s_color5);
        m_UDFCB6->SetBackgroundColour(mmUserColor::s_color6);
        m_UDFCB7->SetBackgroundColour(mmUserColor::s_color7);
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

#ifdef __linux__
    void TrxPref::onSoundChoice(wxCommandEvent& event)
    {
        if (event.GetSelection() > 0) {
            long result = wxExecute("paplay --version", wxEXEC_SYNC | wxEXEC_HIDE_CONSOLE);
            if (result != 0) {
                wxLogWarning("The selected transaction sound can not be configured!\n\nThe command 'paplay' is missing or could not be executed, please check your system configuration.");
                m_use_sound->SetSelection(0);
            }
        } 
    }
#endif
