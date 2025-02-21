/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015 Gabriele-V
 Copyright (C) 2022 Mark WHalley (mark@ipx.co.uk)

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

#ifndef MM_EX_TAGDIALOG_H_
#define MM_EX_TAGDIALOG_H_

#include "defs.h"
#include "wx/dialog.h"
#include <wx/srchctrl.h>
#include <wx/listbox.h>
#include <wx/checklst.h>
#include <wx/listbase.h>

class mmTagDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmTagDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmTagDialog();
    ~mmTagDialog();
    mmTagDialog(wxWindow* parent, bool isSelection = false, const wxArrayString& selectedTags = wxArrayString());

    bool getRefreshRequested() const;
    wxArrayString getSelectedTags() const;

private:
    bool Create(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxString& caption = _t("Tag Manager"),
        const wxString& name = "Organize Tags",
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX | wxRESIZE_BORDER);

    void CreateControls();

    void OnAdd(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnTextChanged(wxCommandEvent& event);
    void OnListSelChanged(wxCommandEvent& event);
    void OnCheckboxSelChanged(wxCommandEvent& event);
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void fillControls();
    bool refreshRequested_ = false;
    bool validateName(const wxString& name);
    void setSelectedItem(int index);
    void setSelectedString(const wxString& name);

    wxSearchCtrl* searchCtrl_ = nullptr;
    wxString searchText_;
    wxListBoxBase* tagListBox_ = nullptr;
    wxButton* buttonEdit_ = nullptr;
    wxButton* buttonAdd_ = nullptr;
    wxButton* buttonDelete_ = nullptr;
    bool isSelection_;
    wxArrayString tagList_;
    wxArrayString selectedTags_;
    wxString mask_string_ = wxEmptyString;
};

inline bool mmTagDialog::getRefreshRequested() const { return refreshRequested_; }
inline wxArrayString mmTagDialog::getSelectedTags() const { return selectedTags_; }
#endif
