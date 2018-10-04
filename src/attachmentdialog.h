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

#ifndef MM_EX_ATTACHMENTDIALOG_H_
#define MM_EX_ATTACHMENTDIALOG_H_

#include "defs.h"
#include <wx/dataview.h>
#include <map>

class mmAttachmentDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmAttachmentDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmAttachmentDialog(wxWindow* parent, const wxString& RefType, int RefId, const wxString& name = "mmAttachmentDialog");

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

    wxDataViewListCtrl* attachmentListBox_;

    int m_attachment_id;
    std::map<int, wxString> ColName_;
    wxString m_PathSep = wxFileName::GetPathSeparator();

    wxString m_RefType;
    int m_RefId;

    mmAttachmentDialog() : m_attachment_id(-1) {}

    void Create(wxWindow* parent, const wxString& name = "mmAttachmentDialog");
    void CreateControls();
    void fillControls();

    void AddAttachment();
    void OpenAttachment();
    void EditAttachment();
    void DeleteAttachment();
    void OnCancel(wxCommandEvent& WXUNUSED(event));
    void OnOk(wxCommandEvent& WXUNUSED(event));

    void OnListItemSelected(wxDataViewEvent& event);
    void OnMenuSelected(wxCommandEvent& event);
    void OnItemRightClick(wxDataViewEvent& event);
    void OnListItemActivated(wxDataViewEvent& event);
    void OnMagicButton(wxCommandEvent& event);

    bool debug_;
};

class mmAttachmentManage
{
public:
    static wxString InfotablePathSetting();
    static wxString GetAttachmentNoteSign();
    static bool CreateReadmeFile(const wxString& FolderPath);
    static bool CopyAttachment(const wxString& FileToImport, const wxString& ImportedFile);
    static bool DeleteAttachment(const wxString& FileToDelete);
    static bool OpenAttachment(const wxString& FileToOpen);
    static bool DeleteAllAttachments(const wxString& RefType, int RefId);
    static bool RelocateAllAttachments(const wxString& RefType, int OldRefId, int NewRefId);
    static void OpenAttachmentFromPanelIcon(wxWindow* parent, const wxString& RefType, int RefId);
private:
    static wxString m_PathSep;
};

#endif // MM_EX_ATTACHMENTDIALOG_H_
