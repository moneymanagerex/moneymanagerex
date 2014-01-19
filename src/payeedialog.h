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

#ifndef _MM_EX_PAYEEDIALOG_H_
#define _MM_EX_PAYEEDIALOG_H_

#include "defs.h"
#include <wx/dataview.h>
#include <map>

class mmPayeeDialog : public wxDialog
{
    DECLARE_DYNAMIC_CLASS(mmPayeeDialog)
    DECLARE_EVENT_TABLE()

public:
    mmPayeeDialog(wxWindow* parent);

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
        MENU_DEFINE_CATEGORY = 0,
        NENU_NEW_PAYEE,
        NENU_EDIT_PAYEE,
        MENU_DELETE_PAYEE,
        MENU_RELOCATE_PAYEE
    };

    wxDataViewListCtrl* payeeListBox_;

    int m_payee_id;
    int m_payee_rename;
    int m_selected_index;
    bool refreshRequested_;
    std::map<int, wxString> ColName_;
    wxButton* btnCancel_;
    wxButton* button_OK_;

    mmPayeeDialog() : m_payee_id(-1), refreshRequested_(false) {}

    void do_create(wxWindow* parent);
    void CreateControls();
    void fillControls();

    void AddPayee();
    void EditPayee();
    void DeletePayee();
    void DefineDefaultCategory();
    void OnPayeeRelocate();
    void OnCancel(wxCommandEvent& /*event*/);
    void OnOk(wxCommandEvent& /*event*/);

    void OnListItemSelected(wxDataViewEvent& event);
    void OnDataEditStart(wxDataViewEvent& event);
    void OnDataChanged(wxDataViewEvent& event);
    void OnMenuSelected(wxCommandEvent& event);
    void OnItemRightClick(wxDataViewEvent& event);
};
#endif // _MM_EX_PAYEEDIALOG_H_
