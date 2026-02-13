/*******************************************************
 Copyright (C) 2014 Gabriele-V
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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

#include "base/defs.h"
#include <wx/mimetype.h>

#include "base/constants.h"
#include "base/paths.h"
#include "base/images_list.h"
#include "util/_util.h"
#include "util/_simple.h"

#include "model/InfotableModel.h"
#include "model/AssetModel.h"
#include "model/StockModel.h"
#include "model/PayeeModel.h"
#include "model/AttachmentModel.h"

#include "AttachmentDialog.h"

wxIMPLEMENT_DYNAMIC_CLASS(AttachmentDialog, wxDialog);

wxBEGIN_EVENT_TABLE( AttachmentDialog, wxDialog )
    EVT_BUTTON(wxID_CANCEL, AttachmentDialog::OnCancel)
    EVT_BUTTON(wxID_OK, AttachmentDialog::OnOk)
    EVT_BUTTON(wxID_APPLY, AttachmentDialog::OnMagicButton)
    EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, AttachmentDialog::OnListItemSelected)
    EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, AttachmentDialog::OnItemRightClick)
    EVT_MENU_RANGE(MENU_NEW_ATTACHMENT, MENU_DELETE_ATTACHMENT, AttachmentDialog::OnMenuSelected)
    EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, AttachmentDialog::OnListItemActivated)
wxEND_EVENT_TABLE()


AttachmentDialog::AttachmentDialog (wxWindow* parent, const wxString& RefType, int64 RefId, const wxString& name) :
    m_RefType(RefType)
    , m_RefId(RefId)
{
    if (debug_) ColName_[ATTACHMENT_ID] = "#";
    ColName_[ATTACHMENT_DESCRIPTION] = _t("Description");
    ColName_[ATTACHMENT_FILENAME] = _t("File");

    Create(parent, name);
    mmThemeAutoColour(this);

    const wxString AttachmentsFolder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting());

    if (AttachmentsFolder == wxEmptyString)
    {
        wxString msgStr = wxString() << _t("Attachment folder not defined.") << "\n"
            << _tu("Please set it in Tools → Settings… → Attachments") << "\n";
        wxMessageBox(msgStr, _t("Attachment folder not defined"), wxICON_ERROR);
    }
    else if (!wxDirExists(AttachmentsFolder))
    {
        wxString msgStr = wxString() << _t("Unable to find attachments folder:") << "\n"
            << "'" << AttachmentsFolder << "'" << "\n"
            << "\n"
            << _t("Please verify that above path is correct") << "\n";
        wxMessageBox(msgStr, _t("Attachments folder not found."), wxICON_ERROR);
    }
}

void AttachmentDialog::Create(wxWindow* parent, const wxString& name)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    long style = wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER;

    wxString WindowTitle;
    if (m_RefId > 0)
    {
        int refEnum = 0;
        for (int i = 0; i < AttachmentModel::REFTYPE_ID_size; ++i) {
            wxString reftype = ModelBase::reftype_name(i);
            if (reftype == m_RefType)
                break;
            refEnum++;
        }
        wxString RefName;
        switch (refEnum)
        {
        case AttachmentModel::REFTYPE_ID_STOCK:
            RefName = StockModel::get_stock_name(m_RefId);
            break;
        case AttachmentModel::REFTYPE_ID_ASSET:
            RefName = AssetModel::get_asset_name(m_RefId);
            break;
        case AttachmentModel::REFTYPE_ID_BANKACCOUNT:
            RefName = AccountModel::get_account_name(m_RefId);
            break;
        case AttachmentModel::REFTYPE_ID_PAYEE:
            RefName = PayeeModel::get_payee_name(m_RefId);
            break;
        case AttachmentModel::REFTYPE_ID_TRANSACTION:
        case AttachmentModel::REFTYPE_ID_BILLSDEPOSIT:
        default:
            RefName = "";
        }       
        if (RefName.IsEmpty())
            WindowTitle = wxString::Format(_t("Attachment Manager | %1$s | %2$lld"), wxGetTranslation(m_RefType), m_RefId);
        else
            WindowTitle = wxString::Format(_t("Attachment Manager | %1$s | %2$s"), wxGetTranslation(m_RefType), RefName);
    } else
        WindowTitle = wxString::Format(_t("Attachment Manager | New %s"), wxGetTranslation(m_RefType));

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

void AttachmentDialog::CreateControls()
{
    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxVERTICAL);

    attachmentListBox_ = new wxDataViewListCtrl( this
        , wxID_ANY, wxDefaultPosition, wxSize(460, 500)/*, wxDV_HORIZ_RULES*/);

    if (debug_) attachmentListBox_->AppendTextColumn(ColName_[ATTACHMENT_ID], wxDATAVIEW_CELL_INERT, 30);
    attachmentListBox_->AppendTextColumn(ColName_[ATTACHMENT_DESCRIPTION], wxDATAVIEW_CELL_INERT, 150);
    attachmentListBox_->AppendTextColumn(ColName_[ATTACHMENT_FILENAME], wxDATAVIEW_CELL_INERT, 300);
    attachmentListBox_->DragAcceptFiles(true);
    attachmentListBox_->Connect(wxEVT_DROP_FILES, wxDropFilesEventHandler(AttachmentDialog::OnDropFiles), nullptr, this);
    mainBoxSizer->Add(attachmentListBox_, wxSizerFlags(g_flagsExpand).Border(wxALL, 10));

    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    mainBoxSizer->Add(buttons_panel, wxSizerFlags(g_flagsV).Center());
    wxStdDialogButtonSizer* buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxButton* buttonOK = new wxButton(buttons_panel, wxID_OK, _t("&OK "));
    wxButton* btnCancel = new wxButton(buttons_panel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    buttons_sizer->Add(buttonOK, g_flagsH);
    buttons_sizer->Add(btnCancel, g_flagsH);

    wxBitmapButton* magicButton = new wxBitmapButton(buttons_panel
        , wxID_APPLY, mmBitmapBundle(png::MORE_OPTIONS, mmBitmapButtonSize));
    mmToolTip(magicButton, _t("Other tools"));
    buttons_sizer->Add(magicButton, g_flagsH);

    Center();
    this->SetSizer(mainBoxSizer);
}

