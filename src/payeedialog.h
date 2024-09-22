/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2012 - 2016, 2020 - 2022 Nikolay Akimov
 Copyright (C) 2021,2022 Mark Whalley (mark@ipx.co.uk)

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
#include "Model_Payee.h"
#include <wx/dataview.h>
#include <wx/srchctrl.h>
#include <wx/grid.h>
#include <map>
#include <list>
#include "mmSimpleDialogs.h"

class mmEditPayeeDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmEditPayeeDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmEditPayeeDialog();
    mmEditPayeeDialog(wxWindow* parent, Model_Payee::Data* payee, const wxString &name = "mmEditPayeeDialog");
    ~mmEditPayeeDialog();

private:
    Model_Payee::Data* m_payee = nullptr;
    wxTextCtrl* m_payeeName = nullptr;
    wxCheckBox* m_hidden = nullptr;
    mmComboBoxCategory* m_category = nullptr;
    wxTextCtrl* m_reference = nullptr;
    wxTextCtrl* m_website = nullptr;
    wxTextCtrl* m_Notes = nullptr;
    wxGrid* m_patternTable = nullptr;
    wxBoxSizer* patternButton_Arranger = nullptr;

    void CreateControls();
    void fillControls();
    void ResizeDialog();
    void OnComboKey(wxKeyEvent& event);
    void OnCancel(wxCommandEvent& /*event*/);
    void OnOk(wxCommandEvent& /*event*/);
    void OnMoveUp(wxCommandEvent& /*event*/);
    void OnMoveDown(wxCommandEvent& /*event*/);
    void OnPatternTableChanged(wxGridEvent& event);
    void OnPatternTableSize(wxSizeEvent& event);

};

class mmPayeeDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmPayeeDialog);
    wxDECLARE_EVENT_TABLE();

public:
    ~mmPayeeDialog();
    mmPayeeDialog(wxWindow* parent, bool payee_choose, const wxString& name = "mmPayeeDialog", const wxString& payee_selected = wxEmptyString);
    void DisableTools();
    int getPayeeId() const;
    bool getRefreshRequested() const;

private:
    enum cols
    {
        PAYEE_NAME = 0,
        PAYEE_HIDDEN,
        PAYEE_CATEGORY,
        PAYEE_NUMBER,
        PAYEE_WEBSITE,
        PAYEE_NOTES,
        PAYEE_PATTERN
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

    wxListView* payeeListBox_ = nullptr;
    wxSearchCtrl* m_maskTextCtrl = nullptr;
    wxBitmapButton* m_magicButton = nullptr;

    int m_payee_id = -1;
    wxString m_init_selected_payee;
    int m_payee_rename = -1;
    bool m_payee_choose = false;
    wxString m_maskStr;
    int m_sort = cols::PAYEE_NAME, m_lastSort = cols::PAYEE_NAME;
    bool refreshRequested_ = false, m_sortReverse = false;
    std::map<int, wxString> ColName_;

private:
    mmPayeeDialog() {}

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
    void FindSelectedPayees(std::list<int>& indexes);
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
