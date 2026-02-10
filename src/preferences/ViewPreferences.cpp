/*******************************************************
Copyright (C) 2014 Stefano Giorgio
Copyright (C) 2021-2022 Mark Whalley (mark@ipx.co.uk)
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

#include <wx/colordlg.h>

#include "util/util.h"

#include "ViewPreferences.h"

#include "uicontrols/navigatordialog.h"
#include "uicontrols/toolbardialog.h"
#include "images_list.h"
#include "themes.h"

/*******************************************************/
wxBEGIN_EVENT_TABLE(ViewPreferences, wxPanel)
wxEND_EVENT_TABLE()
/*******************************************************/

ViewPreferences::ViewPreferences()
{
}

ViewPreferences::ViewPreferences(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
{
    wxPanel::Create(parent, id, pos, size, style, name);
    Create();
}

ViewPreferences::~ViewPreferences()
{
}

void ViewPreferences::Create()
{
    // mainSizer
    //   panelWindow panelSizer
    //     viewBox viewSizer
    //       viewChoiceSizer
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
    wxStaticBox* viewBox = new wxStaticBox(panelWindow, wxID_STATIC, _t("View"));
    wxStaticBoxSizer* viewSizer = new wxStaticBoxSizer(viewBox, wxVERTICAL);
    panelSizer->Add(viewSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxFlexGridSizer* viewChoiceSizer = new wxFlexGridSizer(0, 2, 0, 5);
    viewSizer->Add(viewChoiceSizer);

    viewChoiceSizer->Add(new wxStaticText(viewBox, wxID_STATIC, _t("Accounts Visible")), g_flagsH);
    const wxString vAccts = SettingModel::instance().getViewAccounts();
    wxArrayString view_accounts;
    view_accounts.Add(VIEW_ACCOUNTS_ALL_STR);
    view_accounts.Add(VIEW_ACCOUNTS_OPEN_STR);
    view_accounts.Add(VIEW_ACCOUNTS_FAVORITES_STR);

    m_choice_visible = new wxChoice(viewBox, wxID_ANY);
    for (const auto& entry : view_accounts) {
        m_choice_visible->Append(wxGetTranslation(entry), new wxStringClientData(entry));
        if (entry == vAccts)
            m_choice_visible->SetStringSelection(wxGetTranslation(entry));
    }
    mmToolTip(m_choice_visible, _t("Specify which accounts are visible"));
    viewChoiceSizer->Add(m_choice_visible, g_flagsH);

    viewChoiceSizer->Add(new wxStaticText(viewBox, wxID_STATIC, _t("Category Delimiter")), g_flagsH);

    m_categ_delimiter_list = new wxComboBox(viewBox, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, {":", ": ", " : ", "-"});
    m_categ_delimiter_list->SetValue(InfotableModel::instance().getString("CATEG_DELIMITER",":"));
    viewChoiceSizer->Add(m_categ_delimiter_list, g_flagsH);

    // Toolbar Appearance
    viewChoiceSizer->Add(new wxStaticText(viewBox, wxID_STATIC, _t("Toolbar")), g_flagsH);
    viewChoiceSizer->Add(new wxButton(viewBox, ID_DIALOG_TOOLBAR_CONFIG, _t("Edit")), g_flagsH);

    // Navigator Appearance
    viewChoiceSizer->Add(new wxStaticText(viewBox, wxID_STATIC, _t("Navigator and account types")), g_flagsH);
    viewChoiceSizer->Add(new wxButton(viewBox, ID_DIALOG_NAVIGATOR_CONFIG, _t("Edit")), g_flagsH);

    viewChoiceSizer->Add(new wxStaticText(viewBox, wxID_STATIC, " "), g_flagsH);  //Placeholder
    m_navShowCashLedger = new wxCheckBox(viewBox, wxID_STATIC, _t("Show cash ledger for portfolios"));
    m_navShowCashLedger->SetValue(PreferencesModel::instance().getShowNavigatorCashLedger());
    viewChoiceSizer->Add(m_navShowCashLedger, g_flagsH);

    viewChoiceSizer->AddSpacer(10);
    viewChoiceSizer->AddSpacer(10);

    viewChoiceSizer->Add(new wxStaticText(viewBox, wxID_STATIC, _t("Transactions")), g_flagsH);
    m_doNotColorFuture = new wxCheckBox(viewBox, wxID_STATIC, _t("Do not use color for future transactions"));
    m_doNotColorFuture->SetValue(PreferencesModel::instance().getDoNotColorFuture());
    viewChoiceSizer->Add(m_doNotColorFuture, g_flagsH);

    viewChoiceSizer->Add(new wxStaticText(viewBox, wxID_STATIC, " "), g_flagsH);  //Placeholder
    m_doSpecialColorReconciled = new wxCheckBox(viewBox, wxID_STATIC, _t("Emphasize not reconciled transactions"));
    m_doSpecialColorReconciled->SetValue(PreferencesModel::instance().getDoSpecialColorReconciled());
    viewChoiceSizer->Add(m_doSpecialColorReconciled, g_flagsV);

    viewSizer->AddSpacer(10);

    m_showToolTips = new wxCheckBox(viewBox, wxID_STATIC, _t("Show Tooltips"));
    m_showToolTips->SetValue(PreferencesModel::instance().getShowToolTips());
    viewSizer->Add(m_showToolTips, g_flagsV);

    m_showMoneyTips = new wxCheckBox(viewBox, wxID_ANY, _t("Show Money Tips"));
    m_showMoneyTips->SetValue(PreferencesModel::instance().getShowMoneyTips());
    viewSizer->Add(m_showMoneyTips, g_flagsV);

    // User Interface (UI) Appearance
    wxStaticBox* uiBox = new wxStaticBox(panelWindow, wxID_STATIC, _t("User Interface"));
    wxStaticBoxSizer* uiSizer = new wxStaticBoxSizer(uiBox, wxVERTICAL);
    panelSizer->Add(uiSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxFlexGridSizer* uiStyleSizer = new wxFlexGridSizer(0, 2, 0, 5);
    uiSizer->Add(uiStyleSizer);

    uiStyleSizer->Add(new wxStaticText(uiBox, wxID_STATIC, _t("Style Template")), g_flagsH);
    m_theme_manager = new wxButton(uiBox, ID_DIALOG_THEMEMANAGER, _t("Open Theme Manager"));
    uiStyleSizer->Add(m_theme_manager, g_flagsH);

    uiStyleSizer->Add(new wxStaticText(uiBox, wxID_STATIC, _t("Theme")), g_flagsH);
    wxArrayString theme_mode_values;
    theme_mode_values.Add(_t("System"));
    theme_mode_values.Add(_t("Light"));
    theme_mode_values.Add(_t("Dark"));

    m_theme_mode = new wxChoice(
        uiBox, wxID_RESIZE_FRAME, wxDefaultPosition, wxDefaultSize,
        theme_mode_values
    );
    mmToolTip(m_theme_mode, _t("Specify preferred theme variant to use if supported"));
    m_theme_mode->SetSelection(PreferencesModel::instance().getThemeMode());
    uiStyleSizer->Add(m_theme_mode, g_flagsH);

    uiStyleSizer->Add(new wxStaticText(uiBox, wxID_STATIC, _t("HTML Scale Factor")), g_flagsH);
    htmlScaleMax = 300;
    htmlScaleMin = 25;
    m_scale_factor = new wxSpinCtrl(
        uiBox, ID_DIALOG_HTML_SCALE, wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxSP_ARROW_KEYS, htmlScaleMin, htmlScaleMax
    );
#ifdef __WXMAC__ // Workaround for bug https://github.com/wxWidgets/wxWidgets/issues/12968
    m_scale_factor->SetRange(0, htmlScaleMax);
    m_scale_factor->Connect(
        ID_DIALOG_HTML_SCALE, wxEVT_SPINCTRL,
        wxSpinEventHandler(ViewPreferences::OnHTMLScaleSpin),
        nullptr, this
    );
#endif

    int vFontSize = PreferencesModel::instance().getHtmlScale();
    m_scale_factor->SetValue(vFontSize);
    mmToolTip(m_scale_factor, _t("Specify which scale factor is used for the report pages"));
    uiStyleSizer->Add(m_scale_factor, g_flagsH);

    uiStyleSizer->Add(new wxStaticText(uiBox, wxID_STATIC, _t("Font Size")), g_flagsH);
    wxArrayString font_choice;
    font_choice.Add(_t("Normal"));
    font_choice.Add(_t("Enlarged"));
    font_choice.Add(_t("Large"));
    font_choice.Add(_t("Huge"));

    m_font_size_chooser = new wxChoice(
        uiBox, wxID_RESIZE_FRAME, wxDefaultPosition, wxDefaultSize,
        font_choice
    );
    m_font_size_chooser->SetSelection(PreferencesModel::instance().getFontSize());
    uiStyleSizer->Add(m_font_size_chooser, g_flagsH);

    // Icons
    wxFlexGridSizer* uiIconSizer = new wxFlexGridSizer(0, 3, 0, 5);
    uiSizer->Add(uiIconSizer);

    uiIconSizer->Add(new wxStaticText(uiBox, wxID_STATIC, _t("Toolbar Icon Size")), g_flagsH);
    uiIconSizer->Add(new wxStaticText(uiBox, wxID_STATIC, _t("Navigator Icon Size")), g_flagsH);
    uiIconSizer->Add(new wxStaticText(uiBox, wxID_STATIC, _t("Others Icon Size")), g_flagsH);

    const wxString settings_choice[] = {
        _n("Small (16 px)"),
        _n("Normal (24 px)"),
        _n("Large (32 px)"),
        _n("Huge (48 px)")
    };

    m_toolbar_icon_size = new wxChoice(uiBox, wxID_RESIZE_FRAME);
    m_navigation_icon_size = new wxChoice(uiBox, wxID_RESIZE_FRAME);
    m_others_icon_size = new wxChoice(uiBox, wxID_RESIZE_FRAME);

    for (const auto& entry : settings_choice) {
        m_toolbar_icon_size->Append(wxGetTranslation(entry), new wxStringClientData(entry));
        m_navigation_icon_size->Append(wxGetTranslation(entry), new wxStringClientData(entry));
        m_others_icon_size->Append(wxGetTranslation(entry), new wxStringClientData(entry));
    }

    int vIconSize = PreferencesModel::instance().getToolbarIconSize();
    int selection = vIconSize / 8 - (vIconSize > 32 ? 3 : 2);
    m_toolbar_icon_size->SetSelection(selection);

    vIconSize = PreferencesModel::instance().getNavigationIconSize();
    selection = vIconSize / 8 - (vIconSize > 32 ? 3 : 2);
    m_navigation_icon_size->SetSelection(selection);

    vIconSize = PreferencesModel::instance().getIconSize();
    selection = vIconSize / 8 - (vIconSize > 32 ? 3 : 2);
    m_others_icon_size->SetSelection(selection);

    uiIconSizer->Add(m_toolbar_icon_size, g_flagsH);
    uiIconSizer->Add(m_navigation_icon_size, g_flagsH);
    uiIconSizer->Add(m_others_icon_size, g_flagsH);

    // Navigator Appearance
    /*
    wxStaticBox* navBox = new wxStaticBox(panelWindow, wxID_STATIC, _t("Navigator display / Account types"));

    wxStaticBoxSizer* navSizer = new wxStaticBoxSizer(navBox, wxVERTICAL);
    panelSizer->Add(navSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    m_navigator_cfg = new wxButton(navBox, ID_DIALOG_NAVIGATOR_CONFIG, _t("Edit navigator entries and account types"));
    navSizer->Add(m_navigator_cfg, g_flagsV);

    m_navShowCashLedger = new wxCheckBox(navBox, wxID_STATIC, _t("Show cash ledger for portfolios"));
    m_navShowCashLedger->SetValue(PreferencesModel::instance().getShowNavigatorCashLedger());
    navSizer->Add(m_navShowCashLedger, g_flagsV);*/

    SetBoldFontToStaticBoxHeader(viewBox);
    SetBoldFontToStaticBoxHeader(uiBox);
    //SetBoldFontToStaticBoxHeader(navBox);

    // -------------------
    Fit();
    panelWindow->SetMinSize(panelWindow->GetBestVirtualSize());
    panelWindow->SetScrollRate(6, 6);

    this->Connect(
        ID_DIALOG_THEMEMANAGER, wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(ViewPreferences::OnThemeManagerSelected),
        nullptr, this
    );
    this->Connect(
        ID_DIALOG_NAVIGATOR_CONFIG, wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(ViewPreferences::OnNavigationConfigSelected),
        nullptr, this
    );
    this->Connect(
        ID_DIALOG_TOOLBAR_CONFIG, wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(ViewPreferences::OnToolbarConfigSelected),
        nullptr, this
    );
}

//----------------------------------------------------------------------------
// Workaround for bug https://github.com/wxWidgets/wxWidgets/issues/12968
void ViewPreferences::OnHTMLScaleSpin(wxSpinEvent& event)
{
    if (m_scale_factor->GetValue() < htmlScaleMin)
        m_scale_factor->SetValue(htmlScaleMin);
    event.Skip();
}

void ViewPreferences::OnThemeManagerSelected(wxCommandEvent&)
{
    mmThemesDialog dlg(this);
    dlg.ShowModal();
}

void ViewPreferences::OnNavigationConfigSelected(wxCommandEvent&)
{
    mmNavigatorDialog dlg(this);
    dlg.ShowModal();
}

void ViewPreferences::OnToolbarConfigSelected(wxCommandEvent&)
{
    mmToolbarDialog dlg(this);
    dlg.ShowModal();
}


bool ViewPreferences::SaveSettings()
{
    auto delimiter = m_categ_delimiter_list->GetValue();
    if (delimiter.empty()) delimiter = ":";
    InfotableModel::instance().setString("CATEG_DELIMITER", delimiter);

    wxString accVisible = VIEW_ACCOUNTS_ALL_STR;
    wxStringClientData* visible_acc_obj = static_cast<wxStringClientData*>(m_choice_visible->GetClientObject(m_choice_visible->GetSelection()));
    if (visible_acc_obj)
        accVisible = visible_acc_obj->GetData();
    SettingModel::instance().setViewAccounts(accVisible);

    int themeMode = m_theme_mode->GetSelection();
    PreferencesModel::instance().setThemeMode(themeMode);

    int size = m_scale_factor->GetValue();
    PreferencesModel::instance().setHtmlScale(size);
    int i[4] = { 16, 24, 32, 48 };
    size = m_others_icon_size->GetSelection();
    size = i[size];
    PreferencesModel::instance().setIconSize(size);

    size = m_font_size_chooser->GetSelection();
    PreferencesModel::instance().setFontSize(size);

    size = m_navigation_icon_size->GetSelection();
    size = i[size];
    PreferencesModel::instance().setNavigationIconSize(size);

    size = m_toolbar_icon_size->GetSelection();
    size = i[size];
    PreferencesModel::instance().setToolbarIconSize(size);

    PreferencesModel::instance().setDoNotColorFuture(m_doNotColorFuture->GetValue());
    PreferencesModel::instance().setDoSpecialColorReconciled(m_doSpecialColorReconciled->GetValue());

    PreferencesModel::instance().setShowToolTips(m_showToolTips->GetValue());
    PreferencesModel::instance().setShowMoneyTips(m_showMoneyTips->GetValue());

    PreferencesModel::instance().setShowNavigatorCashLedger(m_navShowCashLedger->GetValue());

    return true;
}
