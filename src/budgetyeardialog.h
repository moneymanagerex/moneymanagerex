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

#ifndef _MM_EX_BUDGETYEARDIALOG_H_
#define _MM_EX_BUDGETYEARDIALOG_H_

#define SYMBOL_BUDGETYEARDIALOG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_BUDGETYEARDIALOG_TITLE _("Budget Editor")
#define SYMBOL_BUDGETYEARDIALOG_IDNAME ID_DIALOG_BUDGETYEAR
#define SYMBOL_BUDGETYEARDIALOG_SIZE wxSize(500, 300)
#define SYMBOL_BUDGETYEARDIALOG_POSITION wxDefaultPosition

#include "guiid.h"
#include "mmcoredb.h"

class mmBudgetYearDialog : public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( mmBudgetYearDialog )
    DECLARE_EVENT_TABLE()

public:
    mmBudgetYearDialog();
    mmBudgetYearDialog(mmCoreDB* core, 
        wxWindow* parent, wxWindowID id = SYMBOL_BUDGETYEARDIALOG_IDNAME, 
        const wxString& caption = SYMBOL_BUDGETYEARDIALOG_TITLE, 
        const wxPoint& pos = SYMBOL_BUDGETYEARDIALOG_POSITION, 
        const wxSize& size = SYMBOL_BUDGETYEARDIALOG_SIZE, 
        long style = SYMBOL_BUDGETYEARDIALOG_STYLE );

private:
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_BUDGETYEARDIALOG_IDNAME, 
        const wxString& caption = SYMBOL_BUDGETYEARDIALOG_TITLE, 
        const wxPoint& pos = SYMBOL_BUDGETYEARDIALOG_POSITION, 
        const wxSize& size = SYMBOL_BUDGETYEARDIALOG_SIZE, 
        long style = SYMBOL_BUDGETYEARDIALOG_STYLE );

    void CreateControls();
    
    // utility functions
    void OnOk(wxCommandEvent& event);
    void OnAdd(wxCommandEvent& event);
    void OnAddMonth(wxCommandEvent& /*event*/);
    void OnDelete(wxCommandEvent& event);
    void OnBSelect(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);
    
    void fillControls();
    void OnDoubleClicked(wxCommandEvent& event);

    mmCoreDB* core_;
    wxListBox* listBox_;

    int budgetYearID_;
};

#endif

