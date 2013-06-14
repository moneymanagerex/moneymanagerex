/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011 Stefano Giorgio

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

#ifndef _MM_EX_RELOCATECATEGORYDIALOG_H_
#define _MM_EX_RELOCATECATEGORYDIALOG_H_

#define SYMBOL_RELOCATECATEGORYDIALOG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_RELOCATECATEGORYDIALOG_TITLE _("Relocate Category Dialog")
#define SYMBOL_RELOCATECATEGORYDIALOG_IDNAME ID_DIALOG_BUDGETENTRY
#define SYMBOL_RELOCATECATEGORYDIALOG_SIZE wxSize(500, 300)
#define SYMBOL_RELOCATECATEGORYDIALOG_POSITION wxDefaultPosition

#include "guiid.h"
#include "defs.h"
#include "dbwrapper.h"
#include "mmcoredb.h"

class relocateCategoryDialog : public wxDialog
{
    DECLARE_DYNAMIC_CLASS( relocateCategoryDialog )
    DECLARE_EVENT_TABLE()

public:
    relocateCategoryDialog();
    relocateCategoryDialog( mmCoreDB* core,
        wxWindow* parent, int sourceCatID = -1, int sourceSubCatID = -1,
        wxWindowID id = SYMBOL_RELOCATECATEGORYDIALOG_IDNAME,
        const wxString& caption = SYMBOL_RELOCATECATEGORYDIALOG_TITLE,
        const wxPoint& pos = SYMBOL_RELOCATECATEGORYDIALOG_POSITION,
        const wxSize& size = SYMBOL_RELOCATECATEGORYDIALOG_SIZE,
        long style = SYMBOL_RELOCATECATEGORYDIALOG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_RELOCATECATEGORYDIALOG_IDNAME,
        const wxString& caption = SYMBOL_RELOCATECATEGORYDIALOG_TITLE,
        const wxPoint& pos = SYMBOL_RELOCATECATEGORYDIALOG_POSITION,
        const wxSize& size = SYMBOL_RELOCATECATEGORYDIALOG_SIZE,
        long style = SYMBOL_RELOCATECATEGORYDIALOG_STYLE );

    void CreateControls();

    // utility functions
    void OnSelectSource(wxCommandEvent& event);
    void OnSelectDest(wxCommandEvent& event);
    void OnOk(wxCommandEvent& event);

    wxString updatedCategoriesCount();

private:
    mmCoreDB* core_;

    int sourceCatID_;
    int sourceSubCatID_;
    wxButton* sourceBtn_;

    int destCatID_;
    int destSubCatID_;
    wxButton* destBtn_;
    int changedCats_;
    int changedSubCats_;
};

#endif