void AttachmentDialog::fillControls()
{    
    attachmentListBox_->DeleteAllItems();

    AttachmentModel::Data_Set attachments = AttachmentModel::instance().FilterAttachments(m_RefType, m_RefId);
    if (attachments.size() == 0) return;

    int64 firstInTheListAttachentID = -1;
    for (const auto &entry : attachments)
    {
        if (firstInTheListAttachentID == -1) firstInTheListAttachentID = entry.ATTACHMENTID;
        wxVector<wxVariant> data;
        if (debug_) data.push_back(wxVariant(wxString::Format("%lld", entry.ATTACHMENTID)));
        data.push_back(wxVariant(entry.DESCRIPTION));
        data.push_back(wxVariant(entry.REFTYPE + m_PathSep + entry.FILENAME));
        attachmentListBox_->AppendItem(data, static_cast<wxUIntPtr>(entry.ATTACHMENTID.GetValue()));
    }

    m_attachment_id = firstInTheListAttachentID;
}

void AttachmentDialog::AddAttachment(wxString FilePath)
{
    if (FilePath.empty())
    {
        FilePath = wxFileSelector(_t("Import attachment:")
            , wxEmptyString, wxEmptyString, wxEmptyString
            , "All Files |*.*"
            , wxFD_FILE_MUST_EXIST);

        if (FilePath.empty()) return;
    }

    const wxString attachmentFileName = wxFileName(FilePath).GetName();
    const wxString attachmentFileExtension = wxFileName(FilePath).GetExt().MakeLower();
    
    mmDialogComboBoxAutocomplete dlg(this, _t("Enter a description for the new attachment:") + wxString::Format("\n(%s)", FilePath),
        _t("Attachment Manager: Add Attachment"), attachmentFileName, AttachmentModel::instance().allDescriptions());

    if (dlg.ShowModal() != wxID_OK)
        return;

    const wxString attachmentDescription = dlg.getText();

    const wxString attachmentsFolder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting());
    int attachmentLastNumber = AttachmentModel::LastAttachmentNumber(m_RefType, m_RefId);

    wxString importedFileName = m_RefType + "_" + wxString::Format("%lld", m_RefId) + "_Attach"
        + wxString::Format("%i", attachmentLastNumber + 1);
    if (!attachmentFileExtension.empty())
        importedFileName += "." + attachmentFileExtension;

    if (mmAttachmentManage::CopyAttachment(FilePath, attachmentsFolder + m_RefType + m_PathSep + importedFileName))
    {
        AttachmentModel::Data* NewAttachment = AttachmentModel::instance().create();
        NewAttachment->REFTYPE = m_RefType;
        NewAttachment->REFID = m_RefId;
        NewAttachment->DESCRIPTION = attachmentDescription;
        NewAttachment->FILENAME = importedFileName;
        m_attachment_id = AttachmentModel::instance().save(NewAttachment);
        m_attachment_id = NewAttachment->ATTACHMENTID;

        if (m_RefType == TransactionModel::refTypeName)
            TransactionModel::instance().updateTimestamp(m_RefId);
    }

    fillControls();
}

