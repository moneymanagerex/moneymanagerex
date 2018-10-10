/*******************************************************
Copyright (C) 2014 Gabriele-V

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

#include "attachmentdialog.h"
#include "images_list.h"
#include "constants.h"
#include "mmSimpleDialogs.h"
#include "paths.h"
#include "util.h"

#include "Model_Attachment.h"
#include "Model_Infotable.h"

#include <wx/mimetype.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmAttachmentDialog, wxDialog);

wxBEGIN_EVENT_TABLE( mmAttachmentDialog, wxDialog )
    EVT_BUTTON(wxID_CANCEL, mmAttachmentDialog::OnCancel)
    EVT_BUTTON(wxID_OK, mmAttachmentDialog::OnOk)
    EVT_BUTTON(wxID_APPLY, mmAttachmentDialog::OnMagicButton)
    EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, mmAttachmentDialog::OnListItemSelected)
    EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, mmAttachmentDialog::OnItemRightClick)
    EVT_MENU_RANGE(MENU_NEW_ATTACHMENT, MENU_DELETE_ATTACHMENT, mmAttachmentDialog::OnMenuSelected)
    EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, mmAttachmentDialog::OnListItemActivated)
wxEND_EVENT_TABLE()


mmAttachmentDialog::mmAttachmentDialog (wxWindow* parent, const wxString& RefType, int RefId, const wxString& name) :
    m_attachment_id(-1)
    , m_RefType(RefType)
    , m_RefId(RefId)
    #ifdef _DEBUG
        , debug_(true)
    #else
        , debug_(false)
    #endif
{
    if (debug_) ColName_[ATTACHMENT_ID] = _("#");
    ColName_[ATTACHMENT_DESCRIPTION] = _("Description");
    ColName_[ATTACHMENT_FILENAME] = _("File");

    Create(parent, name);

    const wxString AttachmentsFolder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting());

    if (AttachmentsFolder == wxEmptyString)
    {
        wxString msgStr = wxString() << _("Attachment folder not defined.") << "\n"
            << _("Please set it in Tools -> Options -> Attachments") << "\n";
        wxMessageBox(msgStr, _("Attachment folder not defined"), wxICON_ERROR);
    }
    else if (!wxDirExists(AttachmentsFolder))
    {
        wxString msgStr = wxString() << _("Unable to find attachments folder:") << "\n"
            << "'" << AttachmentsFolder << "'" << "\n"
            << "\n"
            << _("Please verify that above path is correct") << "\n";
        wxMessageBox(msgStr, _("Attachments folder not found"), wxICON_ERROR);
    }
}

void mmAttachmentDialog::Create(wxWindow* parent, const wxString& name)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    long style = wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER;

    wxString WindowTitle;
    if (m_RefId > 0)
        WindowTitle = wxString::Format(_("Organize Attachments | %s %i"), wxGetTranslation(m_RefType), m_RefId);
    else
        WindowTitle = wxString::Format(_("Organize Attachments | New %s"), wxGetTranslation(m_RefType));

    if (!wxDialog::Create(parent, wxID_ANY, WindowTitle, wxDefaultPosition, wxDefaultSize, style, name))
        return;

    CreateControls();
    fillControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    fillControls();
    SetIcon(mmex::getProgramIcon());
    Centre();
}

void mmAttachmentDialog::CreateControls()
{
    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxVERTICAL);

    attachmentListBox_ = new wxDataViewListCtrl( this
        , wxID_ANY, wxDefaultPosition, wxSize(460, 500)/*, wxDV_HORIZ_RULES*/);

    if (debug_) attachmentListBox_->AppendTextColumn(ColName_[ATTACHMENT_ID], wxDATAVIEW_CELL_INERT, 30);
    attachmentListBox_->AppendTextColumn(ColName_[ATTACHMENT_DESCRIPTION], wxDATAVIEW_CELL_INERT, 150);
    attachmentListBox_->AppendTextColumn(ColName_[ATTACHMENT_FILENAME], wxDATAVIEW_CELL_INERT, 300);
    mainBoxSizer->Add(attachmentListBox_, wxSizerFlags(g_flagsExpand).Border(wxALL, 10));

    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    mainBoxSizer->Add(buttons_panel, wxSizerFlags(g_flagsV).Center());
    wxStdDialogButtonSizer* buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxButton* buttonOK = new wxButton(buttons_panel, wxID_OK, _("&OK "));
    wxButton* btnCancel = new wxButton(buttons_panel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    buttons_sizer->Add(buttonOK, g_flagsH);
    buttons_sizer->Add(btnCancel, g_flagsH);

    wxBitmapButton* magicButton = new wxBitmapButton(buttons_panel
        , wxID_APPLY, mmBitmap(png::RUN));
    magicButton->SetToolTip(_("Other tools"));
    buttons_sizer->Add(magicButton, g_flagsH);

    Center();
    this->SetSizer(mainBoxSizer);
}

