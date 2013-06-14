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

#ifndef _MM_EX_RELOCATEPAYEEDIALOG_H_
#define _MM_EX_RELOCATEPAYEEDIALOG_H_

#define SYMBOL_RELOCATEPAYEEDIALOG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_RELOCATEPAYEEDIALOG_TITLE _("Relocate Payee Dialog")
#define SYMBOL_RELOCATEPAYEEDIALOG_IDNAME ID_DIALOG_BUDGETENTRY
#define SYMBOL_RELOCATEPAYEEDIALOG_SIZE wxSize(500, 300)
#define SYMBOL_RELOCATEPAYEEDIALOG_POSITION wxDefaultPosition

#include "guiid.h"
#include "defs.h"
#include "dbwrapper.h"
#include "mmcoredb.h"

class relocatePayeeDialog : public wxDialog
{
    DECLARE_DYNAMIC_CLASS( relocatePayeeDialog )
    DECLARE_EVENT_TABLE()

public:
    relocatePayeeDialog();
    relocatePayeeDialog( mmCoreDB* core,
        wxWindow* parent, wxWindowID id = SYMBOL_RELOCATEPAYEEDIALOG_IDNAME,
        const wxString& caption = SYMBOL_RELOCATEPAYEEDIALOG_TITLE,
        const wxPoint& pos = SYMBOL_RELOCATEPAYEEDIALOG_POSITION,
        const wxSize& size = SYMBOL_RELOCATEPAYEEDIALOG_SIZE,
        long style = SYMBOL_RELOCATEPAYEEDIALOG_STYLE );

    wxString updatedPayeesCount();

private:
    mmCoreDB* core_;
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_RELOCATEPAYEEDIALOG_IDNAME,
        const wxString& caption = SYMBOL_RELOCATEPAYEEDIALOG_TITLE,
        const wxPoint& pos = SYMBOL_RELOCATEPAYEEDIALOG_POSITION,
        const wxSize& size = SYMBOL_RELOCATEPAYEEDIALOG_SIZE,
        long style = SYMBOL_RELOCATEPAYEEDIALOG_STYLE );

    void CreateControls();
    void OnOk(wxCommandEvent& event);
    void OnPayeeUpdated(wxCommandEvent& event);

    int sourcePayeeID_;
    int destPayeeID_;

    wxComboBox* cbSourcePayee_;
    wxComboBox* cbDestPayee_;
    int changedPayees_;
    wxString prev_value_;

};

#endif

