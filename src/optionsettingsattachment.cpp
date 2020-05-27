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

#include "optionsettingsattachment.h"
#include "attachmentdialog.h"
#include "paths.h"
#include "util.h"
#include "mmSimpleDialogs.h"

/*******************************************************/
wxBEGIN_EVENT_TABLE(OptionSettingsAttachment, wxPanel)
    EVT_BUTTON(ID_DIALOG_OPTIONS_BUTTON_ATTACHMENTSFOLDER, OptionSettingsAttachment::OnAttachmentsButton)
    EVT_TEXT(ID_DIALOG_OPTIONS_TEXTCTRL_ATTACHMENT, OptionSettingsAttachment::OnAttachmentsPathChanged)
    EVT_CHECKBOX(ID_DIALOG_OPTIONS_CHECKBOX_ATTACHMENTSSUBFOLDER, OptionSettingsAttachment::OnAttachmentsSubfolderChanged)
wxEND_EVENT_TABLE()
/*******************************************************/

OptionSettingsAttachment::OptionSettingsAttachment()
{
}

OptionSettingsAttachment::OptionSettingsAttachment(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
{
    wxPanel::Create(parent, id, pos, size, style, name);
    Create();
}

OptionSettingsAttachment::~OptionSettingsAttachment()
{
}

void OptionSettingsAttachment::Create()
{
    wxBoxSizer* attachmentPanelSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(attachmentPanelSizer);

    //Attachments
    wxStaticBox* attachmentStaticBox = new wxStaticBox(this, wxID_ANY, _("Attachments Settings"));
    SetBoldFont(attachmentStaticBox);
    wxStaticBoxSizer* attachmentStaticBoxSizer = new wxStaticBoxSizer(attachmentStaticBox, wxVERTICAL);

    attachmentPanelSizer->Add(attachmentStaticBoxSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxString OSType = wxPlatformInfo::Get().GetOperatingSystemFamilyName();
    wxString attachmentStaticText_desc = wxString::Format(_("Attachment archive folder for %s only:"), OSType);

    wxStaticText* attachmentStaticText = new wxStaticText(this, wxID_STATIC, attachmentStaticText_desc);
    attachmentStaticBoxSizer->Add(attachmentStaticText, g_flagsV);
    attachmentStaticText->SetToolTip(_("Every OS type (Win,Mac,Unix) has its attachment folder"));

    wxBoxSizer* attachDefinedSizer = new wxBoxSizer(wxHORIZONTAL);
    attachmentStaticBoxSizer->Add(attachDefinedSizer);

    const wxString attachmentFolder = Model_Infotable::instance().GetStringInfo("ATTACHMENTSFOLDER:" + mmPlatformType(), "");
    m_old_path = mmex::getPathAttachment(attachmentFolder);

    wxArrayString list;
    list.Add(ATTACHMENTS_FOLDER_DOCUMENTS);
    list.Add(ATTACHMENTS_FOLDER_USERPROFILE);
    list.Add(ATTACHMENTS_FOLDER_DATABASE);
    list.Add(ATTACHMENTS_FOLDER_APPDATA);

    m_attachments_path = new wxComboBox(this, ID_DIALOG_OPTIONS_TEXTCTRL_ATTACHMENT, "", wxDefaultPosition, wxSize(225, -1), list);
    m_attachments_path->ChangeValue(attachmentFolder);

    wxButton* AttachmentsFolderButton = new wxButton(this
        , ID_DIALOG_OPTIONS_BUTTON_ATTACHMENTSFOLDER, "...", wxDefaultPosition, wxSize(25, -1), 0);
    AttachmentsFolderButton->SetToolTip(_("Browse for folder"));

    attachDefinedSizer->Add(m_attachments_path, g_flagsH);
    attachDefinedSizer->Add(AttachmentsFolderButton, g_flagsH);

    m_attachments_preview = new wxStaticText(this, wxID_STATIC, _("Real path:") + "\n" + mmex::getPathAttachment(attachmentFolder));
    m_attachments_preview->SetFont(this->GetFont().Smaller());
    attachmentStaticBoxSizer->Add(m_attachments_preview, g_flagsV);

    // Legend
    wxStaticBox* attachmentStaticBoxLegend = new wxStaticBox(this, wxID_ANY, _("Legend (can be used as variables at the beginning of above path)"));
    attachmentStaticBoxLegend->SetFont(this->GetFont().Italic());
    wxStaticBoxSizer* attachmentStaticBoxSizerLegend = new wxStaticBoxSizer(attachmentStaticBoxLegend, wxVERTICAL);
    attachmentStaticBoxSizer->Add(attachmentStaticBoxSizerLegend, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxString legend = wxString::Format(_("%s -> User document directory"), ATTACHMENTS_FOLDER_DOCUMENTS);
    legend += "\n" + wxString::Format(_("%s -> User profile folder"), ATTACHMENTS_FOLDER_USERPROFILE);
    legend += "\n" + wxString::Format(_("%s -> Folder of .MMB database file"), ATTACHMENTS_FOLDER_DATABASE);
    legend += "\n" + wxString::Format(_("%s -> MMEX Application data folder"), ATTACHMENTS_FOLDER_APPDATA);
    wxStaticText* legendStaticText = new wxStaticText(this, wxID_STATIC, legend);
    attachmentStaticBoxSizerLegend->Add(legendStaticText);
    //End legend

    //Other OS folders
    wxStaticBox* attachmentStaticBoxInfo = new wxStaticBox(this, wxID_ANY, _("Other OS folders "));
    attachmentStaticBoxInfo->SetFont(this->GetFont().Italic());
    wxStaticBoxSizer* attachmentStaticBoxSizerInfo = new wxStaticBoxSizer(attachmentStaticBoxInfo, wxVERTICAL);
    attachmentStaticBoxSizer->Add(attachmentStaticBoxSizerInfo, wxSizerFlags(g_flagsExpand).Proportion(0));

    const wxString FolderNotSet = _("Not set yet");
    const wxString attachmentFolderWin = Model_Infotable::instance().GetStringInfo("ATTACHMENTSFOLDER:Win", FolderNotSet);
    const wxString attachmentFolderMac = Model_Infotable::instance().GetStringInfo("ATTACHMENTSFOLDER:Mac", FolderNotSet);
    const wxString attachmentFolderUnix = Model_Infotable::instance().GetStringInfo("ATTACHMENTSFOLDER:Uni", FolderNotSet);

    if (mmPlatformType() != "Win")
    {
        wxStaticText* attachmentFolderWinText = new wxStaticText(this, wxID_STATIC, _("Windows folder -> ") + attachmentFolderWin.Left(50));
        attachmentFolderWinText->SetToolTip(attachmentFolderWin);
        attachmentStaticBoxSizerInfo->Add(attachmentFolderWinText);
    }

    if (mmPlatformType() != "Mac")
    {
        wxStaticText* attachmentFolderMacText = new wxStaticText(this, wxID_STATIC, _("Mac folder -> ") + attachmentFolderMac.Left(50));
        attachmentFolderMacText->SetToolTip(attachmentFolderMac);
        attachmentStaticBoxSizerInfo->Add(attachmentFolderMacText);
    }

    if (mmPlatformType() != "Uni")
    {
        wxStaticText* attachmentFolderUnixText = new wxStaticText(this, wxID_STATIC, _("Unix folder -> ") + attachmentFolderUnix.Left(50));
        attachmentFolderUnixText->SetToolTip(attachmentFolderUnix);
        attachmentStaticBoxSizerInfo->Add(attachmentFolderUnixText);
    }
    //End other OS folders

    const wxString LastDBPath = Model_Setting::instance().getLastDbPath();
    const wxString subFolder = wxString::Format("MMEX_%s_Attachments", wxFileName::FileName(LastDBPath).GetName());
    const wxString cbAttachmentsSubfolder_desc = _("Create and use Attachments subfolder");

    m_attachments_subfolder = new wxCheckBox(this, ID_DIALOG_OPTIONS_CHECKBOX_ATTACHMENTSSUBFOLDER
        , cbAttachmentsSubfolder_desc, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_attachments_subfolder->SetValue(Model_Infotable::instance().GetBoolInfo("ATTACHMENTSSUBFOLDER", true));
    attachmentStaticBoxSizer->Add(m_attachments_subfolder, g_flagsV);
    attachmentStaticBoxSizer->Add(new wxStaticText(this, wxID_STATIC, subFolder), g_flagsV);

    attachmentStaticBoxSizer->AddSpacer(20);

    m_delete_attachments = new wxCheckBox(this, wxID_STATIC,
        _("Delete file after import"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_delete_attachments->SetValue(Model_Infotable::instance().GetBoolInfo("ATTACHMENTSDELETE", false));
    m_delete_attachments->SetToolTip(_("Select to delete file after import in attachments archive"));
    attachmentStaticBoxSizer->Add(m_delete_attachments, g_flagsV);

    m_trash_attachments = new wxCheckBox(this, wxID_STATIC,
        _("When remove attachment, move file instead of delete"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_trash_attachments->SetValue(Model_Infotable::instance().GetBoolInfo("ATTACHMENTSTRASH", false));
    m_trash_attachments->SetToolTip(_("Select to don't delete file when attachment is removed, but instead move it to 'Deleted' subfolder"));
    attachmentStaticBoxSizer->Add(m_trash_attachments, g_flagsV);
}

void OptionSettingsAttachment::OnAttachmentsButton(wxCommandEvent& WXUNUSED(event))
{
    wxString AttachmentsFolder = mmex::getPathAttachment(m_attachments_path->GetValue());

    wxDirDialog dlg(this
        , _("Choose folder to set as attachments archive")
        , AttachmentsFolder
        , wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST
    );

    if (dlg.ShowModal() != wxID_OK)
        return;

    AttachmentsFolder = dlg.GetPath();
    m_attachments_path->SetValue(AttachmentsFolder);
}

void OptionSettingsAttachment::OnAttachmentsPathChanged(wxCommandEvent& WXUNUSED(event))
{
    wxString AttachmentsFolder = mmex::getPathAttachment(m_attachments_path->GetValue().Trim());
    m_attachments_preview->SetLabelText(_("Real path:") + "\n" + AttachmentsFolder);
}

void OptionSettingsAttachment::OnAttachmentsSubfolderChanged(wxCommandEvent& event)
{
    Model_Infotable::instance().Set("ATTACHMENTSSUBFOLDER", m_attachments_subfolder->GetValue());
    OnAttachmentsPathChanged(event);
}

bool OptionSettingsAttachment::SaveSettings()
{
    wxString attachmentsFolder = mmex::getPathAttachment(m_attachments_path->GetValue().Trim());

    if (attachmentsFolder != wxEmptyString)
    {
        if (!wxDirExists(attachmentsFolder))
        {
            if (!wxMkdir(attachmentsFolder))
            {
                return false;
            }
        }

        if (!mmAttachmentManage::CreateReadmeFile(attachmentsFolder))
        {
            mmErrorDialogs::MessageError(this, _("Attachments folder it's not writable!"), _("Attachments folder"));
            return false;
        }

        wxString attachmentsFolderOld = mmex::getPathAttachment(m_old_path);
        if (attachmentsFolder != m_old_path && wxDirExists(attachmentsFolderOld))
        {
            int MoveResponse = wxMessageBox(
                wxString::Format("%s\n", _("Attachments path has been changed!"))
                + ("Do you want to move all attachments to the new location?")
                , _("Attachments folder migration")
                , wxYES_NO | wxYES_DEFAULT | wxICON_WARNING);
            if (MoveResponse == wxYES)
            {
                if (!wxRenameFile(attachmentsFolderOld, attachmentsFolder))
                    wxMessageBox(
                    wxString::Format("%s\n\n", _("Error moving attachments folder: please move it manually!")) +
                    wxString::Format("%s: %s\n", _("Origin"), mmex::getPathAttachment(m_old_path)) +
                    wxString::Format("%s: %s", _("Destination"), attachmentsFolder)
                    , _("Attachments folder migration")
                    , wxICON_ERROR);
            }
            m_old_path = attachmentsFolder;
        }
    }

    Model_Infotable::instance().Set("ATTACHMENTSFOLDER:" + mmPlatformType(), m_attachments_path->GetValue().Trim());
    Model_Infotable::instance().Set("ATTACHMENTSDELETE", m_delete_attachments->GetValue());
    Model_Infotable::instance().Set("ATTACHMENTSTRASH", m_trash_attachments->GetValue());

    return true;
}
