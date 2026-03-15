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

#include "model/InfoModel.h"
#include "model/AssetModel.h"
#include "model/StockModel.h"
#include "model/PayeeModel.h"
#include "model/AttachmentModel.h"

#include "AttachmentDialog.h"

wxIMPLEMENT_DYNAMIC_CLASS(AttachmentDialog, wxDialog);

wxBEGIN_EVENT_TABLE( AttachmentDialog, wxDialog )
    EVT_BUTTON(wxID_CANCEL,                  AttachmentDialog::OnCancel)
    EVT_BUTTON(wxID_OK,                      AttachmentDialog::OnOk)
    EVT_BUTTON(wxID_APPLY,                   AttachmentDialog::OnMagicButton)
    EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, AttachmentDialog::OnListItemSelected)
    EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, AttachmentDialog::OnItemRightClick)
    EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY,    AttachmentDialog::OnListItemActivated)
    EVT_MENU_RANGE(MENU_NEW_ATTACHMENT, MENU_DELETE_ATTACHMENT,
        AttachmentDialog::OnMenuSelected
    )
wxEND_EVENT_TABLE()

AttachmentDialog::AttachmentDialog(
    wxWindow* parent,
    RefTypeN ref_type,
    int64 ref_id,
    const wxString& name
) :
    m_ref_type(ref_type),
    m_ref_id(ref_id)
{
    if (debug_)
        ColName_[ATTACHMENT_ID] = "#";
    ColName_[ATTACHMENT_DESCRIPTION] = _t("Description");
    ColName_[ATTACHMENT_FILENAME] = _t("File");

    Create(parent, name);
    mmThemeAutoColour(this);

    const wxString folder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting());

    if (folder == wxEmptyString) {
        wxString msgStr = wxString() << _t("Attachment folder not defined.") << "\n"
            << _tu("Please set it in Tools → Settings… → Attachments") << "\n";
        wxMessageBox(msgStr, _t("Attachment folder not defined"), wxICON_ERROR);
    }
    else if (!wxDirExists(folder)) {
        wxString msgStr = wxString() << _t("Unable to find attachments folder:") << "\n"
            << "'" << folder << "'" << "\n"
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
    if (m_ref_id > 0) {
        wxString ref_name;
        switch (m_ref_type.id_n()) {
        case RefTypeN::e_stock:
            ref_name = StockModel::instance().get_id_name(m_ref_id);
            break;
        case RefTypeN::e_asset:
            ref_name = AssetModel::instance().get_id_name(m_ref_id);
            break;
        case RefTypeN::e_account:
            ref_name = AccountModel::instance().get_id_name(m_ref_id);
            break;
        case RefTypeN::e_payee:
            ref_name = PayeeModel::instance().get_id_name(m_ref_id);
            break;
        case RefTypeN::e_trx:
        case RefTypeN::e_sched:
        default:
            ref_name = "";
        }       
        if (ref_name.IsEmpty())
            WindowTitle = wxString::Format(_t("Attachment Manager | %1$s | %2$lld"),
                wxGetTranslation(m_ref_type.name_n()), m_ref_id
            );
        else
            WindowTitle = wxString::Format(_t("Attachment Manager | %1$s | %2$s"),
                wxGetTranslation(m_ref_type.name_n()), ref_name
            );
    } else
        WindowTitle = wxString::Format(_t("Attachment Manager | New %s"),
            wxGetTranslation(m_ref_type.name_n())
        );

    if (!wxDialog::Create(
        parent, wxID_ANY, WindowTitle,
        wxDefaultPosition, wxDefaultSize, style, name
    ))
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

    attachmentListBox_ = new wxDataViewListCtrl(this, wxID_ANY,
        wxDefaultPosition, wxSize(460, 500)/*, wxDV_HORIZ_RULES*/
    );

    if (debug_)
        attachmentListBox_->AppendTextColumn(ColName_[ATTACHMENT_ID], wxDATAVIEW_CELL_INERT, 30);
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

    AttachmentModel::DataA att_a = AttachmentModel::instance().find_ref_data_a(
        m_ref_type, m_ref_id
    );
    if (att_a.empty())
        return;

    int64 firstInTheListAttachentID = -1;
    for (const auto& att_d : att_a) {
        if (firstInTheListAttachentID == -1)
            firstInTheListAttachentID = att_d.m_id;
        wxVector<wxVariant> data;
        if (debug_)
            data.push_back(wxVariant(wxString::Format("%lld", att_d.m_id)));
        data.push_back(wxVariant(att_d.m_description));
        data.push_back(wxVariant(att_d.m_ref_type_n.name_n() + m_PathSep + att_d.m_filename));
        attachmentListBox_->AppendItem(data, static_cast<wxUIntPtr>(att_d.m_id.GetValue()));
    }

    m_attachment_id = firstInTheListAttachentID;
}