void AttachmentDialog::OpenAttachment()
{
    AttachmentModel::Data *attachments = AttachmentModel::instance().get(m_attachment_id);
    wxString attachmentFilePath = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting())
        + attachments->REFTYPE + m_PathSep + attachments->FILENAME;

    mmAttachmentManage::OpenAttachment(attachmentFilePath);
}

void AttachmentDialog::EditAttachment()
{
    AttachmentModel::Data *attachment = AttachmentModel::instance().get(m_attachment_id);
    if (attachment)
    {
        mmDialogComboBoxAutocomplete dlg(this, _t("Enter a new description for the attachment:"),
            _t("Attachment Manager: Edit Attachment"), attachment->DESCRIPTION,
            AttachmentModel::instance().allDescriptions());

        if (dlg.ShowModal() != wxID_OK)
            return;

        const wxString description = dlg.getText();
        if (description == attachment->DESCRIPTION)
            return;

        attachment->DESCRIPTION = description;
        m_attachment_id = AttachmentModel::instance().save(attachment);
        m_attachment_id = attachment->ATTACHMENTID;

        if (attachment->REFTYPE == TransactionModel::refTypeName)
            TransactionModel::instance().updateTimestamp(attachment->REFID);

        fillControls();
    }
}

void AttachmentDialog::DeleteAttachment()
{
    AttachmentModel::Data *attachment = AttachmentModel::instance().get(m_attachment_id);
    if (attachment)
    {
        int DeleteResponse = wxMessageBox(
            _t("Do you want to delete this attachment?")
            , _t("Confirm Attachment Deletion")
            , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
        if (DeleteResponse == wxYES)
        {
            const wxString AttachmentsFolder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting()) + attachment->REFTYPE;
            if (mmAttachmentManage::DeleteAttachment(AttachmentsFolder + m_PathSep + attachment->FILENAME))
            {
                if (attachment->REFTYPE == TransactionModel::refTypeName)
                    TransactionModel::instance().updateTimestamp(attachment->REFID);
                AttachmentModel::instance().remove(m_attachment_id);
            }
            m_attachment_id = -1;
            fillControls();
        }
    }
}

void AttachmentDialog::OnDropFiles(wxDropFilesEvent& event)
{
    if (event.GetNumberOfFiles() > 0)
    {
        wxString* dropped = event.GetFiles();
        for (int i = 0; i < event.GetNumberOfFiles(); i++)
        {
            wxString FilePath = dropped[i];
            if (wxFileExists(FilePath))
                AddAttachment(FilePath);
        }
    }
}

void AttachmentDialog::OnListItemSelected(wxDataViewEvent& event)
{
    wxDataViewItem item = event.GetItem();
    int selected_index = attachmentListBox_->ItemToRow(item);

    if (selected_index >= 0)
        m_attachment_id = static_cast<int64>(attachmentListBox_->GetItemData(item));
}

void AttachmentDialog::OnListItemActivated(wxDataViewEvent& WXUNUSED(event))
{
    AttachmentModel::Data *attachment = AttachmentModel::instance().get(m_attachment_id);
    const wxString attachmentFilePath = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting())
        + attachment->REFTYPE + m_PathSep + attachment->FILENAME;

    mmAttachmentManage::OpenAttachment(attachmentFilePath);
}

void AttachmentDialog::OnMenuSelected(wxCommandEvent& event)
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

void AttachmentDialog::OnMagicButton(wxCommandEvent& WXUNUSED(event))
{
    wxDataViewEvent evt;
    OnItemRightClick(evt);
}