void mmAttachmentDialog::fillControls()
{    
    attachmentListBox_->DeleteAllItems();

    Model_Attachment::Data_Set attachments = Model_Attachment::instance().FilterAttachments(m_RefType, m_RefId);
    if (attachments.size() == 0) return;

    int firstInTheListAttachentID = -1;
    for (const auto &entry : attachments)
    {
        if (firstInTheListAttachentID == -1) firstInTheListAttachentID = entry.ATTACHMENTID;
        wxVector<wxVariant> data;
        if (debug_) data.push_back(wxVariant(wxString::Format("%i", entry.ATTACHMENTID)));
        data.push_back(wxVariant(entry.DESCRIPTION));
        data.push_back(wxVariant(entry.REFTYPE + m_PathSep + entry.FILENAME));
        attachmentListBox_->AppendItem(data, (wxUIntPtr)entry.ATTACHMENTID);
    }

    m_attachment_id = firstInTheListAttachentID;
}

void mmAttachmentDialog::OnListItemSelected(wxDataViewEvent& event)
{
    wxDataViewItem item = event.GetItem();
    int selected_index = attachmentListBox_->ItemToRow(item);

    if (selected_index >= 0)
        m_attachment_id = (int)attachmentListBox_->GetItemData(item);
}

void mmAttachmentDialog::AddAttachment()
{
    const wxString attachmentFilePath = wxFileSelector(_("Import attachment:")
        , wxEmptyString, wxEmptyString, wxEmptyString
        , _("All Files")+ "|" + wxFileSelectorDefaultWildcardStr
        , wxFD_FILE_MUST_EXIST);

    if (attachmentFilePath.empty()) return;

    const wxString attachmentFileName = wxFileName(attachmentFilePath).GetName();
    const wxString attachmentFileExtension = wxFileName(attachmentFilePath).GetExt().MakeLower();
    
    mmDialogComboBoxAutocomplete dlg(this, _("Enter a description for the new attachment:"),
        _("Organize Attachments: Add Attachment"), attachmentFileName, Model_Attachment::instance().allDescriptions());

    if (dlg.ShowModal() != wxID_OK)
        return;

    const wxString attachmentDescription = dlg.getText();

    wxString AttachmentsFolder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting());
    int attachmentLastNumber = Model_Attachment::LastAttachmentNumber(m_RefType, m_RefId);

    wxString importedFileName = m_RefType + "_" + wxString::Format("%i", m_RefId) + "_Attach"
        + wxString::Format("%i", attachmentLastNumber + 1) + "." + attachmentFileExtension;

    if (mmAttachmentManage::CopyAttachment(attachmentFilePath, AttachmentsFolder + m_PathSep + m_RefType + m_PathSep + importedFileName))
    {
        Model_Attachment::Data* NewAttachment = Model_Attachment::instance().create();
        NewAttachment->REFTYPE = m_RefType;
        NewAttachment->REFID = m_RefId;
        NewAttachment->DESCRIPTION = attachmentDescription;
        NewAttachment->FILENAME = importedFileName;
        m_attachment_id = Model_Attachment::instance().save(NewAttachment);
    }

    fillControls();
}

