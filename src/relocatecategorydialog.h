/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011 Stefano Giorgio
 Copyright (C) 2016, 2020, 2022 Nikolay Akimov
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

#ifndef MM_EX_RELOCATECATEGORYDIALOG_H_
#define MM_EX_RELOCATECATEGORYDIALOG_H_

#include "mmSimpleDialogs.h"
#include <wx/dialog.h>
class wxButton;
class wxStaticText;
class wxButton;

class relocateCategoryDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(relocateCategoryDialog);
    wxDECLARE_EVENT_TABLE();

public:
    relocateCategoryDialog();
    relocateCategoryDialog(wxWindow* parent, int sourceCatID = -1, int sourceSubCatID = -1);

    bool Create(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , const wxString& caption = _("Relocate Category Dialog")
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX );

    void CreateControls();

    // utility functions
    void OnTextUpdated(wxCommandEvent& event);
    void OnFocusChange(wxChildFocusEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnOk(wxCommandEvent& event);

    int updatedCategoriesCount() const;

private:
    void IsOkOk();
    int m_sourceCatID;
    int m_sourceSubCatID;
    mmComboBoxCategory* cbSourceCategory_;
    mmComboBoxCategory* cbDestCategory_;
    wxCheckBox* cbDeleteSourceCategory_;
    wxStaticText* m_info;

    wxButton* m_buttonDest;
    int m_changedRecords;
};

inline int relocateCategoryDialog::updatedCategoriesCount() const { return m_changedRecords; }

#endif

