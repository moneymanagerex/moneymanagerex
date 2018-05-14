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

#include "optionsettingsview.h"
#include "util.h"
#include "option.h"

#include <wx/colordlg.h>

/*******************************************************/
wxBEGIN_EVENT_TABLE(OptionSettingsView, wxPanel)
wxEND_EVENT_TABLE()
/*******************************************************/

OptionSettingsView::OptionSettingsView()
{
}

OptionSettingsView::OptionSettingsView(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
{
    wxPanel::Create(parent, id, pos, size, style, name);
    Create();
}

OptionSettingsView::~OptionSettingsView()
{
}

void OptionSettingsView::Create()
{
    wxBoxSizer* viewsPanelSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(viewsPanelSizer);

    // Account View Options
    wxStaticBox* accountStaticBox = new wxStaticBox(this, wxID_STATIC, _("View Options"));
    SetBoldFont(accountStaticBox);

    wxStaticBoxSizer* accountStaticBoxSizer = new wxStaticBoxSizer(accountStaticBox, wxVERTICAL);
    viewsPanelSizer->Add(accountStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));
    wxFlexGridSizer* view_sizer1 = new wxFlexGridSizer(0, 2, 0, 5);
    accountStaticBoxSizer->Add(view_sizer1);

    view_sizer1->Add(new wxStaticText(this, wxID_STATIC, _("Accounts Visible")), g_flagsH);

    const wxString vAccts = Model_Setting::instance().ViewAccounts();
    wxArrayString view_accounts;
    view_accounts.Add(VIEW_ACCOUNTS_ALL_STR);
    view_accounts.Add(VIEW_ACCOUNTS_OPEN_STR);
    view_accounts.Add(VIEW_ACCOUNTS_FAVORITES_STR);

    m_choice_visible = new wxChoice(this, ID_DIALOG_OPTIONS_VIEW_ACCOUNTS);
    for (const auto& entry : view_accounts)
    {
        m_choice_visible->Append(wxGetTranslation(entry), new wxStringClientData(entry));
        if (entry == vAccts)
            m_choice_visible->SetStringSelection(wxGetTranslation(entry));
    }

    view_sizer1->Add(m_choice_visible, g_flagsH);
    m_choice_visible->SetToolTip(_("Specify which accounts are visible"));

    view_sizer1->Add(new wxStaticText(this, wxID_STATIC, _("Transactions Visible")), g_flagsH);

    wxArrayString view_strings;
    view_strings.Add(VIEW_TRANS_ALL_STR);
    view_strings.Add(VIEW_TRANS_TODAY_STR);
    view_strings.Add(VIEW_TRANS_CURRENT_MONTH_STR);
    view_strings.Add(VIEW_TRANS_LAST_30_DAYS_STR);
    view_strings.Add(VIEW_TRANS_LAST_90_DAYS_STR);
    view_strings.Add(VIEW_TRANS_LAST_MONTH_STR);
    view_strings.Add(VIEW_TRANS_LAST_3MONTHS_STR);
    view_strings.Add(VIEW_TRANS_LAST_12MONTHS_STR);
    view_strings.Add(VIEW_TRANS_CURRENT_YEAR_STR);
    view_strings.Add(VIEW_TRANS_CURRENT_FIN_YEAR_STR);
    view_strings.Add(VIEW_TRANS_LAST_YEAR_STR);
    view_strings.Add(VIEW_TRANS_LAST_FIN_YEAR_STR);

    m_choice_trans_visible = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    for (const auto &entry : view_strings) {
        m_choice_trans_visible->Append(wxGetTranslation(entry)
            , new wxStringClientData(entry));
    }

    view_sizer1->Add(m_choice_trans_visible, g_flagsH);

    const wxString vTrans = Model_Setting::instance().ViewTransactions();
    m_choice_trans_visible->SetStringSelection(wxGetTranslation(vTrans));
    m_choice_trans_visible->SetToolTip(_("Specify which transactions are visible by default"));

    view_sizer1->Add(new wxStaticText(this, wxID_STATIC, _("HTML scale factor")), g_flagsH);

    int max = 300; int min = 25;
    m_scale_factor = new wxSpinCtrl(this, wxID_ANY
        , wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, min, max);

    int vFontSize = Option::instance().HtmlFontSize();
    m_scale_factor->SetValue(vFontSize);
    m_scale_factor->SetToolTip(_("Specify which scale factor is used for the report pages"));
    view_sizer1->Add(m_scale_factor, g_flagsH);

    // Budget Options
    wxStaticBox* budgetStaticBox = new wxStaticBox(this, wxID_STATIC, _("Budget Options"));
    SetBoldFont(budgetStaticBox);

    wxStaticBoxSizer* budgetSizer = new wxStaticBoxSizer(budgetStaticBox, wxVERTICAL);
    viewsPanelSizer->Add(budgetSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_budget_financial_years = new wxCheckBox(this, wxID_STATIC, _("View as Financial Years")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_budget_financial_years->SetValue(Option::instance().BudgetFinancialYears());
    budgetSizer->Add(m_budget_financial_years, g_flagsV);

    m_budget_include_transfers = new wxCheckBox(this, wxID_STATIC
        , _("View with 'transfer' transactions")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_budget_include_transfers->SetValue(Option::instance().BudgetIncludeTransfers());
    budgetSizer->Add(m_budget_include_transfers, g_flagsV);

    m_budget_setup_without_summary = new wxCheckBox(this, wxID_STATIC
        , _("View Setup Without Budget Summaries")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_budget_setup_without_summary->SetValue(Option::instance().BudgetSetupWithoutSummaries());
    budgetSizer->Add(m_budget_setup_without_summary, g_flagsV);

    m_budget_summary_without_category = new wxCheckBox(this, wxID_STATIC
        , _("View Category Report with Summaries")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_budget_summary_without_category->SetValue(Option::instance().BudgetReportWithSummaries());
    budgetSizer->Add(m_budget_summary_without_category, g_flagsV);

    // Allows a year or financial year to start before or after the 1st of the month.
    wxBoxSizer* budget_offset_sizer = new wxBoxSizer(wxHORIZONTAL);
    budgetSizer->Add(budget_offset_sizer);

    budget_offset_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Budget Offset (days):")), g_flagsH);
    
    m_budget_days_offset = new wxSpinCtrl(this, wxID_ANY
        , wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -30, +30);
    m_budget_days_offset->SetToolTip(_("Advance or retard the start date from the 1st of the month or year by the number of days"));
    m_budget_days_offset->SetValue(Option::instance().BudgetDaysOffset());
    budget_offset_sizer->Add(m_budget_days_offset, g_flagsH);

    // Report Options
    wxStaticBox* reportStaticBox = new wxStaticBox(this, wxID_STATIC, _("Report Options"));
    SetBoldFont(reportStaticBox);

    wxStaticBoxSizer* reportSizer = new wxStaticBoxSizer(reportStaticBox, wxVERTICAL);
    viewsPanelSizer->Add(reportSizer, wxSizerFlags(g_flagsExpand).Proportion(0));
    m_ignore_future_transactions = new wxCheckBox(this, wxID_STATIC
        , _("View without Future Transactions")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_ignore_future_transactions->SetValue(Option::instance().IgnoreFutureTransactions());
    reportSizer->Add(m_ignore_future_transactions, g_flagsV);

    // Colours settings
    wxStaticBox* userColourSettingStBox = new wxStaticBox(this, wxID_ANY, _("User Colors"));
    SetBoldFont(userColourSettingStBox);
    wxStaticBoxSizer* userColourSettingStBoxSizer = new wxStaticBoxSizer(userColourSettingStBox, wxHORIZONTAL);
    viewsPanelSizer->Add(userColourSettingStBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    int size_x = 55;
    m_UDFCB1 = new wxButton(this, wxID_HIGHEST + 11, L"1  \u2588\u2588", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB1->SetForegroundColour(mmColors::userDefColor1);
    userColourSettingStBoxSizer->Add(m_UDFCB1, g_flagsH);

    m_UDFCB2 = new wxButton(this, wxID_HIGHEST + 22, L"2  \u2588\u2588", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB2->SetForegroundColour(mmColors::userDefColor2);
    userColourSettingStBoxSizer->Add(m_UDFCB2, g_flagsH);

    m_UDFCB3 = new wxButton(this, wxID_HIGHEST + 33, L"3  \u2588\u2588", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB3->SetForegroundColour(mmColors::userDefColor3);
    userColourSettingStBoxSizer->Add(m_UDFCB3, g_flagsH);

    m_UDFCB4 = new wxButton(this, wxID_HIGHEST + 44, L"4  \u2588\u2588", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB4->SetForegroundColour(mmColors::userDefColor4);
    userColourSettingStBoxSizer->Add(m_UDFCB4, g_flagsH);

    m_UDFCB5 = new wxButton(this, wxID_HIGHEST + 55, L"5  \u2588\u2588", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB5->SetForegroundColour(mmColors::userDefColor5);
    userColourSettingStBoxSizer->Add(m_UDFCB5, g_flagsH);

    m_UDFCB6 = new wxButton(this, wxID_HIGHEST + 66, L"6  \u2588\u2588", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB6->SetForegroundColour(mmColors::userDefColor6);
    userColourSettingStBoxSizer->Add(m_UDFCB6, g_flagsH);

    m_UDFCB7 = new wxButton(this, wxID_HIGHEST + 77, L"7  \u2588\u2588", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB7->SetForegroundColour(mmColors::userDefColor7);
    userColourSettingStBoxSizer->Add(m_UDFCB7, g_flagsH);

    this->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(OptionSettingsView::OnNavTreeColorChanged), nullptr, this);
}

void OptionSettingsView::OnNavTreeColorChanged(wxCommandEvent& event)
{
    wxButton* button = wxDynamicCast(FindWindow(event.GetId()), wxButton);
    if (button)
    {
        wxColour colour = button->GetForegroundColour();
        wxColourData data;
        data.SetChooseFull(true);
        data.SetColour(colour);

        wxColourDialog dialog(this, &data);
        if (dialog.ShowModal() == wxID_OK)
        {
            colour = dialog.GetColourData().GetColour();
            button->SetForegroundColour(colour);
        }
    }
}

void OptionSettingsView::SaveSettings()
{
    wxString accVisible = VIEW_ACCOUNTS_ALL_STR;
    wxStringClientData* visible_acc_obj = (wxStringClientData *)m_choice_visible->GetClientObject(m_choice_visible->GetSelection());
    if (visible_acc_obj)
        accVisible = visible_acc_obj->GetData();
    Model_Setting::instance().SetViewAccounts(accVisible);

    wxString transVisible = VIEW_TRANS_ALL_STR;
    wxStringClientData* visible_obj = (wxStringClientData *)m_choice_trans_visible->GetClientObject(m_choice_trans_visible->GetSelection());
    if (visible_obj)
        transVisible = visible_obj->GetData();
    Model_Setting::instance().SetViewTransactions(transVisible);

    int size = m_scale_factor->GetValue();
    Option::instance().HtmlFontSize(size);

    Option::instance().BudgetFinancialYears(m_budget_financial_years->GetValue());
    Option::instance().BudgetIncludeTransfers(m_budget_include_transfers->GetValue());
    Option::instance().BudgetSetupWithoutSummaries(m_budget_setup_without_summary->GetValue());
    Option::instance().BudgetReportWithSummaries(m_budget_summary_without_category->GetValue());
    Option::instance().BudgetDaysOffset(m_budget_days_offset->GetValue());
    Option::instance().IgnoreFutureTransactions(m_ignore_future_transactions->GetValue());

    mmColors::userDefColor1 = m_UDFCB1->GetForegroundColour();
    mmColors::userDefColor2 = m_UDFCB2->GetForegroundColour();
    mmColors::userDefColor3 = m_UDFCB3->GetForegroundColour();
    mmColors::userDefColor4 = m_UDFCB4->GetForegroundColour();
    mmColors::userDefColor5 = m_UDFCB5->GetForegroundColour();
    mmColors::userDefColor6 = m_UDFCB6->GetForegroundColour();
    mmColors::userDefColor7 = m_UDFCB7->GetForegroundColour();

    Model_Infotable::instance().Set("USER_COLOR1", mmColors::userDefColor1);
    Model_Infotable::instance().Set("USER_COLOR2", mmColors::userDefColor2);
    Model_Infotable::instance().Set("USER_COLOR3", mmColors::userDefColor3);
    Model_Infotable::instance().Set("USER_COLOR4", mmColors::userDefColor4);
    Model_Infotable::instance().Set("USER_COLOR5", mmColors::userDefColor5);
    Model_Infotable::instance().Set("USER_COLOR6", mmColors::userDefColor6);
    Model_Infotable::instance().Set("USER_COLOR7", mmColors::userDefColor7);
}