void mmAttachmentDialog::OpenAttachment()
{
    Model_Attachment::Data *attachments = Model_Attachment::instance().get(m_attachment_id);
    wxString attachmentFilePath = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting())
        + m_PathSep + attachments->REFTYPE + m_PathSep + attachments->FILENAME;

    mmAttachmentManage::OpenAttachment(attachmentFilePath);
}

void mmAttachmentDialog::EditAttachment()
{
    Model_Attachment::Data *attachment = Model_Attachment::instance().get(m_attachment_id);
    if (attachment)
    {
        mmDialogComboBoxAutocomplete dlg(this, _("Enter a new description for the attachment:"),
            _("Organize Attachments: Edit Attachment"), attachment->DESCRIPTION,
            Model_Attachment::instance().allDescriptions());

        if (dlg.ShowModal() != wxID_OK)
            return;

        const wxString description = dlg.getText();
        if (description == attachment->DESCRIPTION)
            return;

        attachment->DESCRIPTION = description;
        m_attachment_id = Model_Attachment::instance().save(attachment);

        fillControls();
    }
}

void mmAttachmentDialog::DeleteAttachment()
{
    Model_Attachment::Data *attachment = Model_Attachment::instance().get(m_attachment_id);
    if (attachment)
    {
        int DeleteResponse = wxMessageBox(
            _("Do you really want to delete this attachment?")
            , _("Confirm Attachment Deletion")
            , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
        if (DeleteResponse == wxYES)
        {
            wxString AttachmentsFolder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting()) + m_PathSep + attachment->REFTYPE;
            if (mmAttachmentManage::DeleteAttachment(AttachmentsFolder + m_PathSep + attachment->FILENAME))
            {
                Model_Attachment::instance().remove(m_attachment_id);
            }
            m_attachment_id = -1;
            fillControls();
        }
    }
}

void mmAttachmentDialog::OnMenuSelected(wxCommandEvent& event)
{
    switch(event.GetId())
    {
        case MENU_NEW_ATTACHMENT: AddAttachment(); break;
        case MENU_OPEN_ATTACHMENT: OpenAttachment(); break;
        case MENU_EDIT_ATTACHMENT: EditAttachment(); break;
        case MENU_DELETE_ATTACHMENT: DeleteAttachment(); break;
        default: break;
    }
}

void mmAttachmentDialog::OnMagicButton(wxCommandEvent& WXUNUSED(event))
{
    wxDataViewEvent evt;
    OnItemRightClick(evt);
}

void mmAttachmentDialog::OnItemRightClick(wxDataViewEvent& event)
{
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, wxID_ANY) ;
    evt.SetEventObject( this );

    Model_Attachment::Data* attachment = Model_Attachment::instance().get(m_attachment_id);

    wxMenu* mainMenu = new wxMenu;
    if (attachment) mainMenu->SetTitle(attachment->DESCRIPTION);
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_NEW_ATTACHMENT, _("&Add ")));
    mainMenu->AppendSeparator();
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_OPEN_ATTACHMENT, _("&Open ")));
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_EDIT_ATTACHMENT, _("&Edit ")));
    if (!attachment) mainMenu->Enable(MENU_EDIT_ATTACHMENT, false);
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_DELETE_ATTACHMENT, _("&Remove ")));
    
    //Disable buttons
    wxString AttachmentsFolder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting());
    if (AttachmentsFolder == wxEmptyString || !wxDirExists(AttachmentsFolder))
        mainMenu->Enable(MENU_NEW_ATTACHMENT, false);

    if (!attachment)
    {
        mainMenu->Enable(MENU_OPEN_ATTACHMENT, false);
        mainMenu->Enable(MENU_EDIT_ATTACHMENT, false);
        mainMenu->Enable(MENU_DELETE_ATTACHMENT, false);
    }

    PopupMenu(mainMenu);
    delete mainMenu;
    event.Skip();
}