void AttachmentDialog::AddAttachment(wxString file_path)
{
    if (file_path.empty()) {
        file_path = wxFileSelector(
            _t("Import attachment:"),
            wxEmptyString, wxEmptyString, wxEmptyString,
            "All Files |*.*",
            wxFD_FILE_MUST_EXIST
        );
        if (file_path.empty())
            return;
    }

    const wxString file_name = wxFileName(file_path).GetName();
    const wxString file_ext = wxFileName(file_path).GetExt().MakeLower();
    
    mmDialogComboBoxAutocomplete dlg(this,
        _t("Enter a description for the new attachment:") +
            wxString::Format("\n(%s)", file_path),
        _t("Attachment Manager: Add Attachment"),
        file_name,
        AttachmentModel::instance().find_all_desc_a()
    );

    if (dlg.ShowModal() != wxID_OK)
        return;

    const wxString desc = dlg.getText();

    const wxString folder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting());
    int last_num = AttachmentModel::instance().find_ref_last_num(m_ref_type, m_ref_id);

    wxString importedFileName = m_ref_type.name_n() + "_" + wxString::Format("%lld", m_ref_id) + "_Attach"
        + wxString::Format("%i", last_num + 1);
    if (!file_ext.empty())
        importedFileName += "." + file_ext;

    if (mmAttachmentManage::CopyAttachment(
        file_path,
        folder + m_ref_type.name_n() + m_PathSep + importedFileName
    )) {
        AttachmentData new_att_d = AttachmentData();
        new_att_d.m_ref_type_n  = m_ref_type;
        new_att_d.m_ref_id      = m_ref_id;
        new_att_d.m_description = desc;
        new_att_d.m_filename    = importedFileName;
        AttachmentModel::instance().add_data_n(new_att_d);
        m_attachment_id = new_att_d.m_id;

        if (m_ref_type == TrxModel::s_ref_type)
            TrxModel::instance().save_timestamp(m_ref_id);
    }

    fillControls();
}

void AttachmentDialog::OpenAttachment()
{
    const AttachmentData* att_n = AttachmentModel::instance().get_id_data_n(m_attachment_id);
    wxString file_path = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting())
        + att_n->m_ref_type_n.name_n() + m_PathSep + att_n->m_filename;
    mmAttachmentManage::OpenAttachment(file_path);
}

void AttachmentDialog::EditAttachment()
{
    AttachmentData *att_n = AttachmentModel::instance().unsafe_get_id_data_n(m_attachment_id);
    if (!att_n)
        return;

    mmDialogComboBoxAutocomplete dlg(this,
        _t("Enter a new description for the attachment:"),
        _t("Attachment Manager: Edit Attachment"),
        att_n->m_description,
        AttachmentModel::instance().find_all_desc_a()
    );

    if (dlg.ShowModal() != wxID_OK)
        return;

    const wxString desc = dlg.getText();
    if (desc == att_n->m_description)
        return;

    att_n->m_description = desc;
    AttachmentModel::instance().unsafe_update_data_n(att_n);
    m_attachment_id = att_n->id();

    if (att_n->m_ref_type_n == TrxModel::s_ref_type)
        TrxModel::instance().save_timestamp(att_n->m_ref_id);

    fillControls();
}

