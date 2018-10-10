/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#ifndef MM_EX_PAYEEDIALOG_H_
#define MM_EX_PAYEEDIALOG_H_

#include <wx/dialog.h>
#include <map>
class wxDataViewListCtrl;
class wxDataViewEvent;
class wxSearchCtrl;

class mmPayeeDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmPayeeDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmPayeeDialog(wxWindow* parent, bool payee_choose, const wxString &name = "mmPayeeDialog");

    int getPayeeId() const {return m_payee_id;}
    bool getRefreshRequested() const {return refreshRequested_;}

private:
    enum cols
    {
        PAYEE_ID = 0,
        PAYEE_NAME,
        PAYEE_CATEGORY
    };

    enum menu_items
    {
        MENU_DEFINE_CATEGORY = 1,
        MENU_NEW_PAYEE,
        MENU_EDIT_PAYEE,
        MENU_DELETE_PAYEE,
        MENU_ORGANIZE_ATTACHMENTS,
        MENU_RELOCATE_PAYEE
    };

    wxDataViewListCtrl* payeeListBox_;
    wxSearchCtrl* m_maskTextCtrl;

    int m_payee_id;
    int m_payee_rename;
    bool m_payee_choose;
    wxString m_maskStr;
    bool refreshRequested_;
    std::map<int, wxString> ColName_;

    mmPayeeDialog() : m_payee_id(-1), refreshRequested_(false) {}

    void Create(wxWindow* parent, const wxString &name);
    void CreateControls();
    void fillControls();

    void AddPayee();
    void EditPayee();
    void DeletePayee();
    void DefineDefaultCategory();
    void OnOrganizeAttachments();
    void OnPayeeRelocate();
    void OnCancel(wxCommandEvent& WXUNUSED(event));
    void OnOk(wxCommandEvent& WXUNUSED(event));

    void OnListItemSelected(wxDataViewEvent& event);
    void OnListItemActivated(wxDataViewEvent& event);
    void OnDataEditStart(wxDataViewEvent& event);
    void OnDataChanged(wxDataViewEvent& event);
    void OnMenuSelected(wxCommandEvent& event);
    void OnItemRightClick(wxDataViewEvent& event);
    void OnTextChanged(wxCommandEvent& event);
    void OnMagicButton(wxCommandEvent& event);

    bool debug_;
};

#endif // MM_EX_PAYEEDIALOG_H_