void mmAttachmentDialog::OnListItemActivated(wxDataViewEvent& WXUNUSED(event))
{
    Model_Attachment::Data *attachment = Model_Attachment::instance().get(m_attachment_id);
    wxString attachmentFilePath = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting())
        + m_PathSep + attachment->REFTYPE + m_PathSep + attachment->FILENAME;

    mmAttachmentManage::OpenAttachment(attachmentFilePath);
}

void mmAttachmentDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

void mmAttachmentDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_OK);
}


/***********************
** mmAttachmentManage **
************************/
wxString mmAttachmentManage::m_PathSep = wxFileName::GetPathSeparator();

wxString mmAttachmentManage::InfotablePathSetting()
{
    return Model_Infotable::instance().GetStringInfo("ATTACHMENTSFOLDER:" + mmPlatformType(), "");
}

wxString mmAttachmentManage::GetAttachmentNoteSign()
{
    return wxString::Format("[%s] ",_("Att."));
}

bool mmAttachmentManage::CreateReadmeFile(const wxString& FolderPath)
{
    wxString ReadmeFilePath = FolderPath + m_PathSep + "readme.txt";
    wxString ReadmeText;
    ReadmeText << _("This directory and its files are automatically managed by Money Manager EX software.") << wxTextFile::GetEOL();
    ReadmeText << wxTextFile::GetEOL();
    ReadmeText << _("Please do not remove, rename or modify manually directories and files.") << wxTextFile::GetEOL();

    if (!wxFileExists(ReadmeFilePath))
    {
        wxFile file(ReadmeFilePath, wxFile::write);

        if (file.IsOpened())
        {
            file.Write(ReadmeText);
            file.Close();
            return true;
        }
    }

    return false;
}

bool mmAttachmentManage::CopyAttachment(const wxString& FileToImport, const wxString& ImportedFile)
{
    wxString destinationFolder = wxPathOnly(ImportedFile);

    if (!wxDirExists(destinationFolder))
    {
        if (wxMkdir(destinationFolder))
            mmAttachmentManage::CreateReadmeFile(destinationFolder);
        else
            return false;
    }

    if (wxFileExists(ImportedFile))
    {
        const auto &attachments = Model_Attachment::instance().find(Model_Attachment::FILENAME(wxFileNameFromPath(ImportedFile)));
        if (attachments.empty())
        {
            wxString msgStr = wxString() << _("Destination file already exist:") << "\n"
                << "'" << ImportedFile << "'" << "\n"
                << "\n"
                << _("File not found in attachments. Please delete or rename it.") << "\n";
            wxMessageBox(msgStr, _("Destination file already exist"), wxICON_ERROR);
        }
        else
        {
            wxString msgStr = wxString() << _("Destination file already exist:") << "\n"
                << "'" << ImportedFile << "'" << "\n"
                << "\n"
                << _("File already found in attachments") << "\n";
            wxMessageBox(msgStr, _("Destination file already exist"), wxICON_ERROR);
        }
        return false;
    }
    else if (wxCopyFile(FileToImport, ImportedFile))
    {
        if (Model_Infotable::instance().GetBoolInfo("ATTACHMENTSDELETE", false))
            wxRemoveFile(FileToImport);
    }
    else
        return false;

    return true;
}

