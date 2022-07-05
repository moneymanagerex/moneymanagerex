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
#include "themes.h"
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
    wxBoxSizer* viewsPanelSizer0 = new wxBoxSizer(wxVERTICAL);
    SetSizer(viewsPanelSizer0);

    wxScrolledWindow* view_panel = new wxScrolledWindow(this, wxID_ANY);
    wxBoxSizer* viewsPanelSizer = new wxBoxSizer(wxVERTICAL);
    view_panel->SetSizer(viewsPanelSizer);
    viewsPanelSizer0->Add(view_panel, wxSizerFlags(g_flagsExpand).Proportion(0));

    // View Options
    wxStaticBox* viewStaticBox = new wxStaticBox(view_panel, wxID_STATIC, _("View Options"));
    SetBoldFont(viewStaticBox);

    wxStaticBoxSizer* viewStaticBoxSizer = new wxStaticBoxSizer(viewStaticBox, wxVERTICAL);
    wxFlexGridSizer* view_sizer1 = new wxFlexGridSizer(0, 2, 0, 5);
    viewStaticBoxSizer->Add(view_sizer1);
    viewsPanelSizer->Add(viewStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    view_sizer1->Add(new wxStaticText(view_panel, wxID_STATIC, _("Accounts Visible")), g_flagsH);

    const wxString vAccts = Model_Setting::instance().GetViewAccounts();
    wxArrayString view_accounts;
    view_accounts.Add(VIEW_ACCOUNTS_ALL_STR);
    view_accounts.Add(VIEW_ACCOUNTS_OPEN_STR);
    view_accounts.Add(VIEW_ACCOUNTS_FAVORITES_STR);

    m_choice_visible = new wxChoice(view_panel, wxID_ANY);
    for (const auto& entry : view_accounts)
    {
        m_choice_visible->Append(wxGetTranslation(entry), new wxStringClientData(entry));
        if (entry == vAccts)
            m_choice_visible->SetStringSelection(wxGetTranslation(entry));
    }

    view_sizer1->Add(m_choice_visible, g_flagsH);
    mmToolTip(m_choice_visible, _("Specify which accounts are visible"));

    //Category delimiter
    view_sizer1->Add(new wxStaticText(view_panel, wxID_STATIC, _("Category Delimiter")), g_flagsH);

    wxArrayString list;
    list.Add(":");
    list.Add(": ");
    list.Add(" : ");

    wxString delimiter = Model_Infotable::instance().GetStringInfo("CATEG_DELIMITER",":");
    m_categ_delimiter_list = new wxComboBox(view_panel, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize, list);
    m_categ_delimiter_list->SetValue(delimiter);

    view_sizer1->Add(m_categ_delimiter_list, g_flagsH);

    m_showToolTips = new wxCheckBox(view_panel, wxID_STATIC
        , _("Show Tooltips")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_showToolTips->SetValue(Option::instance().getShowToolTips());
    view_sizer1->Add(m_showToolTips, g_flagsH);

    m_showMoneyTips = new wxCheckBox(view_panel, wxID_STATIC
        , _("Show Money Tips")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_showMoneyTips->SetValue(Option::instance().getShowMoneyTips());
    view_sizer1->Add(m_showMoneyTips, g_flagsH);

    // Transaction/Budget options
    wxStaticBox* trxStaticBox = new wxStaticBox(view_panel, wxID_STATIC, _("Transaction/Budget Options"));
    SetBoldFont(trxStaticBox);
    wxStaticBoxSizer* trxStaticBoxSizer = new wxStaticBoxSizer(trxStaticBox, wxVERTICAL);
    viewsPanelSizer->Add(trxStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));


    m_budget_financial_years = new wxCheckBox(view_panel, wxID_STATIC, _("View Budgets as Financial Years"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_budget_financial_years->SetValue(Option::instance().BudgetFinancialYears());
    trxStaticBoxSizer->Add(m_budget_financial_years, g_flagsV);

    m_budget_include_transfers = new wxCheckBox(view_panel, wxID_STATIC
        , _("View Budgets with 'transfer' transactions")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_budget_include_transfers->SetValue(Option::instance().BudgetIncludeTransfers());
    trxStaticBoxSizer->Add(m_budget_include_transfers, g_flagsV);

    m_budget_summary_without_category = new wxCheckBox(view_panel, wxID_STATIC
        , _("View Budget Category Report with Summaries")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_budget_summary_without_category->SetValue(Option::instance().BudgetReportWithSummaries());
    trxStaticBoxSizer->Add(m_budget_summary_without_category, g_flagsV);

    // Allows a year or financial year to start before or after the 1st of the month.
    wxBoxSizer* budget_offset_sizer = new wxBoxSizer(wxHORIZONTAL);
    trxStaticBoxSizer->Add(budget_offset_sizer);

    budget_offset_sizer->Add(new wxStaticText(view_panel, wxID_STATIC, _("Budget Offset (days):")), g_flagsH);

    m_budget_days_offset = new wxSpinCtrl(view_panel, wxID_ANY
        , wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -30, +30);
    mmToolTip(m_budget_days_offset, _("Advance or retard the start date from the 1st of the month or year by the number of days"));
    m_budget_days_offset->SetValue(Option::instance().getBudgetDaysOffset());
    budget_offset_sizer->Add(m_budget_days_offset, g_flagsH);

    // Allows the 'first day' in the month to be adjusted for reporting purposes
    wxBoxSizer* reporting_firstday_sizer = new wxBoxSizer(wxHORIZONTAL);
    trxStaticBoxSizer->Add(reporting_firstday_sizer);

    reporting_firstday_sizer->Add(new wxStaticText(view_panel, wxID_STATIC, _("Start day of month for reporting:")), g_flagsH);

    m_reporting_firstday = new wxSpinCtrl(view_panel, wxID_ANY
        , wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 28);
    mmToolTip(m_reporting_firstday, _("Allows the 'first day' in the month to be adjusted for reporting purposes"));
    m_reporting_firstday->SetValue(Option::instance().getReportingFirstDay());
    reporting_firstday_sizer->Add(m_reporting_firstday, g_flagsH);

    m_ignore_future_transactions = new wxCheckBox(view_panel, wxID_STATIC
        , _("Ignore Future Transactions")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_ignore_future_transactions->SetValue(Option::instance().getIgnoreFutureTransactions());
    trxStaticBoxSizer->Add(m_ignore_future_transactions, g_flagsV);

    // Colours settings
    wxStaticBox* userColourSettingStBox = new wxStaticBox(view_panel, wxID_ANY, _("Transaction Colors"));
    SetBoldFont(userColourSettingStBox);
    wxStaticBoxSizer* userColourSettingStBoxSizer = new wxStaticBoxSizer(userColourSettingStBox, wxHORIZONTAL);
    viewsPanelSizer->Add(userColourSettingStBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    int size_x = 30;
    m_UDFCB1 = new wxButton(view_panel, wxID_HIGHEST + 11, "1", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB1->SetBackgroundColour(mmColors::userDefColor1);
    userColourSettingStBoxSizer->Add(m_UDFCB1, g_flagsH);

    m_UDFCB2 = new wxButton(view_panel, wxID_HIGHEST + 22, "2", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB2->SetBackgroundColour(mmColors::userDefColor2);
    userColourSettingStBoxSizer->Add(m_UDFCB2, g_flagsH);

    m_UDFCB3 = new wxButton(view_panel, wxID_HIGHEST + 33, "3", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB3->SetBackgroundColour(mmColors::userDefColor3);
    userColourSettingStBoxSizer->Add(m_UDFCB3, g_flagsH);

    m_UDFCB4 = new wxButton(view_panel, wxID_HIGHEST + 44, "4", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB4->SetBackgroundColour(mmColors::userDefColor4);
    userColourSettingStBoxSizer->Add(m_UDFCB4, g_flagsH);

    m_UDFCB5 = new wxButton(view_panel, wxID_HIGHEST + 55, "5", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB5->SetBackgroundColour(mmColors::userDefColor5);
    userColourSettingStBoxSizer->Add(m_UDFCB5, g_flagsH);

    m_UDFCB6 = new wxButton(view_panel, wxID_HIGHEST + 66, "6", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB6->SetBackgroundColour(mmColors::userDefColor6);
    userColourSettingStBoxSizer->Add(m_UDFCB6, g_flagsH);

    m_UDFCB7 = new wxButton(view_panel, wxID_HIGHEST + 77, "7", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB7->SetBackgroundColour(mmColors::userDefColor7);
    userColourSettingStBoxSizer->Add(m_UDFCB7, g_flagsH);

    wxButton* reset = new wxButton(view_panel, wxID_REDO, _("Default"), wxDefaultPosition, wxDefaultSize, 0);
    m_UDFCB7->SetBackgroundColour(mmColors::userDefColor7);
    userColourSettingStBoxSizer->Add(reset, g_flagsH);
    // UI Appearance

    wxStaticBox* iconStaticBox = new wxStaticBox(view_panel, wxID_STATIC, _("UI Appearance"));
    SetBoldFont(iconStaticBox);

    wxStaticBoxSizer* iconStaticBoxSizer = new wxStaticBoxSizer(iconStaticBox, wxVERTICAL);
    viewsPanelSizer->Add(iconStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxFlexGridSizer* view_sizer2 = new wxFlexGridSizer(0, 2, 0, 5);
    iconStaticBoxSizer->Add(view_sizer2);

    // Theme

    m_theme_manager = new wxButton(view_panel, ID_DIALOG_THEMEMANAGER, _("Open Theme Manager"));

    view_sizer2->Add(new wxStaticText(view_panel, wxID_STATIC, _("Style Template")), g_flagsH);
    view_sizer2->Add(m_theme_manager, g_flagsH);

    //
    wxArrayString theme_mode_values;
    theme_mode_values.Add(_("Auto"));
    theme_mode_values.Add(_("Light"));
    theme_mode_values.Add(_("Dark"));

    m_theme_mode = new wxChoice(view_panel, wxID_RESIZE_FRAME, wxDefaultPosition
                        , wxDefaultSize, theme_mode_values);
    mmToolTip(m_theme_mode, _("Specify preferred theme variant to use if supported"));
    m_theme_mode->SetSelection(Option::instance().getThemeMode());
    view_sizer2->Add(new wxStaticText(view_panel, wxID_STATIC, _("Theme Mode")), g_flagsH);
    view_sizer2->Add(m_theme_mode, g_flagsH);

    //
    view_sizer2->Add(new wxStaticText(view_panel, wxID_STATIC, _("HTML Scale Factor")), g_flagsH);

    htmlScaleMax = 300;
    htmlScaleMin = 25;

    m_scale_factor = new wxSpinCtrl(view_panel, ID_DIALOG_HTML_SCALE
        , wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, htmlScaleMin, htmlScaleMax);
#ifdef __WXMAC__ // Workaround for bug https://github.com/wxWidgets/wxWidgets/issues/12968
    m_scale_factor->SetRange(0, htmlScaleMax);
    m_scale_factor->Connect(ID_DIALOG_HTML_SCALE, wxEVT_SPINCTRL
        , wxSpinEventHandler(OptionSettingsView::OnHTMLScaleSpin), nullptr, this);
#endif

    int vFontSize = Option::instance().getHtmlFontSize();
    m_scale_factor->SetValue(vFontSize);
    mmToolTip(m_scale_factor, _("Specify which scale factor is used for the report pages"));
    view_sizer2->Add(m_scale_factor, g_flagsH);

    // Font size
    wxArrayString font_choice;
    font_choice.Add(wxTRANSLATE("Normal"));
    font_choice.Add(wxTRANSLATE("Enlarged"));
    font_choice.Add(wxTRANSLATE("Large"));
    font_choice.Add(wxTRANSLATE("Huge"));

    view_sizer2->Add(new wxStaticText(view_panel, wxID_STATIC, _("Font size")), g_flagsH);
    m_font_size = new wxChoice(view_panel, wxID_RESIZE_FRAME, wxDefaultPosition
        , wxDefaultSize, font_choice);
    m_font_size->SetSelection(Option::instance().getFontSize());
    view_sizer2->Add(m_font_size, g_flagsH);

    // Icons

    wxFlexGridSizer* view_sizer3 = new wxFlexGridSizer(0, 3, 0, 5);
    iconStaticBoxSizer->Add(view_sizer3);

    const wxString settings_choice[] = {
        wxTRANSLATE("Small (16 px)"),
        wxTRANSLATE("Normal (24 px)"),
        wxTRANSLATE("Large (32 px)"),
        wxTRANSLATE("Huge (48 px)")
    };

    m_toolbar_icon_size = new wxChoice(view_panel, wxID_RESIZE_FRAME);
    m_navigation_icon_size = new wxChoice(view_panel, wxID_RESIZE_FRAME);
    m_others_icon_size = new wxChoice(view_panel, wxID_RESIZE_FRAME);

    for (const auto& entry : settings_choice)
    {
        m_toolbar_icon_size->Append(wxGetTranslation(entry), new wxStringClientData(entry));
        m_navigation_icon_size->Append(wxGetTranslation(entry), new wxStringClientData(entry));
        m_others_icon_size->Append(wxGetTranslation(entry), new wxStringClientData(entry));
    }

    view_sizer3->Add(new wxStaticText(view_panel, wxID_STATIC, _("Toolbar Icon Size")), g_flagsH);
    view_sizer3->Add(new wxStaticText(view_panel, wxID_STATIC, _("Navigation Icon Size")), g_flagsH);
    view_sizer3->Add(new wxStaticText(view_panel, wxID_STATIC, _("Others Icon Size")), g_flagsH);

    view_sizer3->Add(m_toolbar_icon_size, g_flagsH);
    view_sizer3->Add(m_navigation_icon_size, g_flagsH);
    view_sizer3->Add(m_others_icon_size, g_flagsH);

    int vIconSize = Option::instance().getToolbarIconSize();
    int selection = vIconSize / 8 - (vIconSize > 32 ? 3 : 2);
    m_toolbar_icon_size->SetSelection(selection);

    vIconSize = Option::instance().getNavigationIconSize();
    selection = vIconSize / 8 - (vIconSize > 32 ? 3 : 2);
    m_navigation_icon_size->SetSelection(selection);

    vIconSize = Option::instance().getIconSize();
    selection = vIconSize / 8 - (vIconSize > 32 ? 3 : 2);
    m_others_icon_size->SetSelection(selection);

    Fit();
    view_panel->SetMinSize(view_panel->GetBestVirtualSize());
    view_panel->SetScrollRate(1, 1);

    this->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(OptionSettingsView::OnNavTreeColorChanged), nullptr, this);
    this->Connect(ID_DIALOG_THEMEMANAGER, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(OptionSettingsView::OnThemeManagerSelected), nullptr, this);
}

//----------------------------------------------------------------------------
// Workaround for bug https://github.com/wxWidgets/wxWidgets/issues/12968
void OptionSettingsView::OnHTMLScaleSpin(wxSpinEvent& event)
{
    if (m_scale_factor->GetValue() < htmlScaleMin)
        m_scale_factor->SetValue(htmlScaleMin);
    event.Skip();
}
void OptionSettingsView::OnNavTreeColorChanged(wxCommandEvent& event)
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

void OptionSettingsView::OnThemeManagerSelected(wxCommandEvent& event)
{
    mmThemesDialog dlg(this);
    dlg.ShowModal();
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

    int themeMode = m_theme_mode->GetSelection();
    Option::instance().setThemeMode(themeMode);
    
    int size = m_scale_factor->GetValue();
    Option::instance().setHTMLFontSizes(size);
    int i[4] = { 16, 24, 32, 48 };
    size = m_others_icon_size->GetSelection();
    size = i[size];
    Option::instance().setIconSize(size);

    size = m_font_size->GetSelection();
    Option::instance().setFontSize(size);

    size = m_navigation_icon_size->GetSelection();
    size = i[size];
    Option::instance().setNavigationIconSize(size);

    size = m_toolbar_icon_size->GetSelection();
    size = i[size];
    Option::instance().setToolbarIconSize(size);

    Option::instance().BudgetFinancialYears(m_budget_financial_years->GetValue());
    Option::instance().BudgetIncludeTransfers(m_budget_include_transfers->GetValue());
    Option::instance().BudgetReportWithSummaries(m_budget_summary_without_category->GetValue());
    Option::instance().setBudgetDaysOffset(m_budget_days_offset->GetValue());
    Option::instance().setReportingFirstDay(m_reporting_firstday->GetValue());
    Option::instance().IgnoreFutureTransactions(m_ignore_future_transactions->GetValue());
    Option::instance().ShowToolTips(m_showToolTips->GetValue());
    Option::instance().ShowMoneyTips(m_showMoneyTips->GetValue());

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
