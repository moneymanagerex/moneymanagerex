/*******************************************************
Copyright (C) 2014 Stefano Giorgio
Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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
#include "images_list.h"
#include "util.h"

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

    // View Options
    wxStaticBox* viewStaticBox = new wxStaticBox(this, wxID_STATIC, _("View Options"));
    SetBoldFont(viewStaticBox);

    wxStaticBoxSizer* viewStaticBoxSizer = new wxStaticBoxSizer(viewStaticBox, wxVERTICAL);
    viewsPanelSizer->Add(viewStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));
    wxFlexGridSizer* view_sizer1 = new wxFlexGridSizer(0, 2, 0, 5);
    viewStaticBoxSizer->Add(view_sizer1);

    view_sizer1->Add(new wxStaticText(this, wxID_STATIC, _("Accounts Visible")), g_flagsH);

    const wxString vAccts = Model_Setting::instance().ViewAccounts();
    wxArrayString view_accounts;
    view_accounts.Add(VIEW_ACCOUNTS_ALL_STR);
    view_accounts.Add(VIEW_ACCOUNTS_OPEN_STR);
    view_accounts.Add(VIEW_ACCOUNTS_FAVORITES_STR);

    m_choice_visible = new wxChoice(this, wxID_ANY);
    for (const auto& entry : view_accounts)
    {
        m_choice_visible->Append(wxGetTranslation(entry), new wxStringClientData(entry));
        if (entry == vAccts)
            m_choice_visible->SetStringSelection(wxGetTranslation(entry));
    }

    view_sizer1->Add(m_choice_visible, g_flagsH);
    m_choice_visible->SetToolTip(_("Specify which accounts are visible"));

    //Category delimiter
    view_sizer1->Add(new wxStaticText(this, wxID_STATIC, _("Category Delimiter")), g_flagsH);

    wxArrayString list;
    list.Add(":");
    list.Add(": ");
    list.Add(" : ");

    wxString delimiter = Model_Infotable::instance().GetStringInfo("CATEG_DELIMITER",":");
    m_categ_delimiter_list = new wxComboBox(this, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize, list);
    m_categ_delimiter_list->SetValue(delimiter);

    view_sizer1->Add(m_categ_delimiter_list, g_flagsH);

    // Budget options
    wxStaticBox* trxStaticBox = new wxStaticBox(this, wxID_STATIC, _("Transaction/Budget Options"));
    SetBoldFont(trxStaticBox);
    wxStaticBoxSizer* trxStaticBoxSizer = new wxStaticBoxSizer(trxStaticBox, wxVERTICAL);
    viewsPanelSizer->Add(trxStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));


    m_budget_financial_years = new wxCheckBox(this, wxID_STATIC, _("View Budgets as Financial Years"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_budget_financial_years->SetValue(Option::instance().BudgetFinancialYears());
    trxStaticBoxSizer->Add(m_budget_financial_years, g_flagsV);

    m_budget_include_transfers = new wxCheckBox(this, wxID_STATIC
        , _("View Budgets with 'transfer' transactions")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_budget_include_transfers->SetValue(Option::instance().BudgetIncludeTransfers());
    trxStaticBoxSizer->Add(m_budget_include_transfers, g_flagsV);

    m_budget_summary_without_category = new wxCheckBox(this, wxID_STATIC
        , _("View Budget Category Report with Summaries")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_budget_summary_without_category->SetValue(Option::instance().BudgetReportWithSummaries());
    trxStaticBoxSizer->Add(m_budget_summary_without_category, g_flagsV);

    m_ignore_future_transactions = new wxCheckBox(this, wxID_STATIC
        , _("Ignore Future Transactions")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_ignore_future_transactions->SetValue(Option::instance().getIgnoreFutureTransactions());
    trxStaticBoxSizer->Add(m_ignore_future_transactions, g_flagsV);

    // Colours settings
    wxStaticBox* userColourSettingStBox = new wxStaticBox(this, wxID_ANY, _("Transaction Colors"));
    SetBoldFont(userColourSettingStBox);
    wxStaticBoxSizer* userColourSettingStBoxSizer = new wxStaticBoxSizer(userColourSettingStBox, wxHORIZONTAL);
    viewsPanelSizer->Add(userColourSettingStBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    int size_x = 30;
    m_UDFCB1 = new wxButton(this, wxID_HIGHEST + 11, "1", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB1->SetBackgroundColour(mmColors::userDefColor1);
    userColourSettingStBoxSizer->Add(m_UDFCB1, g_flagsH);

    m_UDFCB2 = new wxButton(this, wxID_HIGHEST + 22, "2", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB2->SetBackgroundColour(mmColors::userDefColor2);
    userColourSettingStBoxSizer->Add(m_UDFCB2, g_flagsH);

    m_UDFCB3 = new wxButton(this, wxID_HIGHEST + 33, "3", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB3->SetBackgroundColour(mmColors::userDefColor3);
    userColourSettingStBoxSizer->Add(m_UDFCB3, g_flagsH);

    m_UDFCB4 = new wxButton(this, wxID_HIGHEST + 44, "4", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB4->SetBackgroundColour(mmColors::userDefColor4);
    userColourSettingStBoxSizer->Add(m_UDFCB4, g_flagsH);

    m_UDFCB5 = new wxButton(this, wxID_HIGHEST + 55, "5", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB5->SetBackgroundColour(mmColors::userDefColor5);
    userColourSettingStBoxSizer->Add(m_UDFCB5, g_flagsH);

    m_UDFCB6 = new wxButton(this, wxID_HIGHEST + 66, "6", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB6->SetBackgroundColour(mmColors::userDefColor6);
    userColourSettingStBoxSizer->Add(m_UDFCB6, g_flagsH);

    m_UDFCB7 = new wxButton(this, wxID_HIGHEST + 77, "7", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB7->SetBackgroundColour(mmColors::userDefColor7);
    userColourSettingStBoxSizer->Add(m_UDFCB7, g_flagsH);

    // UI Appearance

    wxStaticBox* iconStaticBox = new wxStaticBox(this, wxID_STATIC, _("UI Appearance"));
    SetBoldFont(iconStaticBox);

    wxStaticBoxSizer* iconStaticBoxSizer = new wxStaticBoxSizer(iconStaticBox, wxVERTICAL);
    viewsPanelSizer->Add(iconStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxFlexGridSizer* view_sizer2 = new wxFlexGridSizer(0, 2, 0, 5);
    iconStaticBoxSizer->Add(view_sizer2);

    // Theme

    m_choice_theme = new wxChoice(this, wxID_ANY);

    view_sizer2->Add(new wxStaticText(this, wxID_STATIC, _("Style Template")), g_flagsH);
    view_sizer2->Add(m_choice_theme, g_flagsH);

    wxString myTheme = Model_Setting::instance().Theme();
    for (const auto& entry : getThemes())
    {
        m_choice_theme->Append(wxGetTranslation(entry), new wxStringClientData(entry));
        if (entry == myTheme)
            m_choice_theme->SetStringSelection(wxGetTranslation(entry));
    }

    view_sizer2->Add(new wxStaticText(this, wxID_STATIC, _("HTML Scale Factor")), g_flagsH);

    int max = 300; int min = 25;
    m_scale_factor = new wxSpinCtrl(this, wxID_ANY
        , wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, min, max);

    int vFontSize = Option::instance().getHtmlFontSize();
    m_scale_factor->SetValue(vFontSize);
    m_scale_factor->SetToolTip(_("Specify which scale factor is used for the report pages"));
    view_sizer2->Add(m_scale_factor, g_flagsH);

    //
    const wxString settings_choice[] = {
        wxTRANSLATE("Small (16 px)"),
        wxTRANSLATE("Normal (24 px)"),
        wxTRANSLATE("Large (32 px)"),
        wxTRANSLATE("Huge (48 px)")
    };

    m_toolbar_icon_size = new wxChoice(this, wxID_RESIZE_FRAME);
    m_navigation_icon_size = new wxChoice(this, wxID_RESIZE_FRAME);
    m_others_icon_size = new wxChoice(this, wxID_RESIZE_FRAME);

    for (const auto& entry : settings_choice)
    {
        m_toolbar_icon_size->Append(wxGetTranslation(entry), new wxStringClientData(entry));
        m_navigation_icon_size->Append(wxGetTranslation(entry), new wxStringClientData(entry));
        m_others_icon_size->Append(wxGetTranslation(entry), new wxStringClientData(entry));
    }

    view_sizer2->Add(new wxStaticText(this, wxID_STATIC, _("Toolbar Icon Size")), g_flagsH);
    view_sizer2->Add(m_toolbar_icon_size, g_flagsH);

    view_sizer2->Add(new wxStaticText(this, wxID_STATIC, _("Navigation Icon Size")), g_flagsH);
    view_sizer2->Add(m_navigation_icon_size, g_flagsH);

    view_sizer2->Add(new wxStaticText(this, wxID_STATIC, _("Others Icon Size")), g_flagsH);
    view_sizer2->Add(m_others_icon_size, g_flagsH);

    int vIconSize = Option::instance().getToolbarIconSize();
    int selection = vIconSize / 8 - (vIconSize > 32 ? 3 : 2);
    m_toolbar_icon_size->SetSelection(selection);

    vIconSize = Option::instance().getNavigationIconSize();
    selection = vIconSize / 8 - (vIconSize > 32 ? 3 : 2);
    m_navigation_icon_size->SetSelection(selection);

    vIconSize = Option::instance().getIconSize();
    selection = vIconSize / 8 - (vIconSize > 32 ? 3 : 2);
    m_others_icon_size->SetSelection(selection);

    this->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(OptionSettingsView::OnNavTreeColorChanged), nullptr, this);
}

void OptionSettingsView::OnNavTreeColorChanged(wxCommandEvent& event)
{
    wxButton* button = wxDynamicCast(FindWindow(event.GetId()), wxButton);
    if (button)
    {
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
}

bool OptionSettingsView::SaveSettings()
{
    auto delimiter = m_categ_delimiter_list->GetValue();
    if (delimiter.empty()) delimiter = ":";
    Model_Infotable::instance().Set("CATEG_DELIMITER", delimiter);

    wxString accVisible = VIEW_ACCOUNTS_ALL_STR;
    wxStringClientData* visible_acc_obj = static_cast<wxStringClientData*>(m_choice_visible->GetClientObject(m_choice_visible->GetSelection()));
    if (visible_acc_obj)
        accVisible = visible_acc_obj->GetData();
    Model_Setting::instance().SetViewAccounts(accVisible);

    wxString themeName = "default";
    wxStringClientData* theme_obj = static_cast<wxStringClientData*>(m_choice_theme->GetClientObject(m_choice_theme->GetSelection()));
    if (theme_obj)
        themeName = theme_obj->GetData();
    Model_Setting::instance().SetTheme(themeName);

    int size = m_scale_factor->GetValue();
    Option::instance().setHTMLFontSizes(size);

    int i[4] = { 16, 24, 32, 48 };
    size = m_others_icon_size->GetSelection();
    size = i[size];
    Option::instance().setIconSize(size);

    size = m_navigation_icon_size->GetSelection();
    size = i[size];
    Option::instance().setNavigationIconSize(size);

    size = m_toolbar_icon_size->GetSelection();
    size = i[size];
    Option::instance().setToolbarIconSize(size);

    Option::instance().BudgetFinancialYears(m_budget_financial_years->GetValue());
    Option::instance().BudgetIncludeTransfers(m_budget_include_transfers->GetValue());
    Option::instance().BudgetReportWithSummaries(m_budget_summary_without_category->GetValue());
    Option::instance().IgnoreFutureTransactions(m_ignore_future_transactions->GetValue());

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

    return true;
}