bool mmAttachmentManage::DeleteAttachment(const wxString& FileToDelete)
{
    if (wxFileExists(FileToDelete))
    {
        if (Model_Infotable::instance().GetBoolInfo("ATTACHMENTSTRASH", false))
        {
            wxString DeletedAttachmentFolder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting()) + m_PathSep + "Deleted";

            if (!wxDirExists(DeletedAttachmentFolder))
            {
                if (wxMkdir(DeletedAttachmentFolder))
                    mmAttachmentManage::CreateReadmeFile(DeletedAttachmentFolder);
                else
                    return false;
            }

            wxString FileToTrash = DeletedAttachmentFolder + m_PathSep
                + wxDateTime::Now().FormatISODate() + "_" + wxFileNameFromPath(FileToDelete);

            if (!wxRenameFile(FileToDelete, FileToTrash))
                return false;
        }
        else if (!wxRemoveFile(FileToDelete))
            return false;
    }
    else
    {
        wxString msgStr = wxString() << _("Attachment not found:") << "\n"
            << "'" << FileToDelete << "'" << "\n"
            << "\n"
            << _("Do you want to continue and delete attachment on database?") << "\n";
        int DeleteResponse = wxMessageBox(msgStr, _("Delete attachment failed"), wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
        if (DeleteResponse == wxYES)
            return true;
        else
            return false;
    }
    return true;
}

bool mmAttachmentManage::OpenAttachment(const wxString& FileToOpen)
{
    if (!wxFileExists(FileToOpen))
    {
        wxString msgStr = wxString() << _("Unable to open file:") << "\n"
            << "'" << FileToOpen << "'" << "\n"
            << "\n"
            << _("Please verify that file exists and user has rights to read it.") << "\n";
        wxMessageBox(msgStr, _("Open attachment failed"), wxICON_ERROR);
        return false;
    }

    return wxLaunchDefaultApplication(FileToOpen);;
}

bool mmAttachmentManage::DeleteAllAttachments(const wxString& RefType, int RefId)
{
    Model_Attachment::Data_Set attachments = Model_Attachment::instance().FilterAttachments(RefType, RefId);
    wxString AttachmentsFolder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting()) + m_PathSep + RefType;

    for (const auto &entry : attachments)
    {
        mmAttachmentManage::DeleteAttachment(AttachmentsFolder + m_PathSep + entry.FILENAME);
        Model_Attachment::instance().remove(entry.ATTACHMENTID);
    }
    return true;
}

bool mmAttachmentManage::RelocateAllAttachments(const wxString& RefType, int OldRefId, int NewRefId)
{
    auto attachments = Model_Attachment::instance().find(Model_Attachment::DB_Table_ATTACHMENT::REFTYPE(RefType), Model_Attachment::REFID(OldRefId));
    wxString AttachmentsFolder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting()) + m_PathSep + RefType + m_PathSep;

    for (auto &entry : attachments)
    {
        wxString NewFileName = entry.FILENAME;
        NewFileName.Replace(entry.REFTYPE + "_" + wxString::Format("%i", entry.REFID), entry.REFTYPE + "_" + wxString::Format("%i", NewRefId));
        wxRenameFile(AttachmentsFolder + entry.FILENAME, AttachmentsFolder + NewFileName);
        entry.REFID = NewRefId;
        entry.FILENAME = NewFileName;
    }
    Model_Attachment::instance().save(attachments);

    return true;
}

void mmAttachmentManage::OpenAttachmentFromPanelIcon(wxWindow* parent, const wxString& RefType, int RefId)
{
    int AttachmentsNr = Model_Attachment::instance().NrAttachments(RefType, RefId);

    if (AttachmentsNr == 1)
    {
        Model_Attachment::Data_Set attachments = Model_Attachment::instance().FilterAttachments(RefType, RefId);
        wxString attachmentFilePath = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting())
            + m_PathSep + attachments[0].REFTYPE + m_PathSep + attachments[0].FILENAME;
        mmAttachmentManage::OpenAttachment(attachmentFilePath);
    }
    else
    {
        mmAttachmentDialog dlg(parent, RefType, RefId);
        dlg.ShowModal();
    }
}
