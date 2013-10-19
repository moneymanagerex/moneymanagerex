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

#ifndef _MM_EX_BUDGETYEARENTRYDIALOG_H_
#define _MM_EX_BUDGETYEARENTRYDIALOG_H_

#define SYMBOL_BUDGETYEARENTRYDIALOG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_BUDGETYEARENTRYDIALOG_TITLE _("Budget Year Entry")
#define SYMBOL_BUDGETYEARENTRYDIALOG_IDNAME ID_DIALOG_TRANS
#define SYMBOL_BUDGETYEARENTRYDIALOG_SIZE wxSize(500, 300)
#define SYMBOL_BUDGETYEARENTRYDIALOG_POSITION wxDefaultPosition

#include "guiid.h"
#include "defs.h"

class wxSpinCtrl;

class mmBudgetYearEntryDialog : public wxDialog
{
    DECLARE_DYNAMIC_CLASS( mmBudgetYearDialog )
    DECLARE_EVENT_TABLE()

public:
    mmBudgetYearEntryDialog();
    mmBudgetYearEntryDialog(wxWindow* parent,
                            bool withMonth = false,
                            wxWindowID id = SYMBOL_BUDGETYEARENTRYDIALOG_IDNAME,
                            const wxString& caption = SYMBOL_BUDGETYEARENTRYDIALOG_TITLE,
                            const wxPoint& pos = SYMBOL_BUDGETYEARENTRYDIALOG_POSITION,
                            const wxSize& size = SYMBOL_BUDGETYEARENTRYDIALOG_SIZE,
                            long style = SYMBOL_BUDGETYEARENTRYDIALOG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_BUDGETYEARENTRYDIALOG_IDNAME,
                 const wxString& caption = SYMBOL_BUDGETYEARENTRYDIALOG_TITLE,
                 const wxPoint& pos = SYMBOL_BUDGETYEARENTRYDIALOG_POSITION,
                 const wxSize& size = SYMBOL_BUDGETYEARENTRYDIALOG_SIZE,
                 long style = SYMBOL_BUDGETYEARENTRYDIALOG_STYLE );

    void CreateControls();

    // utility functions
    void OnOk(wxCommandEvent& event);

private:
    wxChoice* itemChoice_;
    wxSpinCtrl* textYear_;
    bool withMonth_;
    wxSpinCtrl* textMonth_;

public:
    wxString budgetYear_;
    wxString yearToCopy_;
};

#endif