void AttachmentDialog::DeleteAttachment()
{
    const AttachmentData* att_n = AttachmentModel::instance().get_id_data_n(m_attachment_id);
    if (!att_n)
        return;

    int deleteResponse = wxMessageBox(
        _t("Do you want to delete this attachment?"),
        _t("Confirm Attachment Deletion"),
        wxYES_NO | wxNO_DEFAULT | wxICON_ERROR
    );
    if (deleteResponse == wxYES) {
        const wxString AttachmentsFolder = mmex::getPathAttachment(
            mmAttachmentManage::InfotablePathSetting()
        ) + att_n->m_ref_type_n.name_n();
        if (mmAttachmentManage::DeleteAttachment(
            AttachmentsFolder + m_PathSep + att_n->m_filename
        )) {
            if (att_n->m_ref_type_n == TrxModel::s_ref_type)
                TrxModel::instance().save_timestamp(att_n->m_ref_id);
            AttachmentModel::instance().purge_id(m_attachment_id);
        }
        m_attachment_id = -1;
        fillControls();
    }
}

void AttachmentDialog::OnDropFiles(wxDropFilesEvent& event)
{
    if (event.GetNumberOfFiles() > 0) {
        wxString* dropped = event.GetFiles();
        for (int i = 0; i < event.GetNumberOfFiles(); i++) {
            wxString file_path = dropped[i];
            if (wxFileExists(file_path))
                AddAttachment(file_path);
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
    const AttachmentData* att_n = AttachmentModel::instance().get_id_data_n(m_attachment_id);
    const wxString path = mmex::getPathAttachment(
        mmAttachmentManage::InfotablePathSetting()
    ) + att_n->m_ref_type_n.name_n() + m_PathSep + att_n->m_filename;

    mmAttachmentManage::OpenAttachment(path);
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

    const AttachmentData* att_n = AttachmentModel::instance().get_id_data_n(m_attachment_id);

    wxMenu* mainMenu = new wxMenu;
    if (att_n)
        mainMenu->SetTitle(att_n->m_description);
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_NEW_ATTACHMENT, _t("&Add ")));
    mainMenu->AppendSeparator();
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_OPEN_ATTACHMENT, _t("&Open ")));
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_EDIT_ATTACHMENT, _t("&Edit ")));
    if (!att_n)
        mainMenu->Enable(MENU_EDIT_ATTACHMENT, false);
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_DELETE_ATTACHMENT, _t("&Remove ")));
    
    //Disable buttons
    const wxString AttachmentsFolder = mmex::getPathAttachment(
        mmAttachmentManage::InfotablePathSetting()
    );
    if (AttachmentsFolder == wxEmptyString || !wxDirExists(AttachmentsFolder))
        mainMenu->Enable(MENU_NEW_ATTACHMENT, false);

    if (!att_n) {
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
    return InfoModel::instance().getString("ATTACHMENTSFOLDER:" + mmPlatformType(), "");
}

const wxString mmAttachmentManage::GetAttachmentNoteSign()
{
    return wxString::Format("[%s] ",_t("Att."));
}

bool mmAttachmentManage::CreateReadmeFile(const wxString& folder)
{
    wxString readme_path = folder + m_PathSep + "readme.txt";
    if (wxFileExists(readme_path))
        return true;

    wxString readme_text;
    readme_text << _t("This directory and its files are automatically managed by MMEX.")
        << wxTextFile::GetEOL();
    readme_text << wxTextFile::GetEOL();
    readme_text << _t("Please do not remove, rename or modify manually directories and files.")
        << wxTextFile::GetEOL();

    bool ok = false;
    try {
        wxFile file(readme_path, wxFile::write);
        if (file.IsOpened()) {
            file.Write(readme_text);
            file.Close();
            ok = true;
        }
    }
    catch (...) {
    }

    return ok;
}