void AttachmentDialog::OnItemRightClick(wxDataViewEvent& event)
{
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, wxID_ANY) ;
    evt.SetEventObject( this );

    AttachmentModel::Data* attachment = AttachmentModel::instance().get(m_attachment_id);

    wxMenu* mainMenu = new wxMenu;
    if (attachment) mainMenu->SetTitle(attachment->DESCRIPTION);
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_NEW_ATTACHMENT, _t("&Add ")));
    mainMenu->AppendSeparator();
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_OPEN_ATTACHMENT, _t("&Open ")));
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_EDIT_ATTACHMENT, _t("&Edit ")));
    if (!attachment) mainMenu->Enable(MENU_EDIT_ATTACHMENT, false);
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_DELETE_ATTACHMENT, _t("&Remove ")));
    
    //Disable buttons
    const wxString AttachmentsFolder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting());
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

void AttachmentDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void AttachmentDialog::OnOk(wxCommandEvent& /*event*/)
{
    EndModal(wxID_OK);
}


/***********************
** mmAttachmentManage **
************************/
wxString mmAttachmentManage::m_PathSep = wxFileName::GetPathSeparator();

const wxString mmAttachmentManage::InfotablePathSetting()
{
    return InfotableModel::instance().getString("ATTACHMENTSFOLDER:" + mmPlatformType(), "");
}

const wxString mmAttachmentManage::GetAttachmentNoteSign()
{
    return wxString::Format("[%s] ",_t("Att."));
}

bool mmAttachmentManage::CreateReadmeFile(const wxString& FolderPath)
{
    wxString ReadmeFilePath = FolderPath + m_PathSep + "readme.txt";
    wxString ReadmeText;
    ReadmeText << _t("This directory and its files are automatically managed by MMEX.") << wxTextFile::GetEOL();
    ReadmeText << wxTextFile::GetEOL();
    ReadmeText << _t("Please do not remove, rename or modify manually directories and files.") << wxTextFile::GetEOL();

    if (wxFileExists(ReadmeFilePath))
    {
        return true;
    }
    else
    {
        try
        {
            wxFile file(ReadmeFilePath, wxFile::write);

            if (file.IsOpened())
            {
                file.Write(ReadmeText);
                file.Close();
                return true;
            }
        }
        catch (...)
        {
            return false;
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
        const auto &attachments = AttachmentModel::instance().find(AttachmentModel::FILENAME(wxFileNameFromPath(ImportedFile)));
        if (attachments.empty())
        {
            wxString msgStr = wxString() << _t("Destination file already exist:") << "\n"
                << "'" << ImportedFile << "'" << "\n"
                << "\n"
                << _t("File not found in attachments. Please delete or rename it.") << "\n";
            wxMessageBox(msgStr, _t("Destination file already exist"), wxICON_ERROR);
        }
        else
        {
            wxString msgStr = wxString() << _t("Destination file already exist:") << "\n"
                << "'" << ImportedFile << "'" << "\n"
                << "\n"
                << _t("File already found in attachments") << "\n";
            wxMessageBox(msgStr, _t("Destination file already exist"), wxICON_ERROR);
        }
        return false;
    }
    else if (wxCopyFile(FileToImport, ImportedFile))
    {
        if (InfotableModel::instance().getBool("ATTACHMENTSDELETE", false))
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
        if (InfotableModel::instance().getBool("ATTACHMENTSTRASH", false))
        {
            const wxString DeletedAttachmentFolder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting()) + m_PathSep + "Deleted";

            if (!wxDirExists(DeletedAttachmentFolder))
            {
                if (wxMkdir(DeletedAttachmentFolder))
                    mmAttachmentManage::CreateReadmeFile(DeletedAttachmentFolder);
                else
                    return false;
            }

            const wxString FileToTrash = DeletedAttachmentFolder + m_PathSep
                + wxDateTime::Now().FormatISODate() + "_" + wxFileNameFromPath(FileToDelete);

            if (!wxRenameFile(FileToDelete, FileToTrash))
                return false;
        }
        else if (!wxRemoveFile(FileToDelete))
            return false;
    }
    else
    {
        wxString msgStr = wxString() << _t("Attachment not found:") << "\n"
            << "'" << FileToDelete << "'" << "\n"
            << "\n"
            << _t("Do you want to delete the attachment in the database?") << "\n";
        int DeleteResponse = wxMessageBox(msgStr, _t("Delete attachment failed"), wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
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
        wxString msgStr = wxString() << _t("Unable to open file:") << "\n"
            << "'" << FileToOpen << "'" << "\n"
            << "\n"
            << _t("Please verify that file exists and user has rights to read it.") << "\n";
        wxMessageBox(msgStr, _t("Open attachment failed"), wxICON_ERROR);
        return false;
    }

    return wxLaunchDefaultApplication(FileToOpen);;
}

bool mmAttachmentManage::DeleteAllAttachments(const wxString& RefType, int64 RefId)
{
    AttachmentModel::Data_Set attachments = AttachmentModel::instance().FilterAttachments(RefType, RefId);
    wxString AttachmentsFolder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting()) + m_PathSep + RefType;

    for (const auto &entry : attachments)
    {
        mmAttachmentManage::DeleteAttachment(AttachmentsFolder + m_PathSep + entry.FILENAME);
        AttachmentModel::instance().remove(entry.ATTACHMENTID);
    }

    if (RefType == TransactionModel::refTypeName)
        TransactionModel::instance().updateTimestamp(RefId);

    return true;
}

