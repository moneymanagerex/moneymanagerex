/*******************************************************
Copyright (C) 2014 Stefano Giorgio
Copyright (C) 2021-2022 Mark Whalley (mark@ipx.co.uk)

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
    // mainSizer
    //   panelWindow panelSizer
    //     viewBox viewSizer
    //       viewChoiceSizer
    //     trxBox trxSizer
    //       trxChoiceSizer
    //     colorsBox colorsSizer
    //     uiBox uiSizer
    //       uiStyleSizer
    //       uiIconSizer

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(mainSizer);

    wxScrolledWindow* panelWindow = new wxScrolledWindow(this, wxID_ANY);
    wxBoxSizer* panelSizer = new wxBoxSizer(wxVERTICAL);
    panelWindow->SetSizer(panelSizer);
    mainSizer->Add(panelWindow, wxSizerFlags(g_flagsExpand).Proportion(0));

    // View Options
    wxStaticBox* viewBox = new wxStaticBox(panelWindow, wxID_STATIC, _("View"));
    SetBoldFont(viewBox);
    wxStaticBoxSizer* viewSizer = new wxStaticBoxSizer(viewBox, wxVERTICAL);
    panelSizer->Add(viewSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxFlexGridSizer* viewChoiceSizer = new wxFlexGridSizer(0, 2, 0, 5);
    viewSizer->Add(viewChoiceSizer);

    viewChoiceSizer->Add(new wxStaticText(panelWindow, wxID_STATIC, _("Accounts Visible")), g_flagsH);
    const wxString vAccts = Model_Setting::instance().GetViewAccounts();
    wxArrayString view_accounts;
    view_accounts.Add(VIEW_ACCOUNTS_ALL_STR);
    view_accounts.Add(VIEW_ACCOUNTS_OPEN_STR);
    view_accounts.Add(VIEW_ACCOUNTS_FAVORITES_STR);

    m_choice_visible = new wxChoice(panelWindow, wxID_ANY);
    for (const auto& entry : view_accounts) {
        m_choice_visible->Append(wxGetTranslation(entry), new wxStringClientData(entry));
        if (entry == vAccts)
            m_choice_visible->SetStringSelection(wxGetTranslation(entry));
    }
    mmToolTip(m_choice_visible, _("Specify which accounts are visible"));
    viewChoiceSizer->Add(m_choice_visible, g_flagsH);

    viewChoiceSizer->Add(new wxStaticText(panelWindow, wxID_STATIC, _("Category Delimiter")), g_flagsH);
    wxArrayString list;
    list.Add(":");
    list.Add(": ");
    list.Add(" : ");

    wxString delimiter = Model_Infotable::instance().GetStringInfo("CATEG_DELIMITER",":");
    m_categ_delimiter_list = new wxComboBox(
        panelWindow, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
        list
    );
    m_categ_delimiter_list->SetValue(delimiter);
    viewChoiceSizer->Add(m_categ_delimiter_list, g_flagsH);

    m_showToolTips = new wxCheckBox(
        panelWindow, wxID_STATIC,
        _("Show Tooltips"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    m_showToolTips->SetValue(Option::instance().getShowToolTips());
    viewChoiceSizer->Add(m_showToolTips, g_flagsH);

    m_showMoneyTips = new wxCheckBox(
        panelWindow, wxID_STATIC,
        _("Show Money Tips"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    m_showMoneyTips->SetValue(Option::instance().getShowMoneyTips());
    viewChoiceSizer->Add(m_showMoneyTips, g_flagsH);

    // Transaction/Budget options
    wxStaticBox* trxBox = new wxStaticBox(panelWindow, wxID_STATIC, _("Transaction/Budget"));
    SetBoldFont(trxBox);
    wxStaticBoxSizer* trxSizer = new wxStaticBoxSizer(trxBox, wxVERTICAL);
    panelSizer->Add(trxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_budget_financial_years = new wxCheckBox(
        panelWindow, wxID_STATIC,
        _("View Budgets as Financial Years"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    m_budget_financial_years->SetValue(Option::instance().getBudgetFinancialYears());
    trxSizer->Add(m_budget_financial_years, g_flagsV);

    m_budget_include_transfers = new wxCheckBox(
        panelWindow, wxID_STATIC,
        _("View Budgets with 'transfer' transactions"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    m_budget_include_transfers->SetValue(Option::instance().getBudgetIncludeTransfers());
    trxSizer->Add(m_budget_include_transfers, g_flagsV);

    m_budget_summary_without_category = new wxCheckBox(
        panelWindow, wxID_STATIC,
        _("View Budget Category Report with Summaries"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    m_budget_summary_without_category->SetValue(Option::instance().getBudgetSummaryWithoutCategories());
    trxSizer->Add(m_budget_summary_without_category, g_flagsV);

    // Budget Yearly/Monthly relationship if both exist
    m_budget_override = new wxCheckBox(
        panelWindow, wxID_STATIC,
        _("Override yearly budget with monthly budget"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    mmToolTip(m_budget_override, _("If monthly budget exists then use this to override the yearly budget; otherwise combine them"));
    m_budget_override->SetValue(Option::instance().getBudgetOverride());
    trxSizer->Add(m_budget_override, g_flagsV);

    // Option to deduct monthly budget from yearly budget for reporting
    m_budget_deduct_monthly = new wxCheckBox(
        panelWindow, wxID_STATIC,
        _("Subtract monthly budgets from yearly budget in reporting"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    mmToolTip(m_budget_deduct_monthly, _("Yearly budget will be reduced by the amount budgeted monthly.\nTotal estimate for the year will be reported as either the yearly budget OR the sum of the monthly budgets, whichever is greater."));
    m_budget_deduct_monthly->SetValue(Option::instance().getBudgetDeductMonthly());
    trxSizer->Add(m_budget_deduct_monthly, g_flagsV);

    wxFlexGridSizer* trxChoiceSizer = new wxFlexGridSizer(0, 2, 0, 5);
    trxSizer->Add(trxChoiceSizer);

    trxChoiceSizer->Add(new wxStaticText(panelWindow, wxID_STATIC, _("Budget Offset (days)")), g_flagsH);
    m_budget_days_offset = new wxSpinCtrl(
        panelWindow, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxSP_ARROW_KEYS, -30, +30
    );
    mmToolTip(m_budget_days_offset, _("Adjusts the first day of month (normally 1st) for budget calculations"));
    m_budget_days_offset->SetValue(Option::instance().getBudgetDaysOffset());
    trxChoiceSizer->Add(m_budget_days_offset, g_flagsH);

    trxChoiceSizer->Add(new wxStaticText(panelWindow, wxID_STATIC, _("First Day of Month")), g_flagsH);
    m_reporting_firstday = new wxSpinCtrl(
        panelWindow, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxSP_ARROW_KEYS, 1, 28
    );
    mmToolTip(m_reporting_firstday, _("Adjusts the first day of month for reporting"));
    m_reporting_firstday->SetValue(Option::instance().getReportingFirstDay());
    trxChoiceSizer->Add(m_reporting_firstday, g_flagsH);

    trxChoiceSizer->Add(new wxStaticText(panelWindow, wxID_STATIC, _("First Weekday")), g_flagsH);
    m_reporting_first_weekday = new wxChoice(panelWindow, wxID_ANY);
    m_reporting_first_weekday->Append(wxGetTranslation(g_days_of_week[0]));
    m_reporting_first_weekday->Append(wxGetTranslation(g_days_of_week[1]));
    m_reporting_first_weekday->SetSelection(Option::instance().getReportingFirstWeekday());
    mmToolTip(m_reporting_first_weekday, _("Adjusts the first day of week for filtering and reporting"));
    trxChoiceSizer->Add(m_reporting_first_weekday, g_flagsH);

    m_ignore_future_transactions = new wxCheckBox(
        panelWindow, wxID_STATIC,
        _("Ignore Future Transactions"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    m_ignore_future_transactions->SetValue(Option::instance().getIgnoreFutureTransactions());
    trxSizer->Add(m_ignore_future_transactions, g_flagsV);

    m_use_trans_date_time = new wxCheckBox(panelWindow, wxID_ANY, _("Use 'Time' in transaction recording/reporting"));
    m_use_trans_date_time->SetValue(Option::instance().UseTransDateTime());
    trxSizer->Add(m_use_trans_date_time, g_flagsV);

    // Colours settings
    wxStaticBox* colorsBox = new wxStaticBox(panelWindow, wxID_ANY, _("Transaction Colors"));
    SetBoldFont(colorsBox);
    wxStaticBoxSizer* colorsSizer = new wxStaticBoxSizer(colorsBox, wxHORIZONTAL);
    panelSizer->Add(colorsSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    int size_x = 30;
    m_UDFCB1 = new wxButton(panelWindow, wxID_HIGHEST + 11, "1", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB1->SetBackgroundColour(mmColors::userDefColor1);
    colorsSizer->Add(m_UDFCB1, g_flagsH);

    m_UDFCB2 = new wxButton(panelWindow, wxID_HIGHEST + 22, "2", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB2->SetBackgroundColour(mmColors::userDefColor2);
    colorsSizer->Add(m_UDFCB2, g_flagsH);

    m_UDFCB3 = new wxButton(panelWindow, wxID_HIGHEST + 33, "3", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB3->SetBackgroundColour(mmColors::userDefColor3);
    colorsSizer->Add(m_UDFCB3, g_flagsH);

    m_UDFCB4 = new wxButton(panelWindow, wxID_HIGHEST + 44, "4", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB4->SetBackgroundColour(mmColors::userDefColor4);
    colorsSizer->Add(m_UDFCB4, g_flagsH);

    m_UDFCB5 = new wxButton(panelWindow, wxID_HIGHEST + 55, "5", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB5->SetBackgroundColour(mmColors::userDefColor5);
    colorsSizer->Add(m_UDFCB5, g_flagsH);

    m_UDFCB6 = new wxButton(panelWindow, wxID_HIGHEST + 66, "6", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB6->SetBackgroundColour(mmColors::userDefColor6);
    colorsSizer->Add(m_UDFCB6, g_flagsH);

    m_UDFCB7 = new wxButton(panelWindow, wxID_HIGHEST + 77, "7", wxDefaultPosition, wxSize(size_x, -1), 0);
    m_UDFCB7->SetBackgroundColour(mmColors::userDefColor7);
    colorsSizer->Add(m_UDFCB7, g_flagsH);

    wxButton* reset = new wxButton(panelWindow, wxID_REDO, _("Default"), wxDefaultPosition, wxDefaultSize, 0);
    m_UDFCB7->SetBackgroundColour(mmColors::userDefColor7);
    colorsSizer->Add(reset, g_flagsH);

    // User Interface (UI) Appearance
    wxStaticBox* uiBox = new wxStaticBox(panelWindow, wxID_STATIC, _("User Interface"));
    SetBoldFont(uiBox);
    wxStaticBoxSizer* uiSizer = new wxStaticBoxSizer(uiBox, wxVERTICAL);
    panelSizer->Add(uiSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxFlexGridSizer* uiStyleSizer = new wxFlexGridSizer(0, 2, 0, 5);
    uiSizer->Add(uiStyleSizer);

    uiStyleSizer->Add(new wxStaticText(panelWindow, wxID_STATIC, _("Style Template")), g_flagsH);
    m_theme_manager = new wxButton(panelWindow, ID_DIALOG_THEMEMANAGER, _("Open Theme Manager"));
    uiStyleSizer->Add(m_theme_manager, g_flagsH);

    uiStyleSizer->Add(new wxStaticText(panelWindow, wxID_STATIC, _("Theme")), g_flagsH);
    wxArrayString theme_mode_values;
    theme_mode_values.Add(_("System"));
    theme_mode_values.Add(_("Light"));
    theme_mode_values.Add(_("Dark"));

    m_theme_mode = new wxChoice(
        panelWindow, wxID_RESIZE_FRAME, wxDefaultPosition, wxDefaultSize,
        theme_mode_values
    );
    mmToolTip(m_theme_mode, _("Specify preferred theme variant to use if supported"));
    m_theme_mode->SetSelection(Option::instance().getThemeMode());
    uiStyleSizer->Add(m_theme_mode, g_flagsH);

    uiStyleSizer->Add(new wxStaticText(panelWindow, wxID_STATIC, _("HTML Scale Factor")), g_flagsH);
    htmlScaleMax = 300;
    htmlScaleMin = 25;
    m_scale_factor = new wxSpinCtrl(
        panelWindow, ID_DIALOG_HTML_SCALE, wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxSP_ARROW_KEYS, htmlScaleMin, htmlScaleMax
    );
#ifdef __WXMAC__ // Workaround for bug https://github.com/wxWidgets/wxWidgets/issues/12968
    m_scale_factor->SetRange(0, htmlScaleMax);
    m_scale_factor->Connect(
        ID_DIALOG_HTML_SCALE, wxEVT_SPINCTRL,
        wxSpinEventHandler(OptionSettingsView::OnHTMLScaleSpin),
        nullptr, this
    );
#endif

    int vFontSize = Option::instance().getHtmlScale();
    m_scale_factor->SetValue(vFontSize);
    mmToolTip(m_scale_factor, _("Specify which scale factor is used for the report pages"));
    uiStyleSizer->Add(m_scale_factor, g_flagsH);

    uiStyleSizer->Add(new wxStaticText(panelWindow, wxID_STATIC, _("Font Size")), g_flagsH);
    wxArrayString font_choice;
    font_choice.Add(_("Normal"));
    font_choice.Add(_("Enlarged"));
    font_choice.Add(_("Large"));
    font_choice.Add(_("Huge"));

    m_font_size_chooser = new wxChoice(
        panelWindow, wxID_RESIZE_FRAME, wxDefaultPosition, wxDefaultSize,
        font_choice
    );
    m_font_size_chooser->SetSelection(Option::instance().getFontSize());
    uiStyleSizer->Add(m_font_size_chooser, g_flagsH);

    // Icons
    wxFlexGridSizer* uiIconSizer = new wxFlexGridSizer(0, 3, 0, 5);
    uiSizer->Add(uiIconSizer);

    uiIconSizer->Add(new wxStaticText(panelWindow, wxID_STATIC, _("Toolbar Icon Size")), g_flagsH);
    uiIconSizer->Add(new wxStaticText(panelWindow, wxID_STATIC, _("Navigator Icon Size")), g_flagsH);
    uiIconSizer->Add(new wxStaticText(panelWindow, wxID_STATIC, _("Others Icon Size")), g_flagsH);

    const wxString settings_choice[] = {
        wxTRANSLATE("Small (16 px)"),
        wxTRANSLATE("Normal (24 px)"),
        wxTRANSLATE("Large (32 px)"),
        wxTRANSLATE("Huge (48 px)")
    };

    m_toolbar_icon_size = new wxChoice(panelWindow, wxID_RESIZE_FRAME);
    m_navigation_icon_size = new wxChoice(panelWindow, wxID_RESIZE_FRAME);
    m_others_icon_size = new wxChoice(panelWindow, wxID_RESIZE_FRAME);

    for (const auto& entry : settings_choice) {
        m_toolbar_icon_size->Append(wxGetTranslation(entry), new wxStringClientData(entry));
        m_navigation_icon_size->Append(wxGetTranslation(entry), new wxStringClientData(entry));
        m_others_icon_size->Append(wxGetTranslation(entry), new wxStringClientData(entry));
    }

    int vIconSize = Option::instance().getToolbarIconSize();
    int selection = vIconSize / 8 - (vIconSize > 32 ? 3 : 2);
    m_toolbar_icon_size->SetSelection(selection);

    vIconSize = Option::instance().getNavigationIconSize();
    selection = vIconSize / 8 - (vIconSize > 32 ? 3 : 2);
    m_navigation_icon_size->SetSelection(selection);

    vIconSize = Option::instance().getIconSize();
    selection = vIconSize / 8 - (vIconSize > 32 ? 3 : 2);
    m_others_icon_size->SetSelection(selection);

    uiIconSizer->Add(m_toolbar_icon_size, g_flagsH);
    uiIconSizer->Add(m_navigation_icon_size, g_flagsH);
    uiIconSizer->Add(m_others_icon_size, g_flagsH);

    Fit();
    panelWindow->SetMinSize(panelWindow->GetBestVirtualSize());
    panelWindow->SetScrollRate(6, 6);

    this->Connect(
        wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(OptionSettingsView::OnNavTreeColorChanged),
        nullptr, this
    );
    this->Connect(
        ID_DIALOG_THEMEMANAGER, wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(OptionSettingsView::OnThemeManagerSelected),
        nullptr, this
    );
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

void OptionSettingsView::OnThemeManagerSelected(wxCommandEvent&)
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
    Option::instance().setHtmlScale(size);
    int i[4] = { 16, 24, 32, 48 };
    size = m_others_icon_size->GetSelection();
    size = i[size];
    Option::instance().setIconSize(size);

    size = m_font_size_chooser->GetSelection();
    Option::instance().setFontSize(size);

    size = m_navigation_icon_size->GetSelection();
    size = i[size];
    Option::instance().setNavigationIconSize(size);

    size = m_toolbar_icon_size->GetSelection();
    size = i[size];
    Option::instance().setToolbarIconSize(size);

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
    Option::instance().setShowToolTips(m_showToolTips->GetValue());
    Option::instance().setShowMoneyTips(m_showMoneyTips->GetValue());
    Option::instance().UseTransDateTime(m_use_trans_date_time->GetValue());

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
