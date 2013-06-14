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

class mmCoreDB;

class mmPayeeDialog : public wxDialog
{
    DECLARE_DYNAMIC_CLASS(mmPayeeDialog)
    DECLARE_EVENT_TABLE()

public:
    mmPayeeDialog(wxWindow* parent, mmCoreDB* core, bool showSelectButton = true);

    int getPayeeId() const { return m_payee_id_; }

private:
    int m_payee_id_;
    mmCoreDB *core_;
    bool showSelectButton_;
    wxListBox* listBox_;
    wxButton* addButton_;
    wxButton* editButton_;
    wxButton* deleteButton_;
    wxButton* selectButton_;
    wxButton* btnCancel_;
    wxTextCtrl* textCtrl_;
    wxTextCtrl* hideTextCtrl_;
    wxCheckBox* cbShowAll_;

    mmPayeeDialog() : m_payee_id_(-1) {}

    void do_create(wxWindow* parent);
    void CreateControls();
    void fillControls();

    void OnAdd(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnBSelect(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);
    void OnListKeyDown(wxKeyEvent &event);
    void OnTextCtrlChanged(wxCommandEvent& event);
    void OnSelChanged(wxCommandEvent& event);
    void OnComboSelected(wxCommandEvent& event);
    void OnPayeeRelocate(wxCommandEvent& event);
    void OnDoubleClicked(wxCommandEvent& event);
    void OnFocus(wxFocusEvent& event);
    void OnSetFocus(wxFocusEvent& event);
    void OnCancel(wxCommandEvent& /*event*/);
    void OnShowHiddenChbClick(wxCommandEvent& /*event*/);
    void saveFilterSettings(wxCommandEvent& event);
};
#endif // _MM_EX_PAYEEDIALOG_H_
