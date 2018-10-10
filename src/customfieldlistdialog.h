/*******************************************************
 Copyright (C) 2016 Gabriele-V

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

#ifndef MM_EX_CUSTOMFIELDLISTDIALOG_H_
#define MM_EX_CUSTOMFIELDLISTDIALOG_H_

#include <wx/dialog.h>
#include <map>
class wxDataViewListCtrl;
class wxDataViewEvent;

class mmCustomFieldListDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmCustomFieldListDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmCustomFieldListDialog(wxWindow* parent, const wxString& RefType);

private:
    enum cols
    {
        FIELD_ID = 0,
        FIELD_DESCRIPTION,
        FIELD_TYPE,
        FIELD_PROPERTIES
    };

    enum menu_items
    {
        MENU_NEW_FIELD = 1,
        MENU_EDIT_FIELD,
        MENU_DELETE_FIELD,
        MENU_UPDATE_FIELD,
    };

    wxDataViewListCtrl* fieldListBox_;

    wxString m_RefType;
    int m_field_id;
    mmCustomFieldListDialog() : m_field_id(-1) {}
    std::map<int, wxString> ColName_;

    void Create(wxWindow* parent);
    void CreateControls();
    void fillControls();

    void AddField();
    void EditField();
    void DeleteField();
    void UpdateField();
    void OnClose(wxCommandEvent& WXUNUSED(event));

    void OnListItemSelected(wxDataViewEvent& event);
    void OnMenuSelected(wxCommandEvent& event);
    void OnItemRightClick(wxDataViewEvent& event);
    void OnListItemActivated(wxDataViewEvent& event);
    void OnMagicButton(wxCommandEvent& event);

    bool debug_;
};

#endif // MM_EX_CUSTOMFIELDLISTDIALOG_H_