bool mmAttachmentManage::CopyAttachment(
    const wxString& FileToImport,
    const wxString& ImportedFile
) {
    wxString destinationFolder = wxPathOnly(ImportedFile);

    if (!wxDirExists(destinationFolder)) {
        if (wxMkdir(destinationFolder))
            mmAttachmentManage::CreateReadmeFile(destinationFolder);
        else
            return false;
    }

    if (wxFileExists(ImportedFile)) {
        const auto &attachments = AttachmentModel::instance().find(
            AttachmentCol::FILENAME(wxFileNameFromPath(ImportedFile))
        );
        if (attachments.empty()) {
            wxString msgStr = wxString() << _t("Destination file already exist:") << "\n"
                << "'" << ImportedFile << "'" << "\n"
                << "\n"
                << _t("File not found in attachments. Please delete or rename it.") << "\n";
            wxMessageBox(msgStr, _t("Destination file already exist"), wxICON_ERROR);
        }
        else {
            wxString msgStr = wxString() << _t("Destination file already exist:") << "\n"
                << "'" << ImportedFile << "'" << "\n"
                << "\n"
                << _t("File already found in attachments") << "\n";
            wxMessageBox(msgStr, _t("Destination file already exist"), wxICON_ERROR);
        }
        return false;
    }
    else if (wxCopyFile(FileToImport, ImportedFile)) {
        if (InfoModel::instance().getBool("ATTACHMENTSDELETE", false))
            wxRemoveFile(FileToImport);
    }
    else
        return false;

    return true;
}

bool mmAttachmentManage::DeleteAttachment(const wxString& FileToDelete)
{
    if (wxFileExists(FileToDelete)) {
        if (InfoModel::instance().getBool("ATTACHMENTSTRASH", false)) {
            const wxString folder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting());
            const wxString folder_deleted = folder + m_PathSep + "Deleted";

            if (!wxDirExists(folder_deleted)) {
                if (wxMkdir(folder_deleted))
                    mmAttachmentManage::CreateReadmeFile(folder_deleted);
                else
                    return false;
            }

            const wxString FileToTrash = folder_deleted + m_PathSep
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
        int DeleteResponse = wxMessageBox(
            msgStr,
            _t("Delete attachment failed"),
            wxYES_NO | wxNO_DEFAULT | wxICON_ERROR
        );
        if (DeleteResponse == wxYES)
            return true;
        else
            return false;
    }
    return true;
}

bool mmAttachmentManage::OpenAttachment(const wxString& FileToOpen)
{
    if (!wxFileExists(FileToOpen)) {
        wxString msgStr = wxString() << _t("Unable to open file:") << "\n"
            << "'" << FileToOpen << "'" << "\n"
            << "\n"
            << _t("Please verify that file exists and user has rights to read it.") << "\n";
        wxMessageBox(msgStr, _t("Open attachment failed"), wxICON_ERROR);
        return false;
    }

    return wxLaunchDefaultApplication(FileToOpen);;
}

bool mmAttachmentManage::DeleteAllAttachments(RefTypeN ref_type, int64 ref_id)
{
    wxString folder = mmex::getPathAttachment(
        mmAttachmentManage::InfotablePathSetting()
    ) + m_PathSep + ref_type.name_n();

    for (const AttachmentData& att_d : AttachmentModel::instance().find_ref_data_a(
        ref_type, ref_id
    )) {
        mmAttachmentManage::DeleteAttachment(folder + m_PathSep + att_d.m_filename);
        AttachmentModel::instance().purge_id(att_d.m_id);
    }

    if (ref_type.id_n() == TrxModel::s_ref_type.id_n())
        TrxModel::instance().save_timestamp(ref_id);

    return true;
}