bool mmAttachmentManage::RelocateAllAttachments(const wxString& OldRefType, int64 OldRefId, const wxString& NewRefType, int64 NewRefId)
{
    auto attachments = AttachmentModel::instance().find(AttachmentModel::DB_Table_ATTACHMENT_V1::REFTYPE(OldRefType), AttachmentModel::REFID(OldRefId));

    if (attachments.size() == 0)
        return false;

    const wxString OldAttachmentsFolder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting()) + OldRefType + m_PathSep;
    const wxString NewAttachmentsFolder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting()) + NewRefType + m_PathSep;

    for (auto &entry : attachments)
    {
        wxString NewFileName = entry.FILENAME;
        NewFileName.Replace(entry.REFTYPE + "_" + wxString::Format("%lld", entry.REFID), NewRefType + "_" + wxString::Format("%lld", NewRefId));
        wxRenameFile(OldAttachmentsFolder + entry.FILENAME, NewAttachmentsFolder + NewFileName);
        entry.REFTYPE = NewRefType;
        entry.REFID = NewRefId;
        entry.FILENAME = NewFileName;
    }
    AttachmentModel::instance().save(attachments);

    if (OldRefType == TransactionModel::refTypeName)
        TransactionModel::instance().updateTimestamp(OldRefId);
    if (NewRefType == TransactionModel::refTypeName)
        TransactionModel::instance().updateTimestamp(NewRefId);

    return true;
}

bool mmAttachmentManage::CloneAllAttachments(const wxString& RefType, int64 OldRefId, int64 NewRefId)
{
    auto attachments = AttachmentModel::instance().find(AttachmentModel::DB_Table_ATTACHMENT_V1::REFTYPE(RefType), AttachmentModel::REFID(OldRefId));
    const wxString AttachmentsFolder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting()) + RefType + m_PathSep;

    for (auto &entry : attachments)
    {
        wxString NewFileName = entry.FILENAME;
        NewFileName.Replace(entry.REFTYPE + "_" + wxString::Format("%lld", entry.REFID), entry.REFTYPE + "_" + wxString::Format("%lld", NewRefId));
        wxCopyFile(AttachmentsFolder + entry.FILENAME, AttachmentsFolder + NewFileName);
        AttachmentModel::Data* NewAttachment = AttachmentModel::instance().create();
        NewAttachment->REFTYPE = RefType;
        NewAttachment->REFID = NewRefId;
        NewAttachment->FILENAME = NewFileName;
        NewAttachment->DESCRIPTION = entry.DESCRIPTION;
        AttachmentModel::instance().save(NewAttachment);
    }

    if (RefType == TransactionModel::refTypeName)
        TransactionModel::instance().updateTimestamp(NewRefId);

    return true;
}

void mmAttachmentManage::OpenAttachmentFromPanelIcon(wxWindow* parent, const wxString& RefType, int64 RefId)
{
    int AttachmentsNr = AttachmentModel::instance().NrAttachments(RefType, RefId);

    if (AttachmentsNr == 1)
    {
        AttachmentModel::Data_Set attachments = AttachmentModel::instance().FilterAttachments(RefType, RefId);
        wxString attachmentFilePath = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting())
            + attachments[0].REFTYPE + m_PathSep + attachments[0].FILENAME;
        mmAttachmentManage::OpenAttachment(attachmentFilePath);
    }
    else
    {
        AttachmentDialog dlg(parent, RefType, RefId);
        dlg.ShowModal();
    }
}
