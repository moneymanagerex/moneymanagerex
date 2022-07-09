/*******************************************************
Copyright (C) 2014 Stefano Giorgio
Copyright (C) 2020 - 2022 Nikolay Akimov

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

#include "optionsettingsmisc.h"
#include "option.h"
#include "util.h"

#include "model/Model_Checking.h"

#include <wx/spinctrl.h>

/*******************************************************/
wxBEGIN_EVENT_TABLE(OptionSettingsMisc, wxPanel)
    EVT_CHECKBOX(ID_DIALOG_OPTIONS_CHK_BACKUP, OptionSettingsMisc::OnBackupChanged)
    EVT_CHECKBOX(ID_DIALOG_OPTIONS_CHK_BACKUP_UPDATE, OptionSettingsMisc::OnBackupChanged)
wxEND_EVENT_TABLE()
/*******************************************************/

OptionSettingsMisc::OptionSettingsMisc()
{
}

OptionSettingsMisc::OptionSettingsMisc(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
{
    wxPanel::Create(parent, id, pos, size, style, name);
    Create();
}

OptionSettingsMisc::~OptionSettingsMisc()
{
}

void OptionSettingsMisc::Create()
{
    wxBoxSizer* othersPanelSizer0 = new wxBoxSizer(wxVERTICAL);
    SetSizer(othersPanelSizer0);

    wxScrolledWindow* misc_panel = new wxScrolledWindow(this, wxID_ANY);

    wxBoxSizer* othersPanelSizer = new wxBoxSizer(wxVERTICAL);
    misc_panel->SetSizer(othersPanelSizer);
    othersPanelSizer0->Add(misc_panel, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxStaticText* itemStaticTextURL = new wxStaticText(misc_panel, wxID_STATIC, _("Stock Quote Web Page"));
    SetBoldFont(itemStaticTextURL);

    othersPanelSizer->Add(itemStaticTextURL, g_flagsV);

    wxArrayString list;
    list.Add(mmex::weblink::DefStockUrl);
    //list.Add("https://www.morningstar.com/stocks/xnas/%s/quote");
    //list.Add("https://www.marketwatch.com/investing/stock/%s");
    //list.Add("https://www.ifcmarkets.co.in/en/market-data/stocks-prices/%s");

    wxString stockURL = Model_Infotable::instance().GetStringInfo("STOCKURL", mmex::weblink::DefStockUrl);
    wxComboBox* itemListOfURL = new wxComboBox(misc_panel, ID_DIALOG_OPTIONS_TEXTCTRL_STOCKURL, ""
        , wxDefaultPosition, wxDefaultSize, list);
    itemListOfURL->SetValue(stockURL);

    othersPanelSizer->Add(itemListOfURL, wxSizerFlags(g_flagsExpand).Proportion(0));
    mmToolTip(itemListOfURL, _("Clear the field to Reset the value to system default."));

    // Share Precision
    wxFlexGridSizer* share_precision_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    share_precision_sizer->Add(new wxStaticText(misc_panel, wxID_STATIC, _("Share Precision")), wxSizerFlags(g_flagsExpand).Proportion(0));

    m_share_precision = new wxSpinCtrl(misc_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize
        , wxSP_ARROW_KEYS, 2, 10, Option::instance().SharePrecision());
    m_share_precision->SetValue(Option::instance().SharePrecision());
    mmToolTip(m_share_precision, _("Set the precision for Share prices"));

    share_precision_sizer->Add(m_share_precision, wxSizerFlags(g_flagsExpand).Proportion(0));
    othersPanelSizer->Add(share_precision_sizer, g_flagsBorder1V);

    // New transaction dialog settings
    wxStaticBox* transSettingsStaticBox = new wxStaticBox(misc_panel, wxID_STATIC, _("New Transaction Dialog Settings"));
    SetBoldFont(transSettingsStaticBox);
    
    wxStaticBoxSizer* transSettingsStaticBoxSizer = new wxStaticBoxSizer(transSettingsStaticBox, wxVERTICAL);
    othersPanelSizer->Add(transSettingsStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxArrayString default_values;
    default_values.Add(_("None"));
    default_values.Add(_("Last Used"));

    wxChoice* defaultCategoryTransferChoice = new wxChoice(misc_panel
        , ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY_TRANSFER
        , wxDefaultPosition, wxDefaultSize, default_values);
    defaultCategoryTransferChoice->SetSelection(Option::instance().TransCategorySelectionTransfer());

    wxChoice* defaultDateChoice = new wxChoice(misc_panel
        , ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_DATE
        , wxDefaultPosition, wxDefaultSize, default_values);
    defaultDateChoice->SetSelection(Option::instance().TransDateDefault());

    default_values.Add(_("Unused"));
    wxChoice* defaultPayeeChoice = new wxChoice(misc_panel
        , ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_PAYEE
        , wxDefaultPosition, wxDefaultSize, default_values);
    defaultPayeeChoice->SetSelection(Option::instance().TransPayeeSelection());

    default_values[1] = (_("Last used for payee"));
    default_values.Add(_("Use default for payee"));
    wxChoice* defaultCategoryNonTransferChoice = new wxChoice(misc_panel
        , ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY_NONTRANSFER
        , wxDefaultPosition, wxDefaultSize, default_values);
    defaultCategoryNonTransferChoice->SetSelection(Option::instance().TransCategorySelectionNonTransfer());

    wxChoice* default_status = new wxChoice(misc_panel
        , ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_STATUS);

    for (const auto& i : Model_Checking::all_status())
        default_status->Append(wxGetTranslation(i), new wxStringClientData(i));

    default_status->SetSelection(Option::instance().TransStatusReconciled());

    wxArrayString true_false;
    true_false.Add(wxTRANSLATE("Yes"));
    true_false.Add(wxTRANSLATE("No"));
    wxChoice* bulk_enter = new wxChoice(misc_panel, ID_DIALOG_OPTIONS_BULK_ENTER);
    for (const auto& i : true_false)
        bulk_enter->Append(wxGetTranslation(i), new wxStringClientData(i));
    bulk_enter->SetSelection(Option::instance().get_bulk_transactions() ? 0 : 1);

    wxFlexGridSizer* newTransflexGridSizer = new wxFlexGridSizer(0, 2, 0, 0);
    newTransflexGridSizer->AddGrowableCol(1, 0);
    transSettingsStaticBoxSizer->Add(newTransflexGridSizer);
    newTransflexGridSizer->Add(new wxStaticText(misc_panel, wxID_STATIC, _("Default Date:")), g_flagsH);
    newTransflexGridSizer->Add(defaultDateChoice, g_flagsExpand);
    newTransflexGridSizer->Add(new wxStaticText(misc_panel, wxID_STATIC, _("Default Payee:")), g_flagsH);
    newTransflexGridSizer->Add(defaultPayeeChoice, g_flagsExpand);
    newTransflexGridSizer->Add(new wxStaticText(misc_panel, wxID_STATIC, _("Default Deposit/Withdrawal Category:")), g_flagsH);
    newTransflexGridSizer->Add(defaultCategoryNonTransferChoice, g_flagsExpand);
    newTransflexGridSizer->Add(new wxStaticText(misc_panel, wxID_STATIC, _("Default Transfer Category:")), g_flagsH);
    newTransflexGridSizer->Add(defaultCategoryTransferChoice, g_flagsExpand);
    newTransflexGridSizer->Add(new wxStaticText(misc_panel, wxID_STATIC, _("Default Status:")), g_flagsH);
    newTransflexGridSizer->Add(default_status, g_flagsExpand);
    newTransflexGridSizer->Add(new wxStaticText(misc_panel, wxID_STATIC, _("Bulk Transactions:")), g_flagsH);
    newTransflexGridSizer->Add(bulk_enter, g_flagsExpand);

    //----------------------------------------------
    //a bit more space visual appearance
    othersPanelSizer->AddSpacer(10);

    wxBoxSizer* itemBoxSizerStockURL = new wxBoxSizer(wxVERTICAL);
    othersPanelSizer->Add(itemBoxSizerStockURL);

    // Backup Settings
    wxStaticBox* backupStaticBox = new wxStaticBox(misc_panel, wxID_STATIC, _("Database Backup"));
    SetBoldFont(backupStaticBox);

    wxStaticBoxSizer* backupStaticBoxSizer = new wxStaticBoxSizer(backupStaticBox, wxVERTICAL);
    othersPanelSizer->Add(backupStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxCheckBox* backupCheckBox = new wxCheckBox(misc_panel, ID_DIALOG_OPTIONS_CHK_BACKUP
        , _("Create a new backup when MMEX Start"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    backupCheckBox->SetValue(GetIniDatabaseCheckboxValue("BACKUPDB", false));
    backupCheckBox->SetToolTip(_("When MMEX Starts,\n"
        "creates the backup database: dbFile_start_YYYY-MM-DD.ext."));
    backupStaticBoxSizer->Add(backupCheckBox, g_flagsV);

    wxCheckBox* backupUpdateCheckBox = new wxCheckBox(misc_panel, ID_DIALOG_OPTIONS_CHK_BACKUP_UPDATE
        , _("Backup database on exit."), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    backupUpdateCheckBox->SetValue(GetIniDatabaseCheckboxValue("BACKUPDB_UPDATE", true));
    backupUpdateCheckBox->SetToolTip(_("When MMEX shuts down and changes made to database,\n"
        "creates or updates the backup database: dbFile_update_YYYY-MM-DD.ext."));
    backupStaticBoxSizer->Add(backupUpdateCheckBox, g_flagsV);

    int max = Model_Setting::instance().GetIntSetting("MAX_BACKUP_FILES", 4);
    m_max_files = new wxSpinCtrl(misc_panel, wxID_ANY
        , wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 999, max);
    m_max_files->SetValue(max);
    mmToolTip(m_max_files, _("Specify max number of backup files"));

    wxFlexGridSizer* flex_sizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    flex_sizer2->Add(new wxStaticText(misc_panel, wxID_STATIC, _("Max Files")), g_flagsH);
    flex_sizer2->Add(m_max_files, g_flagsH);
    backupStaticBoxSizer->Add(flex_sizer2);

    //CSV Import
    const wxString delimiter = Model_Infotable::instance().GetStringInfo("DELIMITER", mmex::DEFDELIMTER);

    wxStaticBox* csvStaticBox = new wxStaticBox(misc_panel, wxID_ANY, _("CSV Settings"));
    SetBoldFont(csvStaticBox);
    wxStaticBoxSizer* csvStaticBoxSizer = new wxStaticBoxSizer(csvStaticBox, wxVERTICAL);

    othersPanelSizer->Add(csvStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));
    wxFlexGridSizer* csvStaticBoxSizerGrid = new wxFlexGridSizer(0, 2, 0, 10);
    csvStaticBoxSizer->Add(csvStaticBoxSizerGrid, g_flagsV);

    csvStaticBoxSizerGrid->Add(new wxStaticText(misc_panel, wxID_STATIC, _("Delimiter")), g_flagsH);
    wxTextCtrl* textDelimiter4 = new wxTextCtrl(misc_panel
        , ID_DIALOG_OPTIONS_TEXTCTRL_DELIMITER4, delimiter);
    mmToolTip(textDelimiter4, _("Specify the delimiter to use when importing/exporting CSV files"));
    textDelimiter4->SetMaxLength(1);
    csvStaticBoxSizerGrid->Add(textDelimiter4, g_flagsH);

    wxCommandEvent evt;
    OptionSettingsMisc::OnBackupChanged(evt);

    Fit();
    misc_panel->SetMinSize(misc_panel->GetBestVirtualSize());
    misc_panel->SetScrollRate(1, 1);
}

void OptionSettingsMisc::OnBackupChanged(wxCommandEvent& WXUNUSED(event))
{
    wxCheckBox* ChkBackup = static_cast<wxCheckBox*>(FindWindow(ID_DIALOG_OPTIONS_CHK_BACKUP));
    wxCheckBox* ChkBackupUpdate = static_cast<wxCheckBox*>(FindWindow(ID_DIALOG_OPTIONS_CHK_BACKUP_UPDATE));
    m_max_files->Enable(ChkBackup->GetValue() || ChkBackupUpdate->GetValue());
}

void OptionSettingsMisc::SaveStocksUrl()
{
    wxComboBox* url = static_cast<wxComboBox*>(FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_STOCKURL));
    wxString stockURL = url->GetValue();
    if (!stockURL.IsEmpty())
    {
        Model_Infotable::instance().Set("STOCKURL", stockURL);
    }
    else
    {
        Model_Infotable::Data_Set items = Model_Infotable::instance().find(Model_Infotable::INFONAME("STOCKURL"));
        if (!items.empty())
            Model_Infotable::instance().remove(items[0].INFOID);
    }
}

bool OptionSettingsMisc::SaveSettings()
{
    wxChoice* itemChoice = static_cast<wxChoice*>(FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_PAYEE));
    Option::instance().TransPayeeSelection(itemChoice->GetSelection());

    itemChoice = static_cast<wxChoice*>(FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY_NONTRANSFER));
    Option::instance().TransCategorySelectionNonTransfer(itemChoice->GetSelection());

    itemChoice = static_cast<wxChoice*>(FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY_TRANSFER));
    Option::instance().TransCategorySelectionTransfer(itemChoice->GetSelection());

    itemChoice = static_cast<wxChoice*>(FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_STATUS));
    Option::instance().TransStatusReconciled(itemChoice->GetSelection());

    itemChoice = static_cast<wxChoice*>(FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_DATE));
    Option::instance().TransDateDefault(itemChoice->GetSelection());

    itemChoice = static_cast<wxChoice*>(FindWindow(ID_DIALOG_OPTIONS_BULK_ENTER));
    Option::instance().set_bulk_transactions(itemChoice->GetSelection() == 0);

    SaveStocksUrl();
    Option::instance().SharePrecision(m_share_precision->GetValue());

    wxCheckBox* itemCheckBox = static_cast<wxCheckBox*>(FindWindow(ID_DIALOG_OPTIONS_CHK_BACKUP));
    Model_Setting::instance().Set("BACKUPDB", itemCheckBox->GetValue());

    wxCheckBox* itemCheckBoxUpdate = static_cast<wxCheckBox*>(FindWindow(ID_DIALOG_OPTIONS_CHK_BACKUP_UPDATE));
    Model_Setting::instance().Set("BACKUPDB_UPDATE", itemCheckBoxUpdate->GetValue());

    Model_Setting::instance().Set("MAX_BACKUP_FILES", m_max_files->GetValue());

    wxTextCtrl* st = static_cast<wxTextCtrl*>(FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_DELIMITER4));
    const wxString& delim = st->GetValue();
    if (!delim.IsEmpty()) Model_Infotable::instance().Set("DELIMITER", delim);

    return true;
}