bool mmAttachmentManage::RelocateAllAttachments(
    RefTypeN old_ref_type, int64 old_ref_id,
    RefTypeN new_ref_type, int64 new_ref_id
) {
    auto att_a = AttachmentModel::instance().find(
        AttachmentCol::REFTYPE(old_ref_type.name_n()),
        AttachmentCol::REFID(old_ref_id)
    );

    if (att_a.size() == 0)
        return false;

    const wxString old_folder = mmex::getPathAttachment(
        mmAttachmentManage::InfotablePathSetting()
    ) + old_ref_type.name_n() + m_PathSep;
    const wxString new_folder = mmex::getPathAttachment(
        mmAttachmentManage::InfotablePathSetting()
    ) + new_ref_type.name_n() + m_PathSep;

    for (auto& att_d : att_a) {
        wxString newFileName = att_d.m_filename;
        newFileName.Replace(
            att_d.m_ref_type_n.name_n() + "_" + wxString::Format("%lld", att_d.m_ref_id),
            new_ref_type.name_n() + "_" + wxString::Format("%lld", new_ref_id)
        );
        wxRenameFile(
            old_folder + att_d.m_filename,
            new_folder + newFileName
        );
        att_d.m_ref_type_n = new_ref_type;
        att_d.m_ref_id     = new_ref_id;
        att_d.m_filename   = newFileName;
    }
    AttachmentModel::instance().save_data_a(att_a);

    if (old_ref_type.id_n() == TrxModel::s_ref_type.id_n())
        TrxModel::instance().save_timestamp(old_ref_id);
    if (new_ref_type.id_n() == TrxModel::s_ref_type.id_n())
        TrxModel::instance().save_timestamp(new_ref_id);

    return true;
}

bool mmAttachmentManage::CloneAllAttachments(
    RefTypeN ref_type,
    int64 src_ref_id,
    int64 dst_ref_id
) {
    const wxString folder = mmex::getPathAttachment(
        mmAttachmentManage::InfotablePathSetting()
    ) + ref_type.name_n() + m_PathSep;

    for (auto& src_att_d : AttachmentModel::instance().find(
        AttachmentCol::REFTYPE(ref_type.name_n()),
        AttachmentCol::REFID(src_ref_id)
    )) {
        wxString dst_filename = src_att_d.m_filename;
        dst_filename.Replace(
            src_att_d.m_ref_type_n.name_n() + "_" + wxString::Format("%lld", src_att_d.m_ref_id),
            src_att_d.m_ref_type_n.name_n() + "_" + wxString::Format("%lld", dst_ref_id)
        );
        wxCopyFile(folder + src_att_d.m_filename, folder + dst_filename);
        AttachmentData new_att_d = AttachmentData();
        new_att_d.m_ref_type_n  = ref_type;
        new_att_d.m_ref_id      = dst_ref_id;
        new_att_d.m_filename    = dst_filename;
        new_att_d.m_description = src_att_d.m_description;
        AttachmentModel::instance().add_data_n(new_att_d);
    }

    if (ref_type.id_n() == TrxModel::s_ref_type.id_n())
        TrxModel::instance().save_timestamp(dst_ref_id);

    return true;
}

void mmAttachmentManage::OpenAttachmentFromPanelIcon(
    wxWindow* parent,
    RefTypeN ref_type,
    int64 ref_id
) {
    int att_c = AttachmentModel::instance().find_ref_c(ref_type, ref_id);

    if (att_c == 1) {
        AttachmentModel::DataA att_a = AttachmentModel::instance().find_ref_data_a(
            ref_type, ref_id
        );
        wxString file_path = mmex::getPathAttachment(
            mmAttachmentManage::InfotablePathSetting()
        ) + att_a[0].m_ref_type_n.name_n() + m_PathSep + att_a[0].m_filename;
        mmAttachmentManage::OpenAttachment(file_path);
    }
    else {
        AttachmentDialog dlg(parent, ref_type, ref_id);
        dlg.ShowModal();
    }
}
