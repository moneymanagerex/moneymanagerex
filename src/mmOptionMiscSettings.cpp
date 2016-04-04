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

#include "mmOptionMiscSettings.h"
#include "mmOption.h"

#include "model/Model_Checking.h"

#include <wx/spinctrl.h>

/*******************************************************/
wxBEGIN_EVENT_TABLE(mmOptionMiscSettings, wxPanel)
    EVT_CHECKBOX(ID_DIALOG_OPTIONS_CHK_BACKUP, mmOptionMiscSettings::OnBackupChanged)
    EVT_CHECKBOX(ID_DIALOG_OPTIONS_CHK_BACKUP_UPDATE, mmOptionMiscSettings::OnBackupChanged)
wxEND_EVENT_TABLE()
/*******************************************************/

mmOptionMiscSettings::mmOptionMiscSettings()
{
}

mmOptionMiscSettings::mmOptionMiscSettings(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
{
    wxPanel::Create(parent, id, pos, size, style, name);
    Create();
}

mmOptionMiscSettings::~mmOptionMiscSettings()
{
}

void mmOptionMiscSettings::Create()
{
    wxBoxSizer* othersPanelSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(othersPanelSizer);

    wxStaticText* itemStaticTextURL = new wxStaticText(this, wxID_STATIC, _("Stock Quote Web Page"));
    SetBoldFont(itemStaticTextURL);

    othersPanelSizer->Add(itemStaticTextURL, g_flags);

    wxString stockURL = Model_Infotable::instance().GetStringInfo("STOCKURL", mmex::weblink::DefStockUrl);
    wxTextCtrl* itemTextCtrURL = new wxTextCtrl(this
        , ID_DIALOG_OPTIONS_TEXTCTRL_STOCKURL, stockURL);
    othersPanelSizer->Add(itemTextCtrURL, wxSizerFlags(g_flagsExpand).Proportion(0));
    itemTextCtrURL->SetToolTip(_("Clear the field to Reset the value to system default."));
    othersPanelSizer->AddSpacer(15);

    // New transaction dialog settings
    wxStaticBox* transSettingsStaticBox = new wxStaticBox(this, wxID_STATIC, _("New Transaction Dialog Settings"));
    SetBoldFont(transSettingsStaticBox);
    
    wxStaticBoxSizer* transSettingsStaticBoxSizer = new wxStaticBoxSizer(transSettingsStaticBox, wxVERTICAL);
    othersPanelSizer->Add(transSettingsStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxArrayString defaultValues_;
    defaultValues_.Add(_("None"));
    defaultValues_.Add(_("Last Used"));

    wxChoice* defaultDateChoice = new wxChoice(this
        , ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_DATE
        , wxDefaultPosition, wxSize(140, -1), defaultValues_);
    defaultDateChoice->SetSelection(Option::instance().TransDateDefault());

    defaultValues_[1] = (_("Last used for payee"));
    defaultValues_.Add(_("Unused"));
    wxChoice* defaultPayeeChoice = new wxChoice(this
        , ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_PAYEE
        , wxDefaultPosition, wxSize(140, -1), defaultValues_);
    defaultPayeeChoice->SetSelection(Option::instance().TransPayeeSelectionNone());

    wxChoice* defaultCategoryChoice = new wxChoice(this
        , ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY
        , wxDefaultPosition, defaultPayeeChoice->GetSize(), defaultValues_);
    defaultCategoryChoice->SetSelection(Option::instance().TransCategorySelectionNone());

    wxChoice* default_status = new wxChoice(this
        , ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_STATUS
        , wxDefaultPosition, defaultDateChoice->GetSize());

    for (const auto& i : Model_Checking::all_status())
        default_status->Append(wxGetTranslation(i), new wxStringClientData(i));

    default_status->SetSelection(Option::instance().TransStatusReconciled());

    wxFlexGridSizer* newTransflexGridSizer = new wxFlexGridSizer(0, 2, 0, 0);
    transSettingsStaticBoxSizer->Add(newTransflexGridSizer);
    newTransflexGridSizer->Add(new wxStaticText(this, wxID_STATIC, _("Default Date:")), g_flags);
    newTransflexGridSizer->Add(defaultDateChoice, g_flags);
    newTransflexGridSizer->Add(new wxStaticText(this, wxID_STATIC, _("Default Payee:")), g_flags);
    newTransflexGridSizer->Add(defaultPayeeChoice, g_flags);
    newTransflexGridSizer->Add(new wxStaticText(this, wxID_STATIC, _("Default Category:")), g_flags);
    newTransflexGridSizer->Add(defaultCategoryChoice, g_flags);
    newTransflexGridSizer->Add(new wxStaticText(this, wxID_STATIC, _("Default Status:")), g_flags);
    newTransflexGridSizer->Add(default_status, g_flags);

    //----------------------------------------------
    //a bit more space visual appearance
    othersPanelSizer->AddSpacer(10);

    wxBoxSizer* itemBoxSizerStockURL = new wxBoxSizer(wxVERTICAL);
    othersPanelSizer->Add(itemBoxSizerStockURL);

    // Backup Settings
    wxStaticBox* backupStaticBox = new wxStaticBox(this, wxID_STATIC, _("Database Backup"));
    SetBoldFont(backupStaticBox);

    wxStaticBoxSizer* backupStaticBoxSizer = new wxStaticBoxSizer(backupStaticBox, wxVERTICAL);
    othersPanelSizer->Add(backupStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxCheckBox* backupCheckBox = new wxCheckBox(this, ID_DIALOG_OPTIONS_CHK_BACKUP
        , _("Create a new backup when MMEX Start"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    backupCheckBox->SetValue(GetIniDatabaseCheckboxValue("BACKUPDB", false));
    backupCheckBox->SetToolTip(_("When MMEX Starts,\n"
        "creates the backup database: dbFile_start_YYYY-MM-DD.ext."));
    backupStaticBoxSizer->Add(backupCheckBox, g_flags);

    wxCheckBox* backupUpdateCheckBox = new wxCheckBox(this, ID_DIALOG_OPTIONS_CHK_BACKUP_UPDATE
        , _("Backup database on exit."), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    backupUpdateCheckBox->SetValue(GetIniDatabaseCheckboxValue("BACKUPDB_UPDATE", true));
    backupUpdateCheckBox->SetToolTip(_("When MMEX shuts down and changes made to database,\n"
        "creates or updates the backup database: dbFile_update_YYYY-MM-DD.ext."));
    backupStaticBoxSizer->Add(backupUpdateCheckBox, g_flags);

    int max = Model_Setting::instance().GetIntSetting("MAX_BACKUP_FILES", 4);
    m_max_files = new wxSpinCtrl(this, wxID_ANY
        , wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 999, max);
    m_max_files->SetValue(max);
    m_max_files->SetToolTip(_("Specify max number of backup files"));

    wxFlexGridSizer* flex_sizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    flex_sizer2->Add(new wxStaticText(this, wxID_STATIC, _("Max Files")), g_flags);
    flex_sizer2->Add(m_max_files, g_flags);
    backupStaticBoxSizer->Add(flex_sizer2);

    //CSV Import
    const wxString delimiter = Model_Infotable::instance().GetStringInfo("DELIMITER", mmex::DEFDELIMTER);

    wxStaticBox* csvStaticBox = new wxStaticBox(this, wxID_ANY, _("CSV Settings"));
    SetBoldFont(csvStaticBox);
    wxStaticBoxSizer* csvStaticBoxSizer = new wxStaticBoxSizer(csvStaticBox, wxVERTICAL);

    othersPanelSizer->Add(csvStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));
    wxFlexGridSizer* csvStaticBoxSizerGrid = new wxFlexGridSizer(0, 2, 0, 10);
    csvStaticBoxSizer->Add(csvStaticBoxSizerGrid, g_flags);

    csvStaticBoxSizerGrid->Add(new wxStaticText(this, wxID_STATIC, _("Delimiter")), g_flags);
    wxTextCtrl* textDelimiter4 = new wxTextCtrl(this
        , ID_DIALOG_OPTIONS_TEXTCTRL_DELIMITER4, delimiter);
    textDelimiter4->SetToolTip(_("Specify the delimiter to use when importing/exporting CSV files"));
    textDelimiter4->SetMaxLength(1);
    csvStaticBoxSizerGrid->Add(textDelimiter4, g_flags);

    wxCommandEvent evt;
    mmOptionMiscSettings::OnBackupChanged(evt);
}

void mmOptionMiscSettings::OnBackupChanged(wxCommandEvent& event)
{
    wxCheckBox* ChkBackup = (wxCheckBox*)FindWindow(ID_DIALOG_OPTIONS_CHK_BACKUP);
    wxCheckBox* ChkBackupUpdate = (wxCheckBox*)FindWindow(ID_DIALOG_OPTIONS_CHK_BACKUP_UPDATE);
    m_max_files->Enable(ChkBackup->GetValue() || ChkBackupUpdate->GetValue());
}

void mmOptionMiscSettings::SaveStocksUrl()
{
    wxTextCtrl* url = (wxTextCtrl*)FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_STOCKURL);
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

void mmOptionMiscSettings::SaveSettings()
{
    wxChoice* itemChoice = (wxChoice*)FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_PAYEE);
    Option::instance().TransPayeeSelectionNone(itemChoice->GetSelection());

    itemChoice = (wxChoice*)FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY);
    Option::instance().TransCategorySelectionNone(itemChoice->GetSelection());

    itemChoice = (wxChoice*)FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_STATUS);
    Option::instance().TransStatusReconciled(itemChoice->GetSelection());

    itemChoice = (wxChoice*)FindWindow(ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_DATE);
    Option::instance().TransDateDefault(itemChoice->GetSelection());

    SaveStocksUrl();

    wxCheckBox* itemCheckBox = (wxCheckBox*)FindWindow(ID_DIALOG_OPTIONS_CHK_BACKUP);
    Model_Setting::instance().Set("BACKUPDB", itemCheckBox->GetValue());

    wxCheckBox* itemCheckBoxUpdate = (wxCheckBox*)FindWindow(ID_DIALOG_OPTIONS_CHK_BACKUP_UPDATE);
    Model_Setting::instance().Set("BACKUPDB_UPDATE", itemCheckBoxUpdate->GetValue());

    Model_Setting::instance().Set("MAX_BACKUP_FILES", m_max_files->GetValue());

    wxTextCtrl* st = (wxTextCtrl*)FindWindow(ID_DIALOG_OPTIONS_TEXTCTRL_DELIMITER4);
    const wxString& delim = st->GetValue();
    if (!delim.IsEmpty()) Model_Infotable::instance().Set("DELIMITER", delim);
}
