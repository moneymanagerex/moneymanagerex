/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2012 - 2016, 2020 - 2022 Nikolay Akimov
 Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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

#include "defs.h"
#include <wx/dataview.h>
#include <wx/srchctrl.h>
#include <map>

class mmPayeeDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmPayeeDialog);
    wxDECLARE_EVENT_TABLE();

public:
    ~mmPayeeDialog();
    mmPayeeDialog(wxWindow* parent, bool payee_choose, const wxString &name = "mmPayeeDialog");
    void DisableTools();
    int getPayeeId() const;
    bool getRefreshRequested() const;

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
        MENU_REMOVE_CATEGORY,
        MENU_NEW_PAYEE,
        MENU_EDIT_PAYEE,
        MENU_DELETE_PAYEE,
        MENU_ORGANIZE_ATTACHMENTS,
        MENU_RELOCATE_PAYEE
    };

    wxListView* payeeListBox_;
    wxSearchCtrl* m_maskTextCtrl;
    wxBitmapButton* m_magicButton;

    int m_payee_id;
    int m_payee_rename;
    bool m_payee_choose;
    wxString m_maskStr;
    bool refreshRequested_, m_sortByPayee, m_sortReverse;
    std::map<int, wxString> ColName_;

private:
    mmPayeeDialog() : m_payee_id(-1), refreshRequested_(false) {}

    void Create(wxWindow* parent, const wxString &name);
    void CreateControls();
    void fillControls();

    void AddPayee();
    void EditPayee();
    void DeletePayee();
    void DefineDefaultCategory();
    void RemoveDefaultCategory();
    void OnOrganizeAttachments();
    void OnPayeeRelocate();
    int FindSelectedPayee();
    void OnCancel(wxCommandEvent& /*event*/);
    void OnOk(wxCommandEvent& /*event*/);

    void OnListItemActivated(wxListEvent& event);
    void OnListItemSelected(wxListEvent& event);
    void OnListItemDeselected(wxListEvent& event);
    void OnMenuSelected(wxCommandEvent& event);
    void OnItemRightClick(wxListEvent& event);
    void OnTextChanged(wxCommandEvent& event);
    void OnMagicButton(wxCommandEvent& event);
    void OnSort(wxListEvent& event);
};

inline void mmPayeeDialog::DisableTools() { m_magicButton->Disable(); }
inline int mmPayeeDialog::getPayeeId() const { return m_payee_id; }
inline bool mmPayeeDialog::getRefreshRequested() const { return refreshRequested_; }
inline void mmPayeeDialog::OnListItemDeselected(wxListEvent& WXUNUSED(event)) { m_payee_id = -1; }


#endif // MM_EX_PAYEEDIALOG_H_
