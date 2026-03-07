/*******************************************************
 Copyright (C) 2014 Gabriele-V
 Copyright (C) 2021, 2022 Mark Whalley (mark@ipx.co.uk)

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

#pragma once

#include "base/defs.h"
#include <wx/dataview.h>
#include <map>

#include "util/_primitive.h"
#include "data/_DataEnum.h"

class AttachmentDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(AttachmentDialog);
    wxDECLARE_EVENT_TABLE();

private:
    enum cols
    {
        ATTACHMENT_ID = 0,
        ATTACHMENT_DESCRIPTION,
        ATTACHMENT_FILENAME
    };

    enum menu_items
    {
        MENU_NEW_ATTACHMENT = 1,
        MENU_OPEN_ATTACHMENT,
        MENU_EDIT_ATTACHMENT,
        MENU_DELETE_ATTACHMENT,
    };

private:
    int64 m_attachment_id = -1;
    RefTypeN m_ref_type;
    int64 m_ref_id = -1;
    std::map<int, wxString> ColName_;
    wxString m_PathSep = wxFileName::GetPathSeparator();

    //wxButton* btnCancel_ = nullptr;
    //wxButton* button_OK_ = nullptr;
    wxDataViewListCtrl* attachmentListBox_ = nullptr;

    #ifdef _DEBUG
        bool debug_ = true;
    #else
        bool debug_ = false;
    #endif

public:
    AttachmentDialog() {}
    AttachmentDialog(
        wxWindow* parent,
        RefTypeN ref_type,
        int64 ref_id,
        const wxString& name = "AttachmentDialog"
    );

private:
    void Create(wxWindow* parent, const wxString& name);
    void CreateControls();
    void fillControls();

    void AddAttachment(wxString Path = "");
    void OpenAttachment();
    void EditAttachment();
    void DeleteAttachment();
    void OnCancel(wxCommandEvent& /*event*/);
    void OnOk(wxCommandEvent& /*event*/);

    void OnDropFiles(wxDropFilesEvent& event);
    void OnListItemSelected(wxDataViewEvent& event);
    void OnMenuSelected(wxCommandEvent& event);
    void OnItemRightClick(wxDataViewEvent& event);
    void OnListItemActivated(wxDataViewEvent& event);
    void OnMagicButton(wxCommandEvent& event);
};

// TODO: move to AttachmentModel
class mmAttachmentManage
{
private:
    static wxString m_PathSep;

public:
    static const wxString InfotablePathSetting();
    static const wxString GetAttachmentNoteSign();
    static bool CreateReadmeFile(const wxString& FolderPath);
    static bool CopyAttachment(const wxString& FileToImport, const wxString& ImportedFile);
    static bool DeleteAttachment(const wxString& FileToDelete);
    static bool OpenAttachment(const wxString& FileToOpen);

    static bool DeleteAllAttachments(RefTypeN ref_type, int64 ref_id);
    static bool RelocateAllAttachments(
        RefTypeN old_ref_type, int64 old_ref_id,
        RefTypeN new_ref_type, int64 new_ref_id
    );
    static bool CloneAllAttachments(RefTypeN ref_type, int64 src_ref_id, int64 dst_ref_id);
    static void OpenAttachmentFromPanelIcon(
        wxWindow* parent,
        RefTypeN ref_type, int64 ref_id
    );
};
